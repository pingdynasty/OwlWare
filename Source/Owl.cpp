#include "Owl.h"
#include "arm_math.h"
#include "armcontrol.h"
#include "usbcontrol.h"
#include "owlcontrol.h"
#include "PatchRegistry.h"
#include "StompBox.h"
#include "PatchProcessor.h"
#include "MidiController.h"
#include "CodecController.h"
#include "PatchController.h"
#include "SampleBuffer.hpp"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"

#include "serial.h"
#include "clock.h"
#include "device.h"

#define DEBUG_AUDIO

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry registry;
PatchController patches;
volatile bool bypass = false;

void updateLed(){
  setLed((LedPin)patches.getActiveSlot());
  midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
}

void updateBypassMode(){
  if(isStompSwitchPressed()){
    bypass = true;
    setLed(NONE);
    midi.sendCc(LED, 0);
  }else{
    bypass = false;
    updateLed();
  }
}

void footSwitchCallback(){
  DEBOUNCE(bypass, 200);
  updateBypassMode();
}

void toggleActiveSlot(){
  patches.toggleActiveSlot();
  updateLed();
  midi.sendPatchParameterNames();
}

void pushButtonCallback(){
  DEBOUNCE(pushbutton, 200);
  if(isPushButtonPressed())
    toggleActiveSlot();
}

#if AUDIO_BITDEPTH == 32
SampleBuffer32 buffer CCM;
#elif AUDIO_BITDEPTH == 24
SampleBuffer32 buffer CCM;
#elif AUDIO_BITDEPTH == 16
SampleBuffer16 buffer CCM;
#else
#error invalid AUDIO_BITDEPTH
#endif

volatile bool doProcessAudio = false;
volatile bool collision = false;
uint16_t* source;
uint16_t* dest;

extern uint32_t dwt_count = 0;
__attribute__ ((section (".coderam")))
void run(){
#ifdef DEBUG_AUDIO
      volatile unsigned int *DWT_CYCCNT = (volatile unsigned int *)0xE0001004; //address of the register
      volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
      volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register
      *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
      *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter
#endif
  for(;;){
    if(doProcessAudio){
#ifdef DEBUG_AUDIO
      setPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
      *DWT_CYCCNT = 0; // reset the counter
#endif
      buffer.split(source);
      patches.process(buffer);
      buffer.comb(dest);
      if(collision){
	collision = false;
#ifdef DEBUG_AUDIO
	debugToggle();
#endif
      }else{
	doProcessAudio = false;
      }
#ifdef DEBUG_AUDIO
      clearPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
      dwt_count = *DWT_CYCCNT;
#endif
    }
  }
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
  patches.init();

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

__attribute__ ((section (".coderam")))
void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz){
#ifdef DEBUG_AUDIO
  togglePin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
#endif
  if(doProcessAudio)
    collision = true;
  source = src;
  dest = dst;
  doProcessAudio = true;
}

#ifdef __cplusplus
}
#endif
