
#include "bkp_sram.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"

void BKPSRAM_Init(void) {
  /* Enable PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
  /* Enable backup SRAM Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	
  /* Allow access to backup domain */
  PWR_BackupAccessCmd(ENABLE);
	
  /* Enable the Backup SRAM low power Regulator */
  /* This will allow data to stay when using VBat mode */
  PWR_BackupRegulatorCmd(ENABLE);
	
  /* Wait for backup regulator to be ready  */
  while (PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET);
}

void* BKPSRAM_GetMemoryAddress(){
  return (void*)BKPSRAM_BASE;
}

uint32_t BKPSRAM_GetMemorySize(){
  return 0x00001000;
}
