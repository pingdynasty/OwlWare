#ifndef __ARM_CONTROL_H
#define __ARM_CONTROL_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif

   void digitalOutputSetup();

   void adcSetupDMA(uint16_t* dma);
   void adcSetup(void);
   /* void dacSetup(void); */
   /* void setAnalogValue(uint8_t channel, uint16_t value); */
   uint16_t getAnalogValue(uint8_t index);

   /* void timerSetup(void (*f)(), uint16_t period); */
   /* void setTimerPeriod(uint32_t period); */
   /* void setTimerPrescaler(uint16_t prescaler); */
   /* void setPeriod(TIM_TypeDef* tim, uint32_t period); */
   /* void setPrescaler(TIM_TypeDef* tim, uint32_t period); */

#ifdef __cplusplus
}
#endif

#endif /* __ARM_CONTROL_H */
