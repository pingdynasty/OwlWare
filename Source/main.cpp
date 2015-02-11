#include <stdint.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ProgramManager.h"

extern void setup();
extern void run();

extern "C" {

  // void runManagerTask(void* p){
  //   program.runManager();
  // }

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

  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  NVIC_SetPriorityGrouping(0);

  setup();	


  // program.start(); // schedules first program task
  program.startManager();

  vTaskStartScheduler();  // should never return

  for (;;);
}
