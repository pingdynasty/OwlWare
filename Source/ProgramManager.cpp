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

// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).
#define PROGRAM_TASK_PRIORITY            (2)
#define MANAGER_TASK_PRIORITY            (4 | portPRIVILEGE_BIT)
#define FLASH_TASK_PRIORITY              (3 | portPRIVILEGE_BIT)
#define PC_TASK_PRIORITY                 (2)

#include "task.h"

ProgramManager program;
ProgramVector staticVector;
ProgramVector* programVector = &staticVector;
extern "C" ProgramVector* getProgramVector() { return programVector; }
volatile ProgramVectorAudioStatus audioStatus = AUDIO_IDLE_STATUS;
static DynamicPatchDefinition dynamo;
TaskHandle_t xProgramHandle = NULL;

template<uint32_t STACK_SIZE>
class StaticTask {
private:
  StaticTask_t xTaskBuffer;
  StackType_t xStack[STACK_SIZE]; // long unsigned int
public:
  TaskHandle_t handle = NULL;
  bool create(TaskFunction_t pxTaskCode, const char * const pcName,
	      UBaseType_t uxPriority){
    if(handle != NULL)
      vTaskDelete(handle);
    handle = xTaskCreateStatic(pxTaskCode, pcName, STACK_SIZE, NULL, uxPriority, xStack, &xTaskBuffer);
    return handle != NULL;
  }
  void notify(uint32_t ulValue){
    if(handle != NULL)
      xTaskNotify(handle, ulValue, eSetBits );
  }
  void notifyFromISR(uint32_t ulValue){
    BaseType_t xHigherPriorityTaskWoken = 0; 
    if(handle != NULL)
      xTaskNotifyFromISR(handle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
};

StaticTask<MANAGER_TASK_STACK_SIZE> managerTask;
StaticTask<UTILITY_TASK_STACK_SIZE> utilityTask;

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
    uint8_t index = flashSectorToWrite;
    uint32_t size = flashSizeToWrite;
    uint8_t* source = (uint8_t*)flashAddressToWrite;
    if(index == 0xff && size < MAX_SYSEX_FIRMWARE_SIZE){
      flashFirmware(source, size);
    }else{
      registry.store(index, source, size);
    }
    midi.sendProgramMessage();
    midi.sendDeviceStats();
    vTaskDelete(NULL);
    utilityTask.handle = NULL;
  }

  void eraseFlashTask(void* p){
    int sector = flashSectorToWrite;
    if(sector == 0xff){
      storage.erase();
      debugMessage("Erased flash storage");
      registry.init();
    }
    midi.sendProgramMessage();
    midi.sendDeviceStats();
    vTaskDelete(NULL);
    utilityTask.handle = NULL;
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
    vTaskDelete(NULL);
    utilityTask.handle = NULL;
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
  audioStatus = AUDIO_READY_STATUS;
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
  while(audioStatus != AUDIO_READY_STATUS);
  audioStatus = AUDIO_PROCESSING_STATUS;
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
  bool ret = managerTask.create(runManagerTask, "Manager", MANAGER_TASK_PRIORITY);
  if(!ret)
    error(PROGRAM_ERROR, "Failed to start manager task");
}

void ProgramManager::notifyManagerFromISR(uint32_t ulValue){
  managerTask.notifyFromISR(ulValue);
}

void ProgramManager::notifyManager(uint32_t ulValue){
  managerTask.notify(ulValue);
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
  if(def != NULL && def->getProgramVector() != NULL){
    currentpatch = def;
    updateProgramIndex(pid);
  }
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  dynamo.load(address, length);
  if(dynamo.getProgramVector() != NULL){
    currentpatch = &dynamo;
    registry.setDynamicPatchDefinition(currentpatch);
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
  if(currentpatch != NULL)
    ss = currentpatch->getStackSize();
  if(ss == 0)
    ss = PROGRAM_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
  return ss;
}

uint32_t ProgramManager::getManagerStackUsed(){
  if(managerTask.handle == NULL)
    return 0;
  uint32_t mh = uxTaskGetStackHighWaterMark(managerTask.handle);
  return getManagerStackAllocation() - mh*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getManagerStackAllocation(){
  return MANAGER_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
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
		    xMaxBlockTime); 
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop program
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
    if(ulNotifiedValue & PROGRAM_FLASH_NOTIFICATION){ // program flash
      bool ret = utilityTask.create(programFlashTask, "Flash Write", FLASH_TASK_PRIORITY);
      if(!ret)
	error(PROGRAM_ERROR, "Failed to start Flash Write task");
    }else if(ulNotifiedValue & ERASE_FLASH_NOTIFICATION){ // erase flash
      bool ret = utilityTask.create(eraseFlashTask, "Flash Erase", FLASH_TASK_PRIORITY);
      if(!ret)
      	error(PROGRAM_ERROR, "Failed to start Flash Erase task");
#ifdef BUTTON_PROGRAM_CHANGE
    }else if(ulNotifiedValue & PROGRAM_CHANGE_NOTIFICATION){ // program change
      bool ret = utilityTask.create(programChangeTask, "Program Change", PC_TASK_PRIORITY);
      if(!ret)
	error(PROGRAM_ERROR, "Failed to start Program Change task");
#endif /* BUTTON_PROGRAM_CHANGE */
    }
    vTaskDelay(20);
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
	}
	if(xProgramHandle == NULL)
	  error(PROGRAM_ERROR, "Failed to start program task");
      }
      // todo: make sure no two tasks are using the same stack
    }
  }
}

void ProgramManager::eraseFromFlash(uint8_t sector){
  flashSectorToWrite = sector;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|ERASE_FLASH_NOTIFICATION);
}

void ProgramManager::saveToFlash(uint8_t sector, void* address, uint32_t length){
  flashSectorToWrite = sector;
  flashAddressToWrite = address;
  flashSizeToWrite = length;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|PROGRAM_FLASH_NOTIFICATION);
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
