#ifndef __OWL_CONTROL_H
#define __OWL_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "device.h"
#include "gpio.h"
#include "message.h"

#ifdef __cplusplus
 extern "C" {
#endif

   bool isClockExternal();
   uint32_t* getDeviceId();
   void jump_to_bootloader();
   char* getFirmwareVersion();

   bool debugGet();

   inline void debugSet(){
     setPin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   inline void debugClear(){
     clearPin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   inline void debugToggle(){
     togglePin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
   }

   typedef enum {
     NONE  = 0,
     GREEN = LED_GREEN,
     RED   = LED_RED
   }  LedPin;

   LedPin getLed();

   inline void setLed(LedPin led){
#ifdef OWLRACK
     if(led == LED_GREEN)
       debugSet();
     else
       debugClear();
#else
     clearPin(LED_PORT, led ^ (LED_RED|LED_GREEN));
     setPin(LED_PORT, led);
#endif
   }

   inline void toggleLed(){
     togglePin(LED_PORT, LED_RED|LED_GREEN);
   }

   void ledSetup();

   void adcSetup();
   int16_t getAnalogValue(uint8_t index);
   int16_t* getAnalogValues();

   inline bool isStompSwitchPressed(){
#if defined OWLMODULAR || defined OWLRACK
     return false;
#else
     return getPin(SWITCH_A_PORT, SWITCH_A_PIN);
#endif
   }

   inline bool isPushButtonPressed(){
#ifdef OWLRACK
     return false;
#else
     return !getPin(SWITCH_B_PORT, SWITCH_B_PIN);
#endif
   }

   inline bool isPushGatePressed(){
#ifdef OWLMODULAR
     return !getPin(SWITCH_A_PORT, SWITCH_A_PIN);
#else
     return false;
#endif
   }

   void setupSwitchA(void (*f)());
   void setupSwitchB(void (*f)());

   void setupExpressionPedal();
   bool hasExpressionPedal();

   void exitProgram(bool isr);
   void updateProgramIndex(uint8_t index);
   void togglePushButton();

   void setRemoteControl(bool remote);

#ifdef __cplusplus
}
#endif

#endif /* __OWL_CONTROL_H */
