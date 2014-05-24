#include "Owl.h"
#include "arm_math.h"
#include "armcontrol.h"
#include "usbcontrol.h"
#include "owlcontrol.h"
#include "StompBox.h"
#include "MidiController.h"
#include "CodecController.h"
// #include "SampleBuffer.hpp"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"

#include "SharedMemory.h"

#include "serial.h"
#include "clock.h"
#include "device.h"

#define DEBUG_AUDIO

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
ApplicationSettings settings;

void updateLed(){
  setLed(getLed() == GREEN ? RED : GREEN);
  midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
}

void updateBypassMode(){
  if(isStompSwitchPressed()){
    smem.audio_status |= AUDIO_STATUS_BYPASS;
    setLed(NONE);
    midi.sendCc(LED, 0);
  }else{
    smem.audio_status &= ~AUDIO_STATUS_BYPASS;
    updateLed();
  }
}

void footSwitchCallback(){
  DEBOUNCE(bypass, 200);
  updateBypassMode();
}

void toggleActiveSlot(){
  updateLed();
  // midi.sendPatchParameterNames();
}

void pushButtonCallback(){
  DEBOUNCE(pushbutton, 200);
  if(isPushButtonPressed()){
    smem.audio_status |= AUDIO_STATUS_BUTTON;
    toggleActiveSlot();
  }else{
    smem.audio_status &= ~AUDIO_STATUS_BUTTON;
  }
}

void run(){
  for(;;);
  // todo: load and run patch
}

void setup(){
//   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // 0 bits for preemption, 4 bits for subpriority
  /* Set up interrupt controller: 2 bits for priority (0-3),
   * 2 bits for sub-priority (0-3). Priorities control which
   * interrupts are allowed to preempt one another.
   */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /* Increase SysTick priority to be higher than USB interrupt
   * priority. USB code stalls inside interrupt and we can't let
   * this throw off the SysTick timer.
   */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, SYSTICK_PRIORITY, SYSTICK_SUBPRIORITY));
  NVIC_SetPriority(DMA1_Stream3_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 0, 0));
  NVIC_SetPriority(DMA1_Stream4_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 0, 0));
  NVIC_SetPriority(SPI2_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 1, 0));
  NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 2, 0));

  ledSetup();
  setLed(RED);

  /* check if we need to DFU boot */
  configureDigitalInput(SWITCH_B_PORT, SWITCH_B_PIN, GPIO_PuPd_UP);
  if(isPushButtonPressed())
    jump_to_bootloader();

  adcSetup();
  clockSetup();
  setupSwitchA(footSwitchCallback);
  setupSwitchB(pushButtonCallback);

  settings.init();
  midi.init(MIDI_CHANNEL);

#ifdef EXPRESSION_PEDAL
  setupExpressionPedal();
#endif

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // DEBUG
  configureDigitalOutput(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
  debugClear();

#ifdef DEBUG_AUDIO
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // DEBUG
  configureDigitalOutput(GPIOA, GPIO_Pin_7); // PA7 DEBUG
  configureDigitalOutput(GPIOC, GPIO_Pin_5); // PC5 DEBUG
  clearPin(GPIOC, GPIO_Pin_5); // DEBUG
  clearPin(GPIOA, GPIO_Pin_7); // DEBUG
#endif /* DEBUG_AUDIO */
	
  usb_init();

#if SERIAL_PORT == 1
  setupSerialPort1(115200);
#elif SERIAL_PORT == 2
  setupSerialPort2(115200); // expression pedal
#warning expression pedal jack configured as serial port
#ifdef EXPRESSION_PEDAL
#error invalid configuration
#endif
#endif

  codec.setup();
  codec.init(settings);

  printString("startup\n");
  updateBypassMode();

  codec.start();
}

#ifdef __cplusplus
 extern "C" {
#endif

void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz){
#ifdef DEBUG_AUDIO
  togglePin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
  if(smem.audio_status && AUDIO_STATUS_PROCESS)
    debugToggle();
#endif
    
  smem.audio_input = src;
  smem.audio_output = dst;
  smem.audio_status |= AUDIO_STATUS_PROCESS;
}

#ifdef __cplusplus
}
#endif
