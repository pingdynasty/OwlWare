#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "SharedMemory.h"
#include "owlcontrol.h"
#include "eepromcontrol.h"
#include "device.h"
#include "DynamicPatchDefinition.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

DynamicPatchDefinition dynamo;
// #define JUMPTO(address) ((void (*)(void))address)();

ProgramManager program;

extern void setup(); // main OWL setup

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;
SemaphoreHandle_t xSemaphore = NULL;

#define MANAGER_STACK_SIZE          (8*1024/sizeof(portSTACK_TYPE))
// #define PROGRAM_STACK_SIZE          (46*1024/sizeof(portSTACK_TYPE))

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] CCM;

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02

#define AUDIO_TASK_SUSPEND
// #define AUDIO_TASK_SEMAPHORE
// #define AUDIO_TASK_DIRECT
// #define AUDIO_TASK_YIELD
/* if AUDIO_TASK_YIELD is defined, define DEFINE_OWL_SYSTICK in device.h */

PatchDefinition* patchdef = NULL;

extern "C" {
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction = NULL;
  void runProgramTask(void* p){
    if(patchdef != NULL){
      setLed(GREEN);
      patchdef->run();
    }
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

/* called by the audio interrupt when a block should be processed */
void ProgramManager::audioReady(){
#ifdef DEBUG_DWT
  *DWT_CYCCNT = 0; // reset the performance counter
#endif /* DEBUG_DWT */
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
  getSharedMemory()->status = AUDIO_READY_STATUS;
#endif
}

/* called by the program when a block has been processed */
void ProgramManager::programReady(){
#ifdef DEBUG_DWT
  getSharedMemory()->cycles_per_block = *DWT_CYCCNT;
#endif /* DEBUG_DWT */
#ifdef AUDIO_TASK_SUSPEND
  vTaskSuspend(xProgramHandle);
#elif defined AUDIO_TASK_SEMAPHORE
  xSemaphoreTake(xSemaphore, 0);
#elif defined AUDIO_TASK_YIELD
  taskYIELD();
#else /* AUDIO_TASK_DIRECT */
  while(getSharedMemory()->status != AUDIO_READY_STATUS);
  getSharedMemory()->status = AUDIO_PROCESSED_STATUS;
#endif
}

/* called by the program when an error or anomaly has occured */
void ProgramManager::programStatus(int){
  setLed(RED);
  for(;;);
}

void ProgramManager::startManager(){
  if(xManagerHandle == NULL)
    xTaskCreate(runManagerTask, "Manager", MANAGER_STACK_SIZE, NULL, 4, &xManagerHandle);
  if(xSemaphore == NULL)
    xSemaphore = xSemaphoreCreateBinary();
}

void ProgramManager::startProgram(){
  getSharedMemory()->status = AUDIO_IDLE_STATUS;
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
  getSharedMemory()->status = AUDIO_EXIT_STATUS; // request program exit
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

void ProgramManager::loadStaticProgram(PatchDefinition* def){
  patchdef = def;
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  patchdef = &dynamo;
}

uint32_t ProgramManager::getProgramStackSize(){
  if(xProgramHandle == NULL)
    return 0;
  pH = uxTaskGetStackHighWaterMark(xProgramHandle);
  return pH*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getProgramStackAllocation(){
  if(patchdef != NULL)
    return patchdef->getStackSize();
  return 0;
}

#if 0

/* assumes program is stopped */
void ProgramManager::loadStaticProgram(uint8_t pid){
  programAddress = NULL;
  programFunction = NULL;
  program_setup(pid);
  programFunction = program_run;
  programStackSize = STATIC_PROGRAM_STACK_SIZE;
  programStackBase = (uint32_t*)spHeap;
  // programStackSize = 1024*1024;
  // programStackSize = 255*1024; // usStackDepth is uint16_t, heap must be < 256kb
  // programStackBase = (uint32_t*)EXTRAM;
  // xTaskGenericCreate(runStaticProgramTask, "Static Program", programStackSize/sizeof(portSTACK_TYPE), NULL, 2, &xProgramHandle, programStackBase, NULL);
}

/* assumes program is stopped */
void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  // programAddress = (uint32_t*)address;
  // programLength = length;
  // programStackBase = (uint32_t*)*(programAddress+3); // stack base pointer (low end of heap/stack)
  // programStackSize = *(programAddress+4) - *(programAddress+3);
  // strncpy(programName, (char*)(programAddress+5), sizeof(programName));
  // uint32_t jumpAddress = *(programAddress+1); // main pointer
  // uint32_t link = *(programAddress+2); // link base address
  // /* copy program to ram */
  // if((link == PATCHRAM && programLength <= 80*1024) ||
  //    (link == EXTRAM && programLength <= 1024*1024)){
  //   memcpy((void*)link, (void*)programAddress, programLength);
  //   programFunction = (ProgramFunction)jumpAddress;
  //   programAddress = (uint32_t*)link;
  // }else{
  //   programFunction = NULL;
  // }
}

bool ProgramManager::verify(){
  if(programFunction == NULL)
    return false;
  if(programFunction == program_run)
    return true;
  if(*(uint32_t*)programAddress != 0xDADAC0DE)
    return false;
  if(((uint32_t)programStackBase >= PATCHRAM && (uint32_t)programStackBase+programStackSize <= (PATCHRAM+80*1024)) ||
     ((uint32_t)programStackBase >= CCMRAM && (uint32_t)programStackBase+programStackSize <= (CCMRAM+64*1024)) ||
     ((uint32_t)programStackBase >= EXTRAM && (uint32_t)programStackBase+programStackSize <= (EXTRAM+80*1024)))
    return true;
  return false;
}
#endif 

void ProgramManager::runManager(){
  uint32_t ulNotifiedValue = 0;
  // TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
  TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  setLed(GREEN);
  for(;;){
    stats();    
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,          /* Don't clear any notification bits on entry. */
		    UINT32_MAX,       /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime ); 
    stats();
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop      
      if(xProgramHandle != NULL){
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
	// running = false;
      }
    }
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      if(xProgramHandle == NULL && patchdef != NULL)
	xTaskGenericCreate(runProgramTask, "Program", 
			   patchdef->getStackSize()/sizeof(portSTACK_TYPE), 
			   NULL, 2, &xProgramHandle, 
			   patchdef->getStackBase(), NULL);
	// xTaskCreate(runProgramTask, "Program", PROGRAM_STACK_SIZE, NULL, 2, &xProgramHandle);
    }
  }
}

#if 0
bool ProgramManager::saveProgram(uint8_t sector){
  if(sector > 4)
    return false;
  uint32_t addr = (uint32_t)0x080E0000; // ADDR_FLASH_SECTOR_11
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000
  addr -= 0x08004000; // FLASH_SECTOR_1, eeprom base address
  if(addr < 0x3c000 || addr > 0xdc000)
    return false;
  eeprom_unlock();
  eeprom_erase(addr);
  // todo: write program size (and name) in first few bytes
  // assumes program is loaded to PATCHRAM
  eeprom_write_block(addr, (uint8_t*)PATCHRAM, programLength);
  eeprom_lock();
  return true;
}

bool ProgramManager::loadProgram(uint8_t sector){
  if(sector > 4)
    return false;
  uint32_t addr = (uint32_t)0x080E0000; // ADDR_FLASH_SECTOR_11
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000  
  uint32_t size = 80*1024; // todo: read program size (and name) from first few bytes
  loadDynamicProgram((void*)addr, size);
  return true;
}
#endif // 0

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
