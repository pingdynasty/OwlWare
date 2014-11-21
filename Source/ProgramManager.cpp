#include <string.h>
#include "stm32f4xx.h"
#include "ProgramManager.h"
#include "SharedMemory.h"
#include "owlcontrol.h"

#define PATCHRAM   ((uint32_t)0x20010000)
typedef void (*ProgramFunction)(void);

ProgramManager program;

void ProgramManager::exit(){
  doRunProgram = false;
  smem.status = AUDIO_EXIT_STATUS;
  setLed(RED);
}

/* exit and restart program */
void ProgramManager::reset(){
  exit();
  doRestartProgram = true;
}

void ProgramManager::load(void* address, uint32_t length){
  programAddress = address;
  programLength = length;
  doCopyProgram = true;
}

void ProgramManager::start(){
  smem.status = AUDIO_IDLE_STATUS;
  doRunProgram = true;
}

void ProgramManager::run(){
  for(;;){
    if(doRunProgram){
      doRunProgram = false;
      if(doCopyProgram){
	doCopyProgram = false;
	/* copy patch to ram */
	memcpy((void*)PATCHRAM, (void*)programAddress, programLength);
      }
      /* Jump to patch */
      /* Check Vector Table: Test if user code is programmed starting from address 
	 "APPLICATION_ADDRESS" */
      if(((*(volatile uint32_t*)PATCHRAM) & 0x2FFE0000 ) == 0x20000000){
	/* store Stack Pointer before jumping */
	msp = __get_MSP();
	uint32_t jumpAddress = *(volatile uint32_t*)(PATCHRAM + 4);
	ProgramFunction jumpToApplication = (ProgramFunction)jumpAddress;
	/* Initialize user application Stack Pointer */
	__set_MSP(*(volatile uint32_t*)PATCHRAM);
	running = true;
	jumpToApplication();
	// where is our stack pointer now?
	/* reset Stack Pointer to pre-program state */
	__set_MSP(msp);
	// program has returned
	smem.status = AUDIO_IDLE_STATUS;
	running = false;
	if(doRestartProgram){
	  doRestartProgram = false;
	  doRunProgram = true;
	}
      }
    }
  }
}
