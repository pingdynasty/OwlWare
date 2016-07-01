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
#include "Owl.h"
#include "MidiController.h"
#include "FlashStorage.h"

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

ProgramManager program;
ProgramVector staticVector;
ProgramVector* programVector = &staticVector;
extern "C" ProgramVector* getProgramVector() { return programVector; }

#if defined AUDIO_TASK_DIRECT
volatile ProgramVectorAudioStatus audioStatus = AUDIO_IDLE_STATUS;
#endif /* AUDIO_TASK_DIRECT */

static DynamicPatchDefinition dynamo;
static DynamicPatchDefinition flashPatches[MAX_USER_PATCHES];

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;
TaskHandle_t xFlashTaskHandle = NULL;
#if defined AUDIO_TASK_SEMAPHORE
SemaphoreHandle_t xSemaphore = NULL;
#endif // AUDIO_TASK_SEMAPHORE

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define PROGRAM_FLASH_NOTIFICATION  0x04
#define ERASE_FLASH_NOTIFICATION    0x08
#define PROGRAM_CHANGE_NOTIFICATION 0x10

PatchDefinition* getPatchDefinition(){
  return program.getPatchDefinition();
}

volatile int flashSectorToWrite;
volatile void* flashAddressToWrite;
volatile uint32_t flashSizeToWrite;

// static void eraseFlashProgram(int sector){
//   uint32_t addr = getFlashAddress(sector);
//   eeprom_unlock();
//   int ret = eeprom_erase(addr);
//   eeprom_lock();
//   if(ret != 0)
//     error(PROGRAM_ERROR, "Failed to erase flash sector");
// }

extern "C" {
  void runManagerTask(void* p){
    setup(); // call main OWL setup
    program.runManager();
  }

  void runProgramTask(void* p){
    PatchDefinition* def = getPatchDefinition();
    ProgramVector* vector = def->getProgramVector();
    if(def != NULL && vector != NULL){
      updateProgramVector(vector);
      programVector = vector;
      audioStatus = AUDIO_IDLE_STATUS;
      setErrorStatus(NO_ERROR);
      setLed(GREEN);
      codec.softMute(false);
      def->run();
      error(PROGRAM_ERROR, "Program exited");
    }else{
      error(PROGRAM_ERROR, "Invalid program");
    }
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
    // todo    
    int sector = flashSectorToWrite;
    uint32_t size = flashSizeToWrite;
    uint8_t* source = (uint8_t*)flashAddressToWrite;
    if(sector == 0xff && size < MAX_SYSEX_FIRMWARE_SIZE){
      flashFirmware(source, size);
    }else{
      if(size > storage.getFreeSize()){
	debugMessage("Insufficient flash available");
      }
      bool ret = storage.append(source, size);
      if(ret)
	debugMessage("Stored program to flash");
    }
    midi.sendProgramMessage();
    midi.sendDeviceStats();

    // int sector = flashSectorToWrite;
    // uint32_t size = flashSizeToWrite;
    // uint8_t* source = (uint8_t*)flashAddressToWrite;
    // if(sector >= 0 && sector < MAX_USER_PATCHES && size <= 128*1024){
    //   uint32_t addr = getFlashAddress(sector);
    //   eeprom_unlock();
    //   int ret = eeprom_erase(addr);
    //   if(ret == 0)
    // 	ret = eeprom_write_block(addr, source, size);
    //   eeprom_lock();
    //   registry.init();
    //   if(ret == 0){
    // 	// load and run program
    // 	int pc = registry.getNumberOfPatches()-MAX_USER_PATCHES+sector;
    // 	program.loadProgram(pc);
    // 	// program.loadDynamicProgram(source, size);
    // 	program.resetProgram(false);
    //   }else{
    // 	error(PROGRAM_ERROR, "Failed to write program to flash");
    //   }
    // }else if(sector == 0xff && size < MAX_SYSEX_FIRMWARE_SIZE){
    //   flashFirmware(source, size);
    // }else{
    //   error(PROGRAM_ERROR, "Invalid flash program command");
    // }
    vTaskDelete(NULL);
  }

  void eraseFlashTask(void* p){
    // todo
    int sector = flashSectorToWrite;
    if(sector == 0xff){
      storage.erase();
      debugMessage("Erased flash storage");
    }
    midi.sendProgramMessage();
    midi.sendDeviceStats();
    // int sector = flashSectorToWrite;
    // if(sector == 0xff){
    //   for(int i=0; i<MAX_USER_PATCHES; ++i)
    // 	eraseFlashProgram(i);
    //   settings.clearFlash();
    // }else if(sector >= 0 && sector < MAX_USER_PATCHES){
    //   eraseFlashProgram(sector);
    // }else{
    //   error(PROGRAM_ERROR, "Invalid flash erase command");
    // }
    // registry.init();
    vTaskDelete(NULL);
  }

  // static int midiMessagesToSend = 0;
  // void sendMidiDataTask(void* p){
  //   switch(midiMessagesToSend){
  //   case SYSEX_PRESET_NAME_COMMAND:
  //     midi.sendPatchNames();
  //     break;
  //   // case 0:
  //   //   midi.sendDeviceInfo();
  //   //   break;
  //   // case SYSEX_PARAMETER_NAME_COMMAND:
  //   //   midi.sendPatchParameterNames();
  //   //   break;
  //   // case SYSEX_FIRMWARE_VERSION:
  //   //   midi.sendFirmwareVersion();
  //   //   break;
  //   // case SYSEX_DEVICE_ID:
  //   //   midi.sendDeviceId();
  //   //   break;
  //   // case SYSEX_DEVICE_STATS:
  //   //   midi.sendDeviceStats();
  //   //   break;
  //   // case SYSEX_PROGRAM_MESSAGE:
  //   //   midi.sendProgramMessage();
  //   //   break;
  //   // case SYSEX_PROGRAM_STATS:
  //   //   midi.sendProgramStats();
  //   //   break;
  //   // case PATCH_BUTTON:
  //   //   midi.sendCc(PATCH_BUTTON, isPushButtonPressed() ? 127 : 0);
  //   //   break;
  //   // case LED:
  //   //   midi.sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
  //   //   break;
  //   // case 127:
  //   //   midi.sendSettings();
  //   //   break;
  //   }
  //   midiMessagesToSend = -1;
  //   vTaskDelete(NULL);    
  // }

#ifdef BUTTON_PROGRAM_CHANGE
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif /* abs */
  void programChangeTask(void* p){
    setLed(RED);
    int pc = settings.program_index;
    int bank = getAnalogValue(0)*5/4096;
    int prog = getAnalogValue(1)*8/4096+1;
    do{
      float a = getAnalogValue(0)*5/4096.0 - 0.5/5;
      float b = getAnalogValue(1)*8/4096.0 - 0.5/8;
      //      if(a - (int)a < 0.8) // deadband each segment: [0.8-1.0)
      if(a > 0 && abs(a - (int)a - 0.1) > 0.2) // deadband each segment: [0.9-1.1]
	bank = (int)a;
      if(b > 0 && abs(b-(int)b - 0.1) > 0.2)
	prog = (int)b+1;
      if(pc != bank*8+prog){
	toggleLed();
	pc = bank*8+prog;
	updateProgramIndex(pc);
	vTaskDelay(20);
      }
    }while(isPushButtonPressed() || pc < 1 || pc >= (int)registry.getNumberOfPatches());
    setLed(RED);
    program.loadProgram(pc);
    program.resetProgram(false);
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
  // getProgramVector()->status = AUDIO_READY_STATUS;
#endif
}

/* called by the program when a block has been processed */
__attribute__ ((section (".coderam")))
void ProgramManager::programReady(){
#ifdef DEBUG_DWT
  programVector->cycles_per_block = *DWT_CYCCNT;
  // getProgramVector()->cycles_per_block = *DWT_CYCCNT;
    // (*DWT_CYCCNT + getProgramVector()->cycles_per_block)>>1;
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

#define IDLE_TASK_STACK_SIZE            (512/sizeof(portSTACK_TYPE))
static StaticTask_t xIdleTaskTCBBuffer CCM;
static StackType_t xIdleStack[IDLE_TASK_STACK_SIZE] CCM;

extern "C" { 
  void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
  }
}

static StaticTask_t xTaskBuffer CCM;
static StackType_t xStack[ MANAGER_TASK_STACK_SIZE ] CCM; // long unsigned int

void ProgramManager::startManager(){
  if(xManagerHandle == NULL)
    // if(xTaskCreateStatic(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY, &xManagerHandle, NULL, NULL) == NULL)
    xManagerHandle = xTaskCreateStatic(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY,(StackType_t*)xStack, &xTaskBuffer);
  else
    error(PROGRAM_ERROR, "Manager task already started");
  if(xManagerHandle == NULL)
    error(PROGRAM_ERROR, "Failed to start manager task");

    // xTaskCreate(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY, &xManagerHandle);
#if defined AUDIO_TASK_SEMAPHORE
  if(xSemaphore == NULL)
    xSemaphore = xSemaphoreCreateBinary();
#endif // AUDIO_TASK_SEMAPHORE
}

void ProgramManager::notifyManagerFromISR(uint32_t ulValue){
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#ifdef DEFINE_OWL_SYSTICK
  // vPortYield(); // can we call this from an interrupt?
  // taskYIELD();
#endif /* DEFINE_OWL_SYSTICK */
}

void ProgramManager::notifyManager(uint32_t ulValue){
  if(xManagerHandle != NULL)
    xTaskNotify(xManagerHandle, ulValue, eSetBits);
}

void ProgramManager::startProgram(bool isr){
  if(isr)
    notifyManagerFromISR(START_PROGRAM_NOTIFICATION);
  else
    notifyManager(START_PROGRAM_NOTIFICATION);
}

void ProgramManager::exitProgram(bool isr){
  if(isr)
    notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION);
  else
    notifyManager(STOP_PROGRAM_NOTIFICATION);
}

/* exit and restart program */
void ProgramManager::resetProgram(bool isr){
  if(isr)
    notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
  else
    notifyManager(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
}

void ProgramManager::startProgramChange(bool isr){
  if(isr)
    notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|PROGRAM_CHANGE_NOTIFICATION);
  else
    notifyManager(STOP_PROGRAM_NOTIFICATION|PROGRAM_CHANGE_NOTIFICATION);
}

void ProgramManager::loadProgram(uint8_t pid){
  PatchDefinition* def = registry.getPatchDefinition(pid);
  if(def != NULL && def != patchdef && def->getProgramVector() != NULL){
    patchdef = def;
    updateProgramIndex(pid);
  }
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  if(dynamo.getProgramVector() != NULL){
    patchdef = &dynamo;
    registry.setDynamicPatchDefinition(patchdef);
    updateProgramIndex(0);
  }else{
    registry.setDynamicPatchDefinition(NULL);
  }
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
  return getProgramVector()->cycles_per_block;
}

uint32_t ProgramManager::getHeapMemoryUsed(){
  return getProgramVector()->heap_bytes_used;
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
      audioStatus = AUDIO_EXIT_STATUS;
      codec.softMute(true);
      if(xProgramHandle != NULL){
	programVector = &staticVector;
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
      }
    }
    // allow idle task to garbage collect if necessary
    vTaskDelay(20);
    // vTaskDelay(pdMS_TO_TICKS(200));
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      PatchDefinition* def = getPatchDefinition();
      if(xProgramHandle == NULL && def != NULL){
	static StaticTask_t xProgramTaskBuffer;
	if(def->getStackBase() != 0 && 
	   def->getStackSize() > configMINIMAL_STACK_SIZE*sizeof(portSTACK_TYPE)){
	  xProgramHandle = xTaskCreateStatic(runProgramTask, "Program", 
					     def->getStackSize()/sizeof(portSTACK_TYPE),
					     NULL, PROGRAM_TASK_PRIORITY, 
					     (StackType_t*)def->getStackBase(), 
					     &xProgramTaskBuffer);
	}else{
	  error(PROGRAM_ERROR, "Invalid program stack");
	  // ret = xTaskCreate(runProgramTask, "Program", PROGRAM_TASK_STACK_SIZE, NULL, PROGRAM_TASK_PRIORITY, &xProgramHandle);
	}
	if(xProgramHandle == NULL)
	  error(PROGRAM_ERROR, "Failed to start program task");
      }
      // todo: make sure no two tasks are using the same stack
#ifdef BUTTON_PROGRAM_CHANGE
    }else if(ulNotifiedValue & PROGRAM_CHANGE_NOTIFICATION){ // program change
      if(xProgramHandle == NULL){
	xProgramHandle = xTaskCreateStatic(programChangeTask, "Program Change", PC_TASK_STACK_SIZE, NULL, PC_TASK_PRIORITY, xStack, &xTaskBuffer);
	if(xProgramHandle == NULL)
	  error(PROGRAM_ERROR, "Failed to start Program Change task");
      }
#endif /* BUTTON_PROGRAM_CHANGE */
    }else if(ulNotifiedValue & PROGRAM_FLASH_NOTIFICATION){ // program flash
      xFlashTaskHandle = xTaskCreateStatic(programFlashTask, "Flash Write", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, xStack, &xTaskBuffer);
      if(xFlashTaskHandle == NULL){
	error(PROGRAM_ERROR, "Failed to start Flash Write task");
      }
    }else if(ulNotifiedValue & ERASE_FLASH_NOTIFICATION){ // erase flash
      xFlashTaskHandle = xTaskCreateStatic(eraseFlashTask, "Flash Erase", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, xStack, &xTaskBuffer);
      if(xFlashTaskHandle == NULL){
	error(PROGRAM_ERROR, "Failed to start Flash Erase task");
      }
    }
  }
}

void ProgramManager::eraseProgramFromFlash(uint8_t sector){
  flashSectorToWrite = sector;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|ERASE_FLASH_NOTIFICATION);
}

void ProgramManager::saveProgramToFlash(uint8_t sector, void* address, uint32_t length){
  flashSectorToWrite = sector;
  flashAddressToWrite = address;
  flashSizeToWrite = length;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|PROGRAM_FLASH_NOTIFICATION);
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
