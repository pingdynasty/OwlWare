#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "ProgramVector.h"
#include "owlcontrol.h"
#include "eepromcontrol.h"
#include "device.h"
#include "DynamicPatchDefinition.hpp"
#include "FreeRTOS.h"
#include "PatchRegistry.h"
#include "ApplicationSettings.h"

// #define AUDIO_TASK_SUSPEND
// #define AUDIO_TASK_SEMAPHORE
#define AUDIO_TASK_DIRECT
// #define AUDIO_TASK_YIELD
/* if AUDIO_TASK_YIELD is defined, define DEFINE_OWL_SYSTICK in device.h */

#include "task.h"
#if defined AUDIO_TASK_SEMAPHORE
#include "semphr.h"
#endif

DynamicPatchDefinition dynamo;
// #define JUMPTO(address) ((void (*)(void))address)();

ProgramManager program;
ProgramVector staticVector;
ProgramVector* currentProgramVector = &staticVector;

ProgramVector* getProgramVector(){
  return currentProgramVector;
}

extern void setup(); // main OWL setup
extern void updateProgramVector(ProgramVector*);

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;
#if defined AUDIO_TASK_SEMAPHORE
SemaphoreHandle_t xSemaphore = NULL;
#endif // AUDIO_TASK_SEMAPHORE

#define MANAGER_STACK_SIZE          (6*1024/sizeof(portSTACK_TYPE))
// #define PROGRAM_STACK_SIZE          (46*1024/sizeof(portSTACK_TYPE))
#define PROGRAM_STACK_SIZE          (32*1024/sizeof(portSTACK_TYPE))
#define PROGRAM_PRIORITY            (2 | portPRIVILEGE_BIT)
#define MANAGER_PRIORITY            (4 | portPRIVILEGE_BIT)
// #define PROGRAM_PRIORITY            (2)
// #define MANAGER_PRIORITY            (4)
// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] CCM;

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define RESET_PROGRAM_NOTIFICATION  0x03

PatchDefinition* patchdef = NULL;

extern "C" {
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction = NULL;
  void runProgramTask(void* p){
    if(patchdef != NULL){
      updateProgramVector(currentProgramVector);
      setErrorStatus(NO_ERROR);
      setLed(GREEN);
      patchdef->run();
    }
    setErrorStatus(PROGRAM_ERROR);
    setLed(RED);
    for(;;);
  }
  void runManagerTask(void* p){
    setup(); // call main OWL setup
    program.runManager();
  }
}

#ifdef DEBUG_DWT
volatile uint32_t *DWT_CYCCNT = (volatile uint32_t *)0xE0001004; //address of the register
#endif /* DEBUG_DWT */
ProgramManager::ProgramManager() { //: patchdef(NULL) {
#ifdef DEBUG_DWT
  // initialise DWT cycle counter
  volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
  volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter
#endif /* DEBUG_DWT */
}

#ifdef DEBUG_STACK
unsigned long pH = 0;
unsigned long mH = 0;
unsigned long tH = 0;

void stats(){
  if(xProgramHandle != NULL){
    pH = uxTaskGetStackHighWaterMark(xProgramHandle);
  }
  if(xManagerHandle != NULL){
    mH = uxTaskGetStackHighWaterMark(xManagerHandle);
  }
  UBaseType_t high;
  high = uxTaskGetNumberOfTasks();
  if(high > tH)
    tH = high;
}
#endif /* DEBUG_STACK */

#if defined AUDIO_TASK_DIRECT
volatile ProgramVectorAudioStatus audioStatus = AUDIO_IDLE_STATUS;
#endif /* AUDIO_TASK_DIRECT */

/* called by the audio interrupt when a block should be processed */
__attribute__ ((section (".coderam")))
void ProgramManager::audioReady(){
#if defined AUDIO_TASK_SUSPEND || defined AUDIO_TASK_YIELD
  if(xProgramHandle != NULL){
    BaseType_t xHigherPriorityTaskWoken = 0; 
    xHigherPriorityTaskWoken = xTaskResumeFromISR(xProgramHandle);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#elif defined AUDIO_TASK_SEMAPHORE
  signed long int xHigherPriorityTaskWoken = pdFALSE; 
  // signed BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
  // xSemaphoreGiveFromISR(xSemaphore, NULL);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else /* AUDIO_TASK_DIRECT */
  audioStatus = AUDIO_READY_STATUS;
  // currentProgramVector->status = AUDIO_READY_STATUS;
#endif
}

/* called by the program when a block has been processed */
__attribute__ ((section (".coderam")))
void ProgramManager::programReady(){
#ifdef DEBUG_DWT
  currentProgramVector->cycles_per_block = 
    (*DWT_CYCCNT + currentProgramVector->cycles_per_block)>>1;
#endif /* DEBUG_DWT */
#ifdef DEBUG_AUDIO
  clearPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
#endif
#ifdef AUDIO_TASK_SUSPEND
  vTaskSuspend(xProgramHandle);
#elif defined AUDIO_TASK_SEMAPHORE
  xSemaphoreTake(xSemaphore, 0);
#elif defined AUDIO_TASK_YIELD
  taskYIELD(); // this will only suspend the task if another is ready to run
#else /* AUDIO_TASK_DIRECT */
  // while(currentProgramVector->status != AUDIO_READY_STATUS);
  // currentProgramVector->status = AUDIO_PROCESSED_STATUS;
  while(audioStatus != AUDIO_READY_STATUS);
  audioStatus = AUDIO_PROCESSED_STATUS;
#endif
#ifdef DEBUG_DWT
  *DWT_CYCCNT = 0; // reset the performance counter
#endif /* DEBUG_DWT */
#ifdef DEBUG_AUDIO
  setPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
#endif
}

/* called by the program when an error or anomaly has occured */
void ProgramManager::programStatus(int){
  setLed(RED);
  for(;;);
}

void ProgramManager::startManager(){
  if(xManagerHandle == NULL)
    xTaskCreate(runManagerTask, "Manager", MANAGER_STACK_SIZE, NULL, MANAGER_PRIORITY, &xManagerHandle);
#if defined AUDIO_TASK_SEMAPHORE
  if(xSemaphore == NULL)
    xSemaphore = xSemaphoreCreateBinary();
#endif // AUDIO_TASK_SEMAPHORE
}

void ProgramManager::startProgram(){
  audioStatus = AUDIO_IDLE_STATUS;
  // currentProgramVector->status = AUDIO_IDLE_STATUS;
  uint32_t ulValue = START_PROGRAM_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#ifdef DEFINE_OWL_SYSTICK
  // vPortYield(); // can we call this from an interrupt?
  // taskYIELD();
#endif /* DEFINE_OWL_SYSTICK */
}

void ProgramManager::exit(){
  audioStatus = AUDIO_EXIT_STATUS;
  // currentProgramVector->status = AUDIO_EXIT_STATUS; // request program exit
  uint32_t ulValue = STOP_PROGRAM_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#ifdef DEFINE_OWL_SYSTICK
  // vPortYield(); // can we call this from an interrupt?
  // taskYIELD();
#endif /* DEFINE_OWL_SYSTICK */
}

/* exit and restart program */
void ProgramManager::reset(){
  // uint32_t ulValue = RESET_PROGRAM_NOTIFICATION;
  uint32_t ulValue = STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

// extern "C" void program_setup(uint8_t pid);
// extern "C" void program_run();
// void runStaticProgramTask(void* p){
//   program_run();
//   for(;;);
// }

void ProgramManager::loadProgram(uint8_t pid){
  // if(pid < MAX_FACTORY_PROGRAM){
  // if(pid > 0 && pid < registry.getNumberOfPatches()){
  PatchDefinition* def = registry.getPatchDefinition(pid);
  if(def != NULL && def != patchdef){
    program.loadStaticProgram(def);
    updateProgramIndex(pid);
  }
  // }
    // if(pid < MAX_FACTORY_PROGRAM)
    //   loadFactoryPatch(pid);
    // else
    //   // time to erase 128kB flash sector, typ 875ms
    //   // Program/erase parallelism
    //   // (PSIZE) = x 32 : 1-2s
    //   loadProgram(pid);
}

void ProgramManager::loadStaticProgram(PatchDefinition* def){
  patchdef = def;
  currentProgramVector = def->getProgramVector();
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  patchdef = &dynamo;
  currentProgramVector = dynamo.getProgramVector();
  updateProgramIndex(0);
}

#ifdef DEBUG_STACK
uint32_t ProgramManager::getProgramStackSize(){
  if(xProgramHandle == NULL)
    return 0;
  pH = uxTaskGetStackHighWaterMark(xProgramHandle);
  return pH*sizeof(portSTACK_TYPE);
}
#endif /* DEBUG_STACK */

uint32_t ProgramManager::getProgramStackAllocation(){
  if(patchdef != NULL)
    return patchdef->getStackSize();
  return 0;
}

uint32_t ProgramManager::getCyclesPerBlock(){
  return currentProgramVector->cycles_per_block;
}

uint32_t ProgramManager::getHeapMemoryUsed(){
  return currentProgramVector->heap_bytes_used;
}

void ProgramManager::runManager(){
  uint32_t ulNotifiedValue = 0;
  // TickType_t xMaxBlockTime = pdMS_TO_TICKS( 1000 );
  TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  setLed(GREEN);
  for(;;){
#ifdef DEBUG_STACK
    stats();
#endif /* DEBUG_STACK */
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,          /* Don't clear any notification bits on entry. */
		    UINT32_MAX,       /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime ); 
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop      
      if(xProgramHandle != NULL){
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
	// allow idle task to garbage collect if necessary
	// vTaskDelay(100/portTICK_PERIOD_MS);
	vTaskDelay(20);
      }
    }
    // if(ulNotifiedValue & RESET_PROGRAM_NOTIFICATION){ // reset
    //   // allow idle task to garbage collect if necessary
    //   // vTaskDelay(100/portTICK_PERIOD_MS);
    //   vTaskDelay(20);
    // }
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      if(xProgramHandle == NULL && patchdef != NULL){
	BaseType_t ret;
	if(patchdef->getStackSize() > configMINIMAL_STACK_SIZE*sizeof(portSTACK_TYPE)){
	  ret = xTaskGenericCreate(runProgramTask, "Program", 
				   patchdef->getStackSize()/sizeof(portSTACK_TYPE), 
				   NULL, PROGRAM_PRIORITY, &xProgramHandle, 
				   patchdef->getStackBase(), NULL);
	}else{
	  ret = xTaskCreate(runProgramTask, "Program", PROGRAM_STACK_SIZE, NULL, PROGRAM_PRIORITY, &xProgramHandle);
	}
	if(ret != pdPASS){
	  setErrorStatus(PROGRAM_ERROR);
	  setLed(RED);
	}
      }
    }
  }
}

DynamicPatchDefinition flashPatches[4];
PatchDefinition* ProgramManager::getPatchDefinitionFromFlash(uint8_t sector){
  if(sector > 4)
    return NULL;
  uint32_t addr = (uint32_t)0x080E0000; // ADDR_FLASH_SECTOR_11
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000  
  ProgramHeader* header = (ProgramHeader*)addr;
  DynamicPatchDefinition* def = &flashPatches[sector];
  uint32_t size = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
  if(header->magic == 0xDADAC0DE && size <= 80*1024){
    if(def->load((void*)addr, size) && def->verify())
      return def;
  }
  return NULL;
}

bool ProgramManager::saveProgramToFlash(uint8_t sector){
  // save current dynamic program (program index 0)
  if(sector > 4)
    return false;
  if(!dynamo.verify())
    return false;
  uint32_t addr = (uint32_t)0x080E0000; // ADDR_FLASH_SECTOR_11
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000
  addr -= 0x08004000; // FLASH_SECTOR_1, eeprom base address
  if(addr < 0x3c000 || addr > 0xdc000)
    return false;
  eeprom_unlock();
  int ret = eeprom_erase(addr);
  if(ret)
    return false;
  ret = eeprom_write_block(addr, (uint8_t*)dynamo.getLinkAddress(), dynamo.getProgramSize());
  eeprom_lock();
  return ret == 0;
}

bool ProgramManager::loadProgramFromFlash(uint8_t sector){
  if(sector > 4)
    return false;
  uint32_t addr = (uint32_t)0x080E0000; // ADDR_FLASH_SECTOR_11
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000  
  uint32_t size = 80*1024; // todo: read program size (and name) from first few bytes
  loadDynamicProgram((void*)addr, size);
  return true;
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
