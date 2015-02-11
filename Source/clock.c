#include "clock.h"
#include "stm32f4xx.h"

/* __IO uint32_t TimingDelay = 0; */
__IO uint32_t systicks = 0;

void clockSetup(){
  /* /\* Configures SysTick to be called every ms *\/ */
  /* RCC_ClocksTypeDef RCC_Clocks; */
  /* RCC_GetClocksFreq(&RCC_Clocks); */
  /* SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000); // divide by 100 for 10ms period */
}

// FreeRTOS callback
void vApplicationTickHook(void) {
  systicks++;
}

/* /\** */
/*   * @param  ms: specifies the delay time length in milliseconds */
/*   *\/ */
/* void delay(__IO uint32_t ms){ */
/*   TimingDelay = ms; */
/*   while(TimingDelay != 0); */
/* } */

uint32_t getSysTicks(){
  return systicks;
}

#ifdef DEFINE_OWL_SYSTICK
void SysTick_Handler(void){
  /* if(TimingDelay != 0x00) */
  /*   TimingDelay--; */
  systicks++;
}
#endif /* DEFINE_OWL_SYSTICK */
