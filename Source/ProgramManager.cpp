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
#include "CodecController.h"

#ifdef BUTTON_PROGRAM_CHANGE
#define PROGRAM_CHANGE_PUSHBUTTON_MS 2000
#include "clock.h"
#endif /* BUTTON_PROGRAM_CHANGE */

// #include "stm32f4xx.h" // for FLASH_SECTOR defs

// #define AUDIO_TASK_SUSPEND
// #define AUDIO_TASK_SEMAPHORE
#define AUDIO_TASK_DIRECT
// #define AUDIO_TASK_YIELD
/* if AUDIO_TASK_YIELD is defined, define DEFINE_OWL_SYSTICK in device.h */

// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).
#define PROGRAM_TASK_PRIORITY            (2)
#define MANAGER_TASK_PRIORITY            (4 | portPRIVILEGE_BIT)
#define FLASH_TASK_PRIORITY              (3 | portPRIVILEGE_BIT)
#define PC_TASK_PRIORITY                 (2)

#include "task.h"
#if defined AUDIO_TASK_SEMAPHORE
#include "semphr.h"
#endif

extern void setup(); // main OWL setup
extern void updateProgramVector(ProgramVector*);

ProgramManager program;
ProgramVector staticVector;
static ProgramVector* currentProgramVector = &staticVector;

// #define JUMPTO(address) ((void (*)(void))address)();
static DynamicPatchDefinition dynamo;
static DynamicPatchDefinition flashPatches[MAX_USER_PATCHES];

ProgramVector* getProgramVector(){
  return currentProgramVector;
}

static uint32_t getFlashAddress(int sector){
  uint32_t addr = ADDR_FLASH_SECTOR_11;
  addr -= sector*128*1024; // count backwards by 128k blocks, ADDR_FLASH_SECTOR_7 is at 0x08060000
  return addr;
}

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;
TaskHandle_t xFlashTaskHandle = NULL;
#if defined AUDIO_TASK_SEMAPHORE
SemaphoreHandle_t xSemaphore = NULL;
#endif // AUDIO_TASK_SEMAPHORE

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] CCM;

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define RESET_PROGRAM_NOTIFICATION  0x03
#define PROGRAM_FLASH_NOTIFICATION  0x04
#define ERASE_FLASH_NOTIFICATION    0x08
#define PROGRAM_CHANGE_NOTIFICATION 0x10

PatchDefinition* patchdef = NULL;
volatile int flashSectorToWrite;
volatile void* flashAddressToWrite;
volatile uint32_t flashSizeToWrite;

static void eraseFlashProgram(int sector){
  uint32_t addr = getFlashAddress(sector);
  eeprom_unlock();
  int ret = eeprom_erase(addr);
  eeprom_lock();
  if(ret != 0)
    setErrorMessage(PROGRAM_ERROR, "Failed to erase flash sector");
}

extern "C" {
  void runManagerTask(void* p){
    setup(); // call main OWL setup
    program.runManager();
  }

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

  /*
   * re-program firmware: this entire function and all subroutines must run from RAM
   * (don't make this static!)
   */
  __attribute__ ((section (".coderam")))
  void flashFirmware(uint8_t* source, uint32_t size){
    __disable_irq(); // Disable ALL interrupts. Can only be executed in Privileged modes.
    setLed(RED);
    eeprom_unlock();
    if(size > (16+16+64+128)*1024){
      eeprom_erase_sector(FLASH_Sector_6);
      toggleLed(); // inline
    }
    if(size > (16+16+64)*1024){
      eeprom_erase_sector(FLASH_Sector_5);
      toggleLed();
    }
    if(size > (16+16)*1024){
      eeprom_erase_sector(FLASH_Sector_4);
      toggleLed();
    }
    if(size > 16*1024){
      eeprom_erase_sector(FLASH_Sector_3);
      toggleLed();
    }
    eeprom_erase_sector(FLASH_Sector_2);
    toggleLed();
    eeprom_write_block(ADDR_FLASH_SECTOR_2, source, size);
    eeprom_lock();
    eeprom_wait();
    NVIC_SystemReset(); // (static inline)
  }

  void programFlashTask(void* p){
    int sector = flashSectorToWrite;
    uint32_t size = flashSizeToWrite;
    uint8_t* source = (uint8_t*)flashAddressToWrite;
    if(sector >= 0 && sector < MAX_USER_PATCHES && size <= 128*1024){
      uint32_t addr = getFlashAddress(sector);
      eeprom_unlock();
      int ret = eeprom_erase(addr);
      if(ret == 0)
	ret = eeprom_write_block(addr, source, size);
      eeprom_lock();
      if(ret == 0){
	registry.init();
	// load and run program
	int pc = registry.getNumberOfPatches()-sector-1;
	program.loadProgram(pc);
	// if we knew the PC we could run from flash
	// program.loadDynamicProgram(source, size);
	program.startProgram(false);
      }else{
	setErrorMessage(PROGRAM_ERROR, "Failed to program flash sector");
      }
    }else if(sector == 0xff && size < MAX_SYSEX_FIRMWARE_SIZE){
      flashFirmware(source, size);
    }else{
      setErrorMessage(PROGRAM_ERROR, "Invalid flash program command");
    }
    vTaskDelete(NULL);
  }

  void eraseFlashTask(void* p){
    int sector = flashSectorToWrite;
    if(sector == 0xff){
      for(int i=0; i<MAX_USER_PATCHES; ++i)
	eraseFlashProgram(i);
      settings.clearFlash();
    }else if(sector >= 0 && sector < MAX_USER_PATCHES){
      eraseFlashProgram(sector);
    }else{
      setErrorMessage(PROGRAM_ERROR, "Invalid flash erase command");
    }
    registry.init();
    vTaskDelete(NULL);
  }

#ifdef BUTTON_PROGRAM_CHANGE
  void programChangeTask(void* p){
    setLed(RED);
    int pc = 0;
    do{
      int bank = getAnalogValue(0) >> 10;
      int program = getAnalogValue(1) >> 9;
      if(pc != bank*8+program+1){
	toggleLed();
	pc = bank*8+program+1;
      }
      // vTaskDelay(20);
    }while(isPushButtonPressed() || pc < 1 || pc >= (int)registry.getNumberOfPatches());
    program.loadProgram(pc);
    program.resetProgram(false);
    setLed(GREEN);
    for(;;); // wait for program manager to delete this task
  }
#endif /* BUTTON_PROGRAM_CHANGE */
}

#ifdef DEBUG_DWT
volatile uint32_t *DWT_CYCCNT = (volatile uint32_t *)0xE0001004; //address of the register
#endif /* DEBUG_DWT */
ProgramManager::ProgramManager() {
#ifdef DEBUG_DWT
  // initialise DWT cycle counter
  volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; // address of the register
  volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; // address of the register
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter
#endif /* DEBUG_DWT */
}

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
  currentProgramVector->cycles_per_block = *DWT_CYCCNT;
    // (*DWT_CYCCNT + currentProgramVector->cycles_per_block)>>1;
#endif /* DEBUG_DWT */
#ifdef DEBUG_AUDIO
  clearPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
#endif

#ifdef BUTTON_PROGRAM_CHANGE
  extern volatile uint32_t pushButtonPressed;
  if(pushButtonPressed && getSysTicks() > pushButtonPressed+PROGRAM_CHANGE_PUSHBUTTON_MS){
    setLed(NONE);
    pushButtonPressed = 0; // prevent re-trigger
    program.startProgramChange(true);
  }
#endif /* BUTTON_PROGRAM_CHANGE */

#ifdef AUDIO_TASK_SUSPEND
  vTaskSuspend(xProgramHandle);
#elif defined AUDIO_TASK_SEMAPHORE
  xSemaphoreTake(xSemaphore, 0);
#elif defined AUDIO_TASK_YIELD
  taskYIELD(); // this will only suspend the task if another is ready to run
#elif defined AUDIO_TASK_DIRECT
  while(audioStatus != AUDIO_READY_STATUS);
  audioStatus = AUDIO_PROCESSING_STATUS;
#else
  #error "Invalid AUDIO_TASK setting"
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
    xTaskCreate(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY, &xManagerHandle);
#if defined AUDIO_TASK_SEMAPHORE
  if(xSemaphore == NULL)
    xSemaphore = xSemaphoreCreateBinary();
#endif // AUDIO_TASK_SEMAPHORE
}

void ProgramManager::notifyProgramFromISR(uint32_t ulValue){
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#ifdef DEFINE_OWL_SYSTICK
  // vPortYield(); // can we call this from an interrupt?
  // taskYIELD();
#endif /* DEFINE_OWL_SYSTICK */
}

void ProgramManager::notifyProgram(uint32_t ulValue){
  if(xManagerHandle != NULL)
    xTaskNotify(xManagerHandle, ulValue, eSetBits);
}

void ProgramManager::startProgram(bool isr){
  audioStatus = AUDIO_IDLE_STATUS;
  if(isr)
    notifyProgramFromISR(START_PROGRAM_NOTIFICATION);
  else
    notifyProgram(START_PROGRAM_NOTIFICATION);
}

void ProgramManager::exitProgram(bool isr){
  audioStatus = AUDIO_EXIT_STATUS;
  if(isr)
    notifyProgramFromISR(STOP_PROGRAM_NOTIFICATION);
  else
    notifyProgram(STOP_PROGRAM_NOTIFICATION);
}

/* exit and restart program */
void ProgramManager::resetProgram(bool isr){
  if(isr)
    notifyProgramFromISR(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
  else
    notifyProgram(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
}

void ProgramManager::startProgramChange(bool isr){
  if(isr)
    notifyProgramFromISR(STOP_PROGRAM_NOTIFICATION|PROGRAM_CHANGE_NOTIFICATION);
  else
    notifyProgram(STOP_PROGRAM_NOTIFICATION|PROGRAM_CHANGE_NOTIFICATION);
}

void ProgramManager::loadProgram(uint8_t pid){
  PatchDefinition* def = registry.getPatchDefinition(pid);
  if(def != NULL && def != patchdef){
    patchdef = def;
    currentProgramVector = def->getProgramVector();
    updateProgramIndex(pid);
  }
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  patchdef = &dynamo;
  currentProgramVector = dynamo.getProgramVector();
  updateProgramIndex(0);
}

#ifdef DEBUG_STACK
uint32_t ProgramManager::getProgramStackUsed(){
  if(xProgramHandle == NULL)
    return 0;
  uint32_t ph = uxTaskGetStackHighWaterMark(xProgramHandle);
  return getProgramStackAllocation() - ph*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getProgramStackAllocation(){
  uint32_t ss = 0;
  if(patchdef != NULL)
    ss = patchdef->getStackSize();
  if(ss == 0)
    ss = PROGRAM_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
  return ss;
}

uint32_t ProgramManager::getManagerStackUsed(){
  if(xManagerHandle == NULL)
    return 0;
  uint32_t mh = uxTaskGetStackHighWaterMark(xManagerHandle);
  return getManagerStackAllocation() - mh*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getManagerStackAllocation(){
  return MANAGER_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getFreeHeapSize(){
  return xPortGetFreeHeapSize();
}
#endif /* DEBUG_STACK */

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
  for(;;){
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,          /* Don't clear any notification bits on entry. */
		    UINT32_MAX,       /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime ); 
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop      
      codec.softMute(true);
      if(xProgramHandle != NULL){
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
	// allow idle task to garbage collect if necessary
	vTaskDelay(20);
      }
    }
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      if(xProgramHandle == NULL && patchdef != NULL){
	BaseType_t ret;
	if(patchdef->getStackSize() > configMINIMAL_STACK_SIZE*sizeof(portSTACK_TYPE)){
	  ret = xTaskGenericCreate(runProgramTask, "Program", 
				   patchdef->getStackSize()/sizeof(portSTACK_TYPE), 
				   NULL, PROGRAM_TASK_PRIORITY, &xProgramHandle, 
				   patchdef->getStackBase(), NULL);
	}else{
	  ret = xTaskCreate(runProgramTask, "Program", PROGRAM_TASK_STACK_SIZE, NULL, PROGRAM_TASK_PRIORITY, &xProgramHandle);
	}
	if(ret != pdPASS){
	  setErrorStatus(PROGRAM_ERROR);
	  setLed(RED);
	}else{
	  codec.softMute(false);
	}
      }
#ifdef BUTTON_PROGRAM_CHANGE
    }else if(ulNotifiedValue & PROGRAM_CHANGE_NOTIFICATION){ // program change
      if(xProgramHandle == NULL){
	BaseType_t ret = xTaskCreate(programChangeTask, "Program Change", PC_TASK_STACK_SIZE, NULL, PC_TASK_PRIORITY, &xProgramHandle);
	if(ret != pdPASS){
	  setErrorMessage(PROGRAM_ERROR, "Failed to start Program Change task");
	  setLed(RED);
	}
      }
#endif /* BUTTON_PROGRAM_CHANGE */
    }else if(ulNotifiedValue == PROGRAM_FLASH_NOTIFICATION){ // program flash
      BaseType_t ret = xTaskCreate(programFlashTask, "Flash Write", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &xFlashTaskHandle);
      if(ret != pdPASS){
	setErrorMessage(PROGRAM_ERROR, "Failed to start Flash Write task");
	setLed(RED);
      }
    }else if(ulNotifiedValue == ERASE_FLASH_NOTIFICATION){ // erase flash
      BaseType_t ret = xTaskCreate(eraseFlashTask, "Flash Erase", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &xFlashTaskHandle);
      if(ret != pdPASS){
	setErrorMessage(PROGRAM_ERROR, "Failed to start Flash Erase task");
	setLed(RED);
      }
    }
  }
}

PatchDefinition* ProgramManager::getPatchDefinitionFromFlash(uint8_t sector){
  if(sector >= MAX_USER_PATCHES)
    return NULL;
  uint32_t addr = getFlashAddress(sector);
  ProgramHeader* header = (ProgramHeader*)addr;
  DynamicPatchDefinition* def = &flashPatches[sector];
  uint32_t size = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
  if(header->magic == 0xDADAC0DE && size <= 80*1024){
    if(def->load((void*)addr, size) && def->verify())
      return def;
  }
  return NULL;
}

void ProgramManager::eraseProgramFromFlash(uint8_t sector){
  flashSectorToWrite = sector;
  uint32_t ulValue = ERASE_FLASH_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0; 
  xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ProgramManager::saveProgramToFlash(uint8_t sector, void* address, uint32_t length){
  flashSectorToWrite = sector;
  flashAddressToWrite = address;
  flashSizeToWrite = length;
  uint32_t ulValue = PROGRAM_FLASH_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0;
  xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
