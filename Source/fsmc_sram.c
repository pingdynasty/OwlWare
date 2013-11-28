#include <stdbool.h>
#include "fsmc_sram.h"
#include "stm32f4xx.h"
#include <string.h> /* for memset */

#define Bank1_SRAM1_ADDR  ((uint32_t)0x60000000)  
#define Bank1_SRAM2_ADDR  ((uint32_t)0x64000000)  
#define Bank1_SRAM3_ADDR  ((uint32_t)0x68000000)  
#define Bank1_SRAM4_ADDR  ((uint32_t)0x6c000000)  

uint32_t SRAM_GetMemorySize(){
  return 1024*1024;
}

void* SRAM_GetMemoryAddress(){
  return (void*)Bank1_SRAM3_ADDR;
}

void SRAM_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 

/*   FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM3); */
  
  /* Enable GPIOs clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG, ENABLE);

  /* Enable FSMC clock */
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE); 
  
/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/

/* OWL:
PD0   D2
PD1   D3
PD4   *OE / FSMC_NOE
PD5   *WE / FSMC_NWE
PD8   D13
PD9   D14
PD10  D15
PD11  A16
PD12  A17
PD13  A18
PD14  D0
PD15  D1

PE0   *LB / FSMC_NBL0
PE1   *UB / FSMC_NBL1
--- PE2 to PE6 nc --
PE7   D4
PE8   D5
PE9   D6
PE10  D7
PE11  D8
PE12  D9
PE13  D10
PE14  D11
PE15  D12

PF0   A0
PF1   A1
PF2   A2
PF3   A3
PF4   A4
PF5   A5

PF12  A6
PF13  A7
PF14  A8
PF15  A9

PG0   A10
PG1   A11
PG2   A12
PG3   A13
PG4   A14
PG5   A15
-- PG9 nc --
PG10  CE / FSMC_NE3 (10k pull-up)

*/

  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  | 
                                GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                                GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOD, &GPIO_InitStructure);


  /* GPIOE configuration */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
  /*  PE2 to PE6 NC */
/*   GPIO_PinAFConfig(GPIOE, GPIO_PinSource2 , GPIO_AF_FSMC); */
/*   GPIO_PinAFConfig(GPIOE, GPIO_PinSource3 , GPIO_AF_FSMC); */
/*   GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC); */
/*   GPIO_PinAFConfig(GPIOE, GPIO_PinSource5 , GPIO_AF_FSMC); */
/*   GPIO_PinAFConfig(GPIOE, GPIO_PinSource6 , GPIO_AF_FSMC); */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |  
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_6  | GPIO_Pin_7 |
                                GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11|
                                GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOE, &GPIO_InitStructure);


  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | 
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;      

  GPIO_Init(GPIOF, &GPIO_InitStructure);


  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource3 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FSMC);
  /* PG9 NC */
/*   GPIO_PinAFConfig(GPIOG, GPIO_PinSource9 , GPIO_AF_FSMC); */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10 , GPIO_AF_FSMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 | 
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_10;      
/*                                 GPIO_Pin_4  | GPIO_Pin_5  |GPIO_Pin_9;       */

  GPIO_Init(GPIOG, &GPIO_InitStructure);

  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;
  FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

/*-- FSMC Configuration ------------------------------------------------------*/
/*   p.FSMC_AddressSetupTime = 3; */
/*   p.FSMC_AddressHoldTime = 0; */
/*   p.FSMC_DataSetupTime = 6; */
/*   p.FSMC_BusTurnAroundDuration = 1; */
/*   p.FSMC_CLKDivision = 0; */
/*   p.FSMC_DataLatency = 0; */
/*   p.FSMC_AccessMode = FSMC_AccessMode_A; */

  /* from AN2784 Application note
     Address setup time:            0x0
     Data setup time:               0x1
     Write cycle time:             12ns
     Read cycle time:              12ns
     Write Enable low pulse width:  8ns
     Address access time:          12ns
  */

  p.FSMC_AddressSetupTime = 0; // FSMC_AddressSetupTime <= 0x0f
  p.FSMC_AddressHoldTime = 0; // FSMC_AddressHoldTime <= 0x0f
  p.FSMC_DataSetupTime = 1; // FSMC_DataSetupTime must be > 0 and <= 0xff
  p.FSMC_BusTurnAroundDuration = 0; // FSMC_BusTurnAroundDuration <= 0x0f
  p.FSMC_CLKDivision = 0; // <= 0x0f
  p.FSMC_DataLatency = 0; // <= 0x0f
  p.FSMC_AccessMode = FSMC_AccessMode_A;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
/*   FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_PSRAM; */
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /*!< Enable FSMC Bank1_SRAM3 Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE); 
}

/**
  * @brief  Writes a Half-word buffer to the FSMC SRAM memory.
  * @param  pBuffer : pointer to buffer.
  * @param  WriteAddr : SRAM memory internal address from which the data will be
  *         written.
  * @param  NumHalfwordToWrite : number of half-words to write.
  * @retval None
  */
void SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
  for (; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
  {
    /* Transfer data to the memory */
    *(uint16_t *) (Bank1_SRAM3_ADDR + WriteAddr) = *pBuffer++;

    /* Increment the address*/
    WriteAddr += 2;
  }
}

/**
  * @brief  Reads a block of data from the FSMC SRAM memory.
  * @param  pBuffer : pointer to the buffer that receives the data read from the
  *         SRAM memory.
  * @param  ReadAddr : SRAM memory internal address to read from.
  * @param  NumHalfwordToRead : number of half-words to read.
  * @retval None
  */
void SRAM_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
  for (; NumHalfwordToRead != 0; NumHalfwordToRead--) /* while there is data to read */
  {
    /* Read a half-word from the memory */
    *pBuffer++ = *(__IO uint16_t*) (Bank1_SRAM3_ADDR + ReadAddr);

    /* Increment the address*/
    ReadAddr += 2;
  }
}

uint32_t SRAM_Test(uint32_t addr, uint32_t NumHalfwordToWrite){
  uint16_t tx[256];
  uint16_t rx[256];
  for(int i=0; i<256; ++i)
    tx[i] = (i/256.0f)*0xffff;
  uint32_t result = 0;
  for(uint32_t i=0; i<NumHalfwordToWrite; i+=256){
    SRAM_WriteBuffer(tx, addr+i, 256);
    memset(rx, 0, sizeof(rx));
    SRAM_ReadBuffer(rx, addr+i, 256);
    for(int x=0; x<256; ++x){
      if(rx[x] != tx[x])
	result++;
    }
  }
  return result;
}

union ufloat {
  float f;
  uint32_t u;
};

uint32_t SRAM_TestFloat(uint32_t addr, uint32_t sz){
  float tx[16];
  float rx[16];
  for(int i=0; i<16; ++i)
    tx[i] = i/8.0f - 1.0;
  uint32_t result = 0;
  union ufloat utx;
  union ufloat urx;
  for(int i=0; i<sz; i+=16){
    SRAM_WriteBuffer((uint16_t*)tx, addr+i, 16*sizeof(float)/2);
    memset(rx, 0, sizeof(rx));
    SRAM_ReadBuffer((uint16_t*)rx, addr+i, 16*sizeof(float)/2);
    for(int x=0; x<16; ++x){
      utx.f = tx[i];
      urx.f = rx[i];
/*       if(rx[x] != tx[x]) */
      if(urx.u != utx.u)
	result++;
    }
  }
  return result;
}

uint32_t SRAM_StraightFloat(uint32_t addr, uint32_t sz){
  float tx, rx;
  union ufloat utx, urx;
  uint32_t result = 0;
  __IO float* ptr = (__IO float*) (Bank1_SRAM3_ADDR + addr);
  for(uint32_t i=0; i<sz; ++i){
    tx = 2.0f*i/sz - 1.0f;
    utx.f = tx;
    *ptr = tx;
    rx = *ptr;
    urx.f = rx;
    if(urx.u != utx.u)
      result++;
    ptr++;
  }
  return result;
}

void SRAM_TestMemory(){
  assert_param(SRAM_Test(0, 512*1024) == 0);
}
