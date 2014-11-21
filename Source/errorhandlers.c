#include "device.h"
#include "owlcontrol.h"
#include <inttypes.h>

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line){ 
  volatile uint32_t delayCounter;
  /* Blink LEDs */
  setLed(RED);
  for(delayCounter = 0; delayCounter < 4000000; delayCounter++);
  setLed(NONE);
  for(delayCounter = 0; delayCounter < 4000000; delayCounter++);
  setLed(RED);
  for(;;);
  /* NVIC_SystemReset(); */
}

/* exception handlers - so we know what's failing */
void NMI_Handler(void){
  assert_failed(0, 0);
}

void HardFault_Handler(void){ 
  assert_failed(0, 0);
}

void MemManage_Handler(void){ 
  assert_failed(0, 0);
}

void BusFault_Handler(void){ 
  assert_failed(0, 0);
}

void UsageFault_Handler(void){ 
  for(;;);
}

void SVC_Handler(void){ 
  for(;;);
}

void DebugMon_Handler(void){ 
  for(;;);
}

void PendSV_Handler(void){ 
  for(;;);
}
