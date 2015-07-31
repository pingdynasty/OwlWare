#include "clock.h"
#include "device.h"
#include "stm32f4xx.h"

#ifndef DEFINE_OWL_SYSTICK
#include "FreeRTOS.h"
#endif

volatile uint32_t systicks = 0;

void clockSetup(){
#ifdef DEFINE_OWL_SYSTICK
  /* Configures SysTick to be called every ms */
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000); // divide by 100 for 10ms period
#endif /* DEFINE_OWL_SYSTICK */
}

#ifdef DEFINE_OWL_SYSTICK
void SysTick_Handler(void){
  systicks++;
#else
// FreeRTOS callback
void vApplicationTickHook(void) {
  systicks ++;
  /* systicks += portTICK_PERIOD_MS; */
}
#endif /* DEFINE_OWL_SYSTICK */
