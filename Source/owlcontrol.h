#ifndef __OWL_CONTROL_H
#define __OWL_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "device.h"
#include "gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif

   bool isClockExternal();
   uint8_t* getDeviceId();
   void jump_to_bootloader();
   char* getFirmwareVersion();

   typedef enum {
     NONE  = 0,
     GREEN = LED_GREEN,
     RED   = LED_RED
   }  LedPin;

   LedPin getLed();

   inline void setLed(LedPin led){
     clearPin(LED_PORT, led ^ (LED_RED|LED_GREEN));
     setPin(LED_PORT, led);
#ifdef OWLMODULAR
     if(led == LED_RED)
       clearPin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output high
     else
       setPin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output low
#endif
   }

   inline void toggleLed(){
     togglePin(LED_PORT, LED_RED|LED_GREEN);
#ifdef OWLMODULAR
     togglePin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output
#endif
   }

   inline void blink(){
     togglePin(LED_PORT, LED_RED|LED_GREEN);
#ifdef OWLMODULAR
     togglePin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output
#endif
     togglePin(LED_PORT, LED_RED|LED_GREEN);
#ifdef OWLMODULAR
     togglePin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output
#endif
   }

   inline void debugSet(){
     setPin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   inline void debugClear(){
     clearPin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   inline void debugToggle(){
     togglePin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   void ledSetup();

   void adcSetup();
   uint16_t getAnalogValue(uint8_t index);
   uint16_t* getAnalogValues();

   void setPatch(uint8_t index);

   inline bool isStompSwitchPressed(){
     return getPin(SWITCH_A_PORT, SWITCH_A_PIN);
   }

   inline bool isPushButtonPressed(){
     return !getPin(SWITCH_B_PORT, SWITCH_B_PIN);
   }

   void setupSwitchA(void (*f)());
   void setupSwitchB(void (*f)());

   void setupExpressionPedal();

   bool hasExpressionPedal();

#ifdef __cplusplus
}
#endif

#endif /* __OWL_CONTROL_H */
