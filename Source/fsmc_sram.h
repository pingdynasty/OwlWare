#ifndef __FSMC_SRAM_H
#define __FSMC_SRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

void SRAM_Init(void);
void SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);
void SRAM_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);
/* return total configured memory size in bytes */
uint32_t SRAM_GetMemorySize();
void* SRAM_GetMemoryAddress();

uint32_t SRAM_Test(uint32_t addr, uint32_t NumHalfwordToWrite);
void SRAM_TestMemory();

#ifdef __cplusplus
}
#endif

#endif /* __FSMC_SRAM_H */
