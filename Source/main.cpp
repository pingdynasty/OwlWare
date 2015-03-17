#include <stdint.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ProgramManager.h"

extern "C" {
  // void dumpTaskStats(){
  //   char buf[5*40]; // 40 bytes per task
  //   vTaskGetRunTimeStats(buf);
  // }
  void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for(;;);
  }
  void vApplicationIdleHook(void) {
  }
  void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for(;;);
  }
}

int main(void){
  /*   SRAM_Init(); called in system_hsx.c before interrupts are enabled */
  // NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  // NVIC_SetPriorityGrouping(0);

  // DISDEFWBUF : disable write buffer (to track down BusFault IMPRECISERR faults)
  // SCnSCB->ACTLR |= 2; 

  program.startManager(); // start the program manager task
  vTaskStartScheduler();  // should never return
  for (;;);
}
