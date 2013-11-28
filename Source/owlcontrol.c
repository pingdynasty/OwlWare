#include "owlcontrol.h"
#include "usbcontrol.h"
#include "armcontrol.h"
#include <string.h>
#include "stm32f4xx.h"
#include "device.h"
#include "gpio.h"

char* getFirmwareVersion(){ 
  return "Prototype Rev03"; 
}

bool isClockExternal(){
/* return RCC_HSE_ON */
  return RCC_WaitForHSEStartUp() == SUCCESS;
}

uint32_t* getDeviceId(){
  const uint32_t* addr = (uint32_t*)0x1fff7a10;
  static uint32_t deviceId[3];
  deviceId[0] = addr[0];
  deviceId[1] = addr[1];
  deviceId[2] = addr[2];
  // read location 0xE0042000
  // 16 bits revision id, 4 bits reserved, 12 bits device id
  /* 0x1000 = Revision A, 0x1001 = Revision Z */

/* The ARM CortexTM-M4F integrates a JEDEC-106 ID code. It is located in the 4KB ROM table mapped on the internal PPB bus at address 0xE00FF000_0xE00FFFFF. */

/*   return DBGMCU_GetREVID() << 16 | DBGMCU_GetDEVID(); */

/* Unique device ID register (96 bits) */
  return deviceId;
}

/* Magic number to store in memory before reset to tell the device to
 * enter system bootloader.
 */
const uint32_t kBootloaderMagicNumber = 0x5EA1AB50;

/* Jump to the internal STM32 bootloader. The way this works is that we
 * set a magic number in memory that our startup code looks for (see startup_stm32f4xx.s).
 * RAM is preserved across system reset, so when it finds this magic number, it will go
 * to the bootloader code rather than the application code.
 */
void jump_to_bootloader(void){
  uint16_t i;
  volatile uint32_t delayCounter;

  /* Disable USB in advance: this will give the computer time to
   * recognise it's been disconnected, so when the system bootloader
   * comes online it will get re-enumerated.
   */
  usb_deinit();

  /* Blink LEDs */
  for(i = 0; i < 3; i++) {
    for(delayCounter = 0; delayCounter < 2000000; delayCounter++)
      toggleLed();
    setLed(RED);
    for(delayCounter = 0; delayCounter < 2000000; delayCounter++);
  }

  /* This address within first 64k of memory: seemed to be problems with higher
   * locations. Note that this must match what's in startup_stm32f4xx.S
   */
  *((unsigned long *)0x2000FFF0) = kBootloaderMagicNumber;

  NVIC_SystemReset();

  /* Shouldn't get here */
  while(1);
}

LedPin getLed(){
  if(getPin(LED_PORT, LED_GREEN))
    return GREEN;
  else if(getPin(LED_PORT, LED_RED))
    return RED;
  else
    return NONE;
}

void ledSetup(){
  RCC_AHB1PeriphClockCmd(LED_CLOCK, ENABLE);
  configureDigitalOutput(LED_PORT, LED_RED|LED_GREEN);
  clearPin(LED_PORT, LED_RED|LED_GREEN);
}

uint16_t adc_values[8];
void adcSetup(){
  memset(adc_values, 0, sizeof adc_values);
  adcSetupDMA(&adc_values[0]);
}

uint16_t getAnalogValue(uint8_t index){
  assert_param(index < sizeof(adc_values));
  return adc_values[index];
}


void (*externalInterruptCallbackA)();
void (*externalInterruptCallbackB)();

/** 
 * Configure EXTI callback
 */
void setupSwitchA(void (*f)()){
  externalInterruptCallbackA = f;

  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the clocks */
  RCC_AHB1PeriphClockCmd(SWITCH_A_CLOCK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure switch pin */
  configureDigitalInput(SWITCH_A_PORT, SWITCH_A_PIN, GPIO_PuPd_UP);

  /* Connect EXTI Line to GPIO Pin */
  SYSCFG_EXTILineConfig(SWITCH_A_PORT_SOURCE, SWITCH_A_PIN_SOURCE);

  /* Configure EXTI line */
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = SWITCH_A_PIN_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = SWITCH_A_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SWITCH_A_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SWITCH_A_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void SWITCH_A_HANDLER(void) {
  if(EXTI_GetITStatus(SWITCH_A_PIN_LINE) != RESET){ 
    (*externalInterruptCallbackA)();
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(SWITCH_A_PIN_LINE);
  }
}

/** 
 * Configure EXTI callback
 */
void setupSwitchB(void (*f)()){
  externalInterruptCallbackB = f;

  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the clocks */
  RCC_AHB1PeriphClockCmd(SWITCH_B_CLOCK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure switch pin */
  configureDigitalInput(SWITCH_B_PORT, SWITCH_B_PIN, GPIO_PuPd_UP);

  /* Connect EXTI Line to GPIO Pin */
  SYSCFG_EXTILineConfig(SWITCH_B_PORT_SOURCE, SWITCH_B_PIN_SOURCE);

  /* Configure EXTI line */
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = SWITCH_B_PIN_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
/*   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; */
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = SWITCH_B_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SWITCH_B_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SWITCH_B_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void SWITCH_B_HANDLER(void) {
  if(EXTI_GetITStatus(SWITCH_B_PIN_LINE) != RESET){ 
    (*externalInterruptCallbackB)();
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(SWITCH_B_PIN_LINE);
  }
}

void setupExpressionPedal(){
  configureDigitalOutput(EXPRESSION_PEDAL_TIP_PORT, EXPRESSION_PEDAL_TIP_PIN); // PA3, expression pedal tip
  setPin(EXPRESSION_PEDAL_TIP_PORT, EXPRESSION_PEDAL_TIP_PIN);
  // expression pedal ring, PA2, is set up in adcSetupDMA()
}

bool hasExpressionPedal(){
  /* clearPin(EXPRESSION_PEDAL_TIP_PORT, EXPRESSION_PEDAL_TIP_PIN); */
  configureDigitalInput(EXPRESSION_PEDAL_TIP_PORT, EXPRESSION_PEDAL_TIP_PIN, GPIO_PuPd_UP);
  bool connected = getPin(EXPRESSION_PEDAL_TIP_PORT, EXPRESSION_PEDAL_TIP_PIN);
  setupExpressionPedal();
  return connected;
}
