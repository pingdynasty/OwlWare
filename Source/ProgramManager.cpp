#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "SharedMemory.h"
#include "owlcontrol.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef void (*ProgramFunction)(void);

ProgramManager program;

TaskHandle_t xProgramHandle = NULL;
TaskHandle_t xManagerHandle = NULL;
SemaphoreHandle_t xSemaphore = NULL;

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02

#define MANAGER_STACK_SIZE          (configMINIMAL_STACK_SIZE*15)
#define PROGRAM_STACK_SIZE          (configMINIMAL_STACK_SIZE*15)

extern "C" {
  void runProgramTask(void* p){
    program.runProgram();
  }
  void runManagerTask(void* p){
    program.runManager();
  }
}

unsigned long pH = 0;
unsigned long mH = 0;
unsigned long tH = 0;

void stats(){
  UBaseType_t high;
  if(xProgramHandle != NULL){
    high = uxTaskGetStackHighWaterMark(xProgramHandle);
    if(high > pH)
      pH = high;
  }
  if(xManagerHandle != NULL){
    high = uxTaskGetStackHighWaterMark(xManagerHandle);
    if(high > mH)
      mH = high;
  }
  high = uxTaskGetNumberOfTasks();
  if(high > tH)
    tH = high;
}

void ProgramManager::audioReady(){
  // if(xProgramHandle != NULL){
  //   BaseType_t xHigherPriorityTaskWoken = 0; 
  //   xHigherPriorityTaskWoken = xTaskResumeFromISR(xProgramHandle);
  //   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  // }
  signed long int xHigherPriorityTaskWoken = pdFALSE; 
  // signed BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
  // xSemaphoreGiveFromISR(xSemaphore, NULL);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ProgramManager::programReady(){
  // vTaskSuspend(xProgramHandle);
  xSemaphoreTake(xSemaphore, 0);
}

void ProgramManager::programStatus(int){
  for(;;);
}

void ProgramManager::startManager(){
  if(xManagerHandle == NULL)
    xTaskCreate(runManagerTask, "Manager", MANAGER_STACK_SIZE, NULL, 4, &xManagerHandle);
  if(xSemaphore == NULL)
    xSemaphore = xSemaphoreCreateBinary();
}

void ProgramManager::start(){
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
  uint32_t sp = *(bin+1); // stack pointer
  uint32_t ld = *(bin+3); // link base address
  uint32_t jumpAddress = *(bin+2); // main pointer
  if((sp & 0x2FFE0000) == 0x20000000 && ld == PATCHRAM){
    ProgramFunction jumpToApplication = (ProgramFunction)jumpAddress;
    running = true;
    setLed(GREEN);
    jumpToApplication();
    // program has returned
  }else{
    setLed(RED);
  }
  getSharedMemory()->status = AUDIO_IDLE_STATUS;
  running = false;
  vTaskSuspend(NULL);
  for(;;); // wait to be killed
  // if(xProgramHandle != NULL){
  //   vTaskDelete(NULL); // delete ourselves
  //   xProgramHandle = NULL;
  // }
}

void ProgramManager::runManager(){
  uint32_t ulNotifiedValue = 0;
  TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
  // TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  setLed(GREEN);
  for(;;){

    stats();    

    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,      /* Don't clear any notification bits on entry. */
		    UINT32_MAX, /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime ); 

    stats();    

    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop      
      if(xProgramHandle != NULL){
	vTaskDelete(xProgramHandle);
	xProgramHandle = NULL;
	running = false;
      }
    }
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start      
#ifdef USE_FREERTOS_MPU
      static portSTACK_TYPE xTaskStack[512] __attribute__((aligned(512*4))) CCM;
      static const TaskParameters_t xTaskDefinition = {
	runProgramTask,  /* pvTaskCode */
	"Program",        /* pcName */
	512,             /* usStackDepth - defined in words, not bytes. */
	NULL,            /* pvParameters */
	2,               /* uxPriority - priority 1, start in User mode. */
	xTaskStack,      /* puxStackBuffer - the array to use as the task stack. */
	/* xRegions */
	{
	  /* Base address   Length                    Parameters */
	  { PATCHRAM,       80*1024,                  portMPU_REGION_READ_WRITE },
	  { EXTRAM,	    1024*1024,                portMPU_REGION_READ_WRITE },
	  { 0,	            0,                        0                         },
	}
      };
      if(xProgramHandle == NULL)
	xTaskCreateRestricted( &xTaskDefinition, &xProgramHandle );
#else
      if(xProgramHandle == NULL)
	xTaskCreate(runProgramTask, "Program", configMINIMAL_STACK_SIZE*4, NULL, 2, &xProgramHandle);
#endif /* USE_FREERTOS_MPU */
    }
  }
}

/*
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
  __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
*/
