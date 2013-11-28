#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

/* Every GPIO port has an input and */
/* output data register, ODR and IDR */
/* respectively, which hold the status of the pin */
   inline bool getPin(GPIO_TypeDef* port, uint32_t pin){
     return port->IDR & pin;
/*      return GPIO_ReadInputDataBit(port, pin); */
   }
/* A logical 1 in BSRRL will set the pin and a logical 1 in BSRRH will */
/* reset the pin. A logical 0 in either register has no effect */
   inline void setPin(GPIO_TypeDef* port, uint32_t pin){
/*        port->BSRR = pin; */
     port->BSRRL = pin;
/*      return GPIO_SetBits(port, pin); */
   }
   inline void clearPin(GPIO_TypeDef* port, uint32_t pin){
/*        port->BRR = pin; */
     port->BSRRH = pin;
/*      GPIO_ResetBits(port, pin); */
   }
   inline void togglePin(GPIO_TypeDef* port, uint32_t pin){
     port->ODR ^= pin;
/*      GPIO_ToggleBits(port, pin); */
   }

   void configureDigitalInput(GPIO_TypeDef* port, uint32_t pin, GPIOPuPd_TypeDef pull);
   void configureDigitalOutput(GPIO_TypeDef* port, uint32_t pin);

   void configureAnalogInput(GPIO_TypeDef* port, uint32_t pin);
   void configureAnalogOutput(GPIO_TypeDef* port, uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
