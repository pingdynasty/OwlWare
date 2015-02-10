#include <stdint.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

extern void setup();
extern void run();
extern "C" void runTask(void*);

int main(void){
  /*   SRAM_Init(); called in system_hsx.c before interrupts are enabled */

  setup();	

  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  // Create a task
  uint8_t ret = xTaskCreate(runTask, "OWL", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

  vTaskStartScheduler();  // should never return

  for (;;);
}

extern "C" {
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

  void runTask(void* p){
    run();
  }

}
