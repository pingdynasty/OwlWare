#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "SharedMemory.h"
#include "owlcontrol.h"

#include "FreeRTOS.h"
#include "task.h"

typedef void (*ProgramFunction)(void);

ProgramManager program;

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define RESET_PROGRAM_NOTIFICATION  0x04

#define MANAGER_STACK_SIZE          (configMINIMAL_STACK_SIZE*5)
#define PROGRAM_STACK_SIZE          (configMINIMAL_STACK_SIZE*5)

extern "C" {
  void runProgramTask(void* p){
    program.runProgram();
  }
  void runManagerTask(void* p){
    program.runManager();
  }
}

void ProgramManager::startManager(){
  xTaskCreate(runManagerTask, "Manager", MANAGER_STACK_SIZE, NULL, 4, &xManagerHandle);
}

void ProgramManager::start(){
  getSharedMemory()->status = AUDIO_IDLE_STATUS;
  // doRunProgram = true;
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
  // doStopProgram = true;
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
  uint32_t ulValue = RESET_PROGRAM_NOTIFICATION;
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(xManagerHandle != NULL)
    xTaskNotifyFromISR(xManagerHandle, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void ProgramManager::load(void* address, uint32_t length){
  programAddress = (uint8_t*)address;
  programLength = length;
  /* copy program to ram */
  memcpy((void*)PATCHRAM, (void*)(programAddress), programLength);
}

bool ProgramManager::verify(){
  if(*(uint32_t*)programAddress != 0xDADAC0DE)
    return false;
  return true;
}

void ProgramManager::runProgram(){
  /* Jump to program */
  /* Check Vector Table: Test if user code is programmed starting from address 
     "APPLICATION_ADDRESS" */
  volatile uint32_t* bin = (volatile uint32_t*)PATCHRAM;
  uint32_t sp = *(bin+1);
  if((sp & 0x2FFE0000) == 0x20000000){
    /* store Stack Pointer before jumping */
    // msp = __get_MSP();
    uint32_t jumpAddress = *(bin+2); // (volatile uint32_t*)(PATCHRAM + 8);
    ProgramFunction jumpToApplication = (ProgramFunction)jumpAddress;
    /* Initialize user application Stack Pointer */
    // __set_MSP(sp); // volatile uint32_t*)PATCHRAM);
    running = true;
    setLed(GREEN);
    jumpToApplication();
    // where is our stack pointer now?
    /* reset Stack Pointer to pre-program state */
    // __set_MSP(msp);
    // program has returned
  }else{
    setLed(RED);
  }
  getSharedMemory()->status = AUDIO_IDLE_STATUS;
  vTaskDelete(NULL); // delete ourselves
  xProgramHandle = NULL;
  running = false;
}

void ProgramManager::runManager(){
  uint32_t ulNotifiedValue = 0;
  bool doStopProgram = false;
  bool doRunProgram = false;
  bool doRestartProgram = false;

  TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
  // TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  for(;;){
    
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,      /* Don't clear any notification bits on entry. */
		    UINT32_MAX, /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime ); 

    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION) // start
      doRunProgram = true;
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION) // stop
      doStopProgram = true;
    if(ulNotifiedValue & RESET_PROGRAM_NOTIFICATION){ // restart
      doStopProgram = true;
      doRestartProgram = true;
    }

    if(doRunProgram){
      doRunProgram = false;
      if(xProgramHandle == NULL)
	xTaskCreate(runProgramTask, "Program", configMINIMAL_STACK_SIZE*4, NULL, 2, &xProgramHandle);
    }
    if(doStopProgram){
      doStopProgram = false;
      // Use the handle to delete the task.
      if(xProgramHandle != NULL){
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
	running = false;
      }
      if(doRestartProgram){
	doRunProgram = true;
	doRestartProgram = false;
      }
    }
  }
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
