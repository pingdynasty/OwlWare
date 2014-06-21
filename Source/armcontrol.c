#include "armcontrol.h"
#include "usbcontrol.h"
#include "device.h"

/**
 * Configure analogue inputs using ADC3 and DMA2 stream 0
 */
void adcSetupDMA(uint16_t* dma){
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

/** 
 * Configure DAC output
 */
void dacSetup(){
  DAC_InitTypeDef DAC_InitStructure;

  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* Configure PA.04 (DAC_OUT1) in analog mode */
  configureAnalogOutput(GPIOA, GPIO_Pin_4);
  /* Configure PA.05 (DAC_OUT2) in analog mode */
/*   configureAnalogOutput(GPIOA, GPIO_Pin_5); */

  /* Enable DAC clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  
  /* DAC channel Configuration */
  DAC_StructInit(&DAC_InitStructure);
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
/*   DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits10_0; */

  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
/*   DAC_Init(DAC_Channel_2, &DAC_InitStructure); */
  
  /* Enable DAC Channels */
  DAC_Cmd(DAC_Channel_1, ENABLE);
/*   DAC_Cmd(DAC_Channel_2, ENABLE); */
}

void setAnalogValue(uint8_t channel, uint16_t value){
/*   assert_param(channel == 0 || channel == 1); */
/*   assert_param((value & 0xfffff000) == 0); */
  value = value & 0xfff;
  if(channel == 0)
    DAC_SetChannel1Data(DAC_Align_12b_R, value);
  else if(channel == 1)
    DAC_SetChannel2Data(DAC_Align_12b_R, value);
}

/************ TIM ************/

void (*tim3Callback)();
void timerSetup(void (*f)(), uint16_t period){
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  uint16_t prescaler;

  /* assign callback */
  tim3Callback = f;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // lower value indicates higher priority
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Compute the prescaler value */
   /*  To get TIM3 counter clock at 1 MHz, the prescaler is computed as follows:
       Prescaler = (TIM3CLK / TIM3 counter clock) - 1
       Prescaler = ((SystemCoreClock /2) /1 MHz) - 1 */
  prescaler = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* TIM Interrupts enable */
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void){
  if(TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET){
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    (*tim3Callback)();
  }
}

void setTimerPrescaler(uint16_t prescaler){
  TIM_PrescalerConfig(TIM3, prescaler, TIM_PSCReloadMode_Immediate);
}

void setTimerPeriod(uint32_t period){
  TIM_SetAutoreload(TIM3, period);
}

void setPeriod(TIM_TypeDef* tim, uint32_t period){
  TIM_SetAutoreload(tim, period);
}

void setPrescaler(TIM_TypeDef* tim, uint32_t prescaler){
  TIM_PrescalerConfig(tim, prescaler, TIM_PSCReloadMode_Immediate);
}
