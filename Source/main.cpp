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

// #include "device.h"
// extern "C" void program_run();
// TaskHandle_t xStaticProgramHandle = NULL;
// void runStaticProgramTask(void* p){
//   program_run();
//   assert_failed(0, 0);
//   for(;;);
// }

int main(void){
  /*   SRAM_Init(); called in system_hsx.c before interrupts are enabled */
  // NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  // NVIC_SetPriorityGrouping(0);

// #ifdef DEBUG
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

  // uint32_t* programStackBase = (uint32_t*)PATCHRAM;
  // uint32_t programStackSize = 32*1024;
  // xTaskGenericCreate(runStaticProgramTask, "Static", programStackSize/sizeof(portSTACK_TYPE), NULL, 2, &xStaticProgramHandle, programStackBase, NULL);

  // xTaskCreate(runStaticProgramTask, "Static", programStackSize/sizeof(portSTACK_TYPE), NULL, 2, &xStaticProgramHandle);

  vTaskStartScheduler();  // should never return
  for (;;);
}
