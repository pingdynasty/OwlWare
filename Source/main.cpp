#include <stdint.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "owlcontrol.h"

#include "ProgramManager.h"

extern "C" {
  void vApplicationMallocFailedHook(void) {
    // taskDISABLE_INTERRUPTS();
    // for(;;);
    exitProgram(false);
    setErrorMessage(PROGRAM_ERROR, "malloc failed");
  }
  void vApplicationIdleHook(void) {
  }
  void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
    exitProgram(false);
    setErrorMessage(PROGRAM_ERROR, "stack overflow");
    // taskDISABLE_INTERRUPTS();
    // for(;;);
  }
}

int main(void){
  /*   SRAM_Init(); called in system_hsx.c before interrupts are enabled */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk; // enable MemManageFault
  NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk; // enable BusFault
  NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));
  SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk; // enable UsageFault
  NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));

#ifdef DEBUG
  // DISDEFWBUF : disable write buffer (to track down BusFault IMPRECISERR faults)
  SCnSCB->ACTLR |= 2; 
#endif

  program.startManager(); // start the program manager task

  vTaskStartScheduler();  // should never return
  for (;;);
}
