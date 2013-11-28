#include "device.h"
#include <inttypes.h>

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line){ 
  /* Infinite loop */
  for(;;);
}

/* exception handlers - so we know what's failing */
void NMI_Handler(void){ 
  for(;;);
}

void HardFault_Handler(void){ 
  for(;;);
}

void MemManage_Handler(void){ 
  for(;;);
}

void BusFault_Handler(void){ 
  for(;;);
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
