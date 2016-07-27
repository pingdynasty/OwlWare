#include "armcontrol.h"
#include "usbcontrol.h"
#include "device.h"

/**
 * Configure analogue inputs using ADC3 and DMA2 stream 0
 */
void adcSetupDMA(int16_t* dma){
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;

  uint8_t channels = NOF_ADC_VALUES;

  DMA_DeInit(DMA2_Stream0);
  DMA_StructInit(&DMA_InitStructure);

  /* Enable required clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* DMA2 Stream0 channel0 configuration */
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&ADC3->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dma;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = channels;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* Configure ADC pins as analog inputs */
  configureAnalogInput(GPIOC, GPIO_Pin_0);
  configureAnalogInput(GPIOC, GPIO_Pin_1);
  configureAnalogInput(GPIOC, GPIO_Pin_2);
  configureAnalogInput(GPIOC, GPIO_Pin_3);

#if defined EXPRESSION_PEDAL
  /* Configure expression pedal pin (PA2 or PA3) as analog input */
  configureAnalogInput(EXPRESSION_PEDAL_RING_PORT, EXPRESSION_PEDAL_RING_PIN);
/*   configureAnalogInput(GPIOA, GPIO_Pin_3); */
#endif

  ADC_DeInit();

  /* ADC Common Init */
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
/*   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_12Cycles; */
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init */
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = channels;
  ADC_Init(ADC3, &ADC_InitStructure);

/*   uint8_t sampletime = ADC_SampleTime_3Cycles; */
/*   uint8_t sampletime = ADC_SampleTime_15Cycles; */
  uint8_t sampletime = ADC_SampleTime_84Cycles;

  /* ADC3 regular channel configuration */
  /* Control Pots A-D, PC0-3 */
  ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 1, sampletime);
  ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 2, sampletime);
  ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 3, sampletime);
  ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 4, sampletime);

#if defined EXPRESSION_PEDAL
  /* Expression Pedal Input PA2/3, ADC123_IN2-3 */
  ADC_RegularChannelConfig(ADC3, EXPRESSION_PEDAL_RING_CHANNEL, 5, sampletime);
/*   ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 5, sampletime); */
#endif

  /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);

  /* Start ADC3 Software Conversion */ 
  ADC_SoftwareStartConv(ADC3);
}

void adcStartDMA(){
  DMA_Cmd(DMA2_Stream0, ENABLE);
}

void adcStopDMA(){
  DMA_Cmd(DMA2_Stream0, DISABLE);
}

uint16_t getSampleCounter(){
  // does not work: always returns values <= 5
  // return DMA_GetCurrDataCounter(DMA2_Stream0);
  // todo:
  volatile uint32_t *DWT_CYCCNT = (volatile uint32_t *)0xE0001004; //address of the register
  // reset every start of block (after lock in programReady())
  return (*DWT_CYCCNT)/3500;
}
