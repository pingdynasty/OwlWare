#include "i2s.h"
#include "stm32f4xx.h"
#include "codec.h"

DMA_InitTypeDef DMA_InitStructure, DMA_InitStructure2;
uint32_t txbuf, rxbuf, szbuf;

void I2S_Pause(){
  /* Pause the I2S DMA Stream 
     Note. For the STM32F4xx devices, the DMA implements a pause feature, 
     by disabling the stream, all configuration is preserved and data 
     transfer is paused till the next enable of the stream.
     This feature is not available on STM32F1xx devices. */
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

void I2S_Resume(){
  /* Resume the I2S DMA Stream 
     Note. For the STM32F4xx devices, the DMA implements a pause feature, 
     by disabling the stream, all configuration is preserved and data 
     transfer is paused till the next enable of the stream.
     This feature is not available on STM32F1xx devices. */
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

/*
 * Init I2S channel for DMA with IRQ per block 
 */
void I2S_Block_Init(void){ 
  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE); 

  /* Configure the TX DMA Stream */
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_DMA_STREAM);
  /* Set the parameters to be configured */
  DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_DMA_PERIPH_DATA_SIZE;
  DMA_InitStructure.DMA_MemoryDataSize = AUDIO_DMA_MEM_DATA_SIZE; 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);  
	
  /* Enable the I2S DMA request */
  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

  /* Configure the RX DMA Stream */
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);
	
  /* Set the parameters to be configured */
  /* why is a separate initstructure needed here? */
  DMA_InitStructure2.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;  
  DMA_InitStructure2.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
  DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
  DMA_InitStructure2.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure2.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
  DMA_InitStructure2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure2.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure2.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
  DMA_InitStructure2.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure2.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure2.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure2.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure2.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);  
	
  /* Enable the Half & Complete DMA interrupts  */
  DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);
    
  /* I2S DMA IRQ Channel configuration */
  NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

  /* Enable the I2S DMA request */
  SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);
}

void I2S_Block_Run(uint32_t txAddr, uint32_t rxAddr, uint32_t Size){
  /* save for IRQ svc  */
  txbuf = txAddr;
  rxbuf = rxAddr;
  szbuf = Size;
	
  /* Configure the tx buffer address and size */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txAddr;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;

  /* Configure the DMA Stream with the new parameters */
  DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);

  /* Configure the rx buffer address and size */
  /* Again with the separate initstructure. Baroo?? */
  DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)rxAddr;
  DMA_InitStructure2.DMA_BufferSize = (uint32_t)Size;

  /* Configure the DMA Stream with the new parameters */
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);

  /* Enable the I2S DMA Streams */
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);   
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);   

}

void I2S_Disable(){
  I2S_Cmd(CODEC_I2S, DISABLE);
}

void I2S_Enable(){
  /* If the I2S peripheral is still not enabled, enable it */
  if ((CODEC_I2S->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S, ENABLE);
  }
  if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S_EXT, ENABLE);
  }
}

/**
 * @brief  This function handles I2S RX DMA block interrupt. 
 * @param  None
 * @retval none
 */
__attribute__ ((section (".coderam")))
void DMA1_Stream3_IRQHandler(void){ 
  uint16_t *src, *dst, sz;
	
  /* Transfer complete interrupt */
  if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET)
    {
      /* Point to 2nd half of buffers */
      sz = szbuf/2;
      src = (uint16_t *)(rxbuf) + sz;
      dst = (uint16_t *)(txbuf) + sz;
		
      /* Handle 2nd half */  
      audioCallback(src, dst, sz);    

      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);
    }

  /* Half Transfer complete interrupt */
  if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT) != RESET)
    {
      /* Point to 1st half of buffers */
      sz = szbuf/2;
      src = (uint16_t *)(rxbuf);
      dst = (uint16_t *)(txbuf);

      /* Handle 1st half */  
      audioCallback(src, dst, sz);    

      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT);    
    }
}
