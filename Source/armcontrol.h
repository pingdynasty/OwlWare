#ifndef __ARM_CONTROL_H
#define __ARM_CONTROL_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif

   void adcSetupDMA(int16_t* dma);
   void adcStartDMA();
   void adcStopDMA();
   uint16_t getSampleCounter();

#ifdef __cplusplus
}
#endif

#endif /* __ARM_CONTROL_H */
