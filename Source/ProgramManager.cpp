#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "SharedMemory.h"
#include "owlcontrol.h"

#include "FreeRTOS.h"
#include "task.h"

typedef void (*ProgramFunction)(void);

ProgramManager program;

TaskHandle_t xHandle = NULL;

extern "C" {
  void runTask(void* p){
    program.run();
  }
}

void ProgramManager::exit(){
  doRunProgram = false;
  getSharedMemory()->status = AUDIO_EXIT_STATUS;
  setLed(RED);
  stop();
}

void ProgramManager::start(){
  if(xHandle == NULL){
    // Create a task
    uint8_t ret = xTaskCreate(runTask, "OWL", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vPortYield(); // can we call this from an interrupt?
  }
  getSharedMemory()->status = AUDIO_IDLE_STATUS;
  doRunProgram = true;
}

void ProgramManager::stop(){
  // Use the handle to delete the task.
  if(xHandle != NULL){
    vTaskDelete(xHandle);
    xHandle = NULL;
    vPortYield(); // can we call this from an interrupt?
  }
}

/* exit and restart program */
void ProgramManager::reset(){
  exit();
  doRestartProgram = true;
}

void ProgramManager::load(void* address, uint32_t length){
  programAddress = (uint8_t*)address;
  programLength = length;
  doCopyProgram = true;
}

bool ProgramManager::verify(){
  if(*(uint32_t*)programAddress != 0xDADAC0DE)
    return false;
  return true;
}

void ProgramManager::run(){
  for(;;){
    if(doRunProgram){
      doRunProgram = false;
      if(doCopyProgram){
	doCopyProgram = false;
	/* copy patch to ram */
	// memcpy((void*)PATCHRAM, (void*)(programAddress+4), programLength-4);
	memcpy((void*)PATCHRAM, (void*)(programAddress), programLength);
      }
      /* Jump to patch */

      /*
    __set_CONTROL(0x3); // Switch to use Process Stack, unprivilegedstate
    __ISB(); // Execute ISB after changing CONTROL (architecturalrecommendation)
      */

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
	jumpToApplication();
	// where is our stack pointer now?
	/* reset Stack Pointer to pre-program state */
	// __set_MSP(msp);
	// program has returned
	getSharedMemory()->status = AUDIO_IDLE_STATUS;
	running = false;
	if(doRestartProgram){
	  doRestartProgram = false;
	  doRunProgram = true;
	}
      }
    }
  }
}
