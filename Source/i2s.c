#include "i2s.h"
#include "stm32f4xx.h"
#include "codec.h"
#include "device.h"

int16_t *txbuf;
int16_t *rxbuf;
uint16_t szbuf;

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
void I2S_Block_Init(int16_t *tx, int16_t *rx, uint16_t blocksize){ 
  DMA_InitTypeDef DMA_InitStructure;
  /* save for IRQ svc  */
  txbuf = tx;
  rxbuf = rx;
  /* szbuf is the size in halfwords of one block; half of the buffer */
#if AUDIO_BITDEPTH == 16
  szbuf = blocksize*AUDIO_CHANNELS;
#else
  szbuf = blocksize*AUDIO_CHANNELS*2;
#endif

  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE); 

  /* Configure the TX DMA Stream */
  DMA_StructInit(&DMA_InitStructure);
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_DMA_STREAM);
  /* Set the parameters to be configured */
  DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
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
  /* Configure the tx buffer address and size */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txbuf;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)szbuf*2;
  DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);  
	
  /* Enable the I2S DMA request */
  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

  /* Configure the RX DMA Stream */
  DMA_StructInit(&DMA_InitStructure);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);
	
  /* Set the parameters to be configured */
  DMA_InitStructure.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  /* Configure the rx buffer address and size */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rxbuf;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)szbuf*2;
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure);  
	
  /* Enable the Half & Complete DMA interrupts  */
  DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);
    
  /* I2S DMA IRQ Channel configuration */
  NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

  /* Enable the I2S DMA request */
  SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);
}

void I2S_Run(){
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
 * handle I2S RX DMA block interrupts
 */
__attribute__ ((section (".coderam")))
void DMA1_Stream3_IRQHandler(void){ 
  if(DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET) {
    /* Transfer complete interrupt */
    /* Handle 2nd half */  
    audioCallback(rxbuf + szbuf, txbuf + szbuf, szbuf);    
    /* Clear the Interrupt flag */
    DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);
  }else if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT) != RESET) {
    /* Half Transfer complete interrupt */
    /* Handle 1st half */  
    audioCallback(rxbuf, txbuf, szbuf);    
    /* Clear the Interrupt flag */
    DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT);    
  }
}
