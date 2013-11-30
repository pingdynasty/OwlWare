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
#include "SampleBuffer.hpp"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"

#include "serial.h"
#include "clock.h"
#include "device.h"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry patches;
Patch* activePatch = NULL;
Patch* nextPatch = NULL;
bool bypass = false;
int activePatchIndex = 0;

void updateLed(){
  setLed(activePatchIndex == settings.patch_red ? RED : GREEN);
}

void updateBypassMode(){
  if(isStompSwitchPressed()){
    setLed(NONE);
    bypass = true;
  }else{
    updateLed();
    bypass = false;
  }
}

void footSwitchCallback(){
  DEBOUNCE(bypass, 200);
  updateBypassMode();
}

void pushButtonCallback(){
  DEBOUNCE(pushbutton, 200);
  if(!getPushButton()){
    if(settings.patch_green != activePatchIndex){
      setActivePatch(settings.patch_green);
    }else if(settings.patch_red != activePatchIndex){
      setActivePatch(settings.patch_red);
    }
  }
  updateLed();
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

bool doProcessAudio = false;
uint16_t* source;
uint16_t* dest;

PatchProcessor* currentPatchProcessor = NULL;
PatchProcessor* getPatchProcessor(){
  return currentPatchProcessor;
}

Patch* createPatch(uint8_t index){
  Patch* patch = patches.create(index);
  if(currentPatchProcessor != NULL)
    delete currentPatchProcessor;
  currentPatchProcessor = new PatchProcessor(patch);
  return patch;
}

void setActivePatch(uint8_t index){
  if(index < patches.getNumberOfPatches() && index != activePatchIndex){
    activePatchIndex = index;
    nextPatch = createPatch(index);
    codec.softMute(true);
  }
}

void setActiveSlot(uint8_t index){
  if(index == GREEN){
    setActivePatch(settings.patch_green);
  }else if(index == RED){
    setActivePatch(settings.patch_red);
  }
}

uint8_t getActivePatch(){
  return activePatchIndex;
}

uint8_t getActiveSlot(){
  return activePatchIndex == settings.patch_red ? RED : GREEN;
}

__attribute__ ((section (".coderam")))
void run(){
  for(;;){
    if(doProcessAudio){
      //     setPin(GPIOA, GPIO_Pin_7); // PA7 DEBUG

      buffer.split(source);
      activePatch->processAudio(buffer);
      buffer.comb(dest);

      if(nextPatch != NULL){
	delete activePatch;
	activePatch = nextPatch;
	nextPatch = NULL;
	codec.softMute(false);
      }

      doProcessAudio = false;
      //     clearPin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
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

  settings.init();
  ledSetup();
  setLed(RED);

  /* check if we need to DFU boot */
  configureDigitalInput(SWITCH_B_PORT, SWITCH_B_PIN, GPIO_PuPd_UP);
  if(isPushButtonPressed())
    jump_to_bootloader();

  activePatch = createPatch(settings.patch_green);

  codec.setup();
  codec.init(settings);

  midi.init(MIDI_CHANNEL);

  adcSetup();
  clockSetup();
  setupSwitchA(footSwitchCallback);
  setupSwitchB(pushButtonCallback);

#ifdef EXPRESSION_PEDAL
  setupExpressionPedal();
#endif

  configureDigitalOutput(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED

//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // DEBUG
//   configureDigitalOutput(GPIOA, GPIO_Pin_6); // PA6, DEBUG
//   configureDigitalOutput(GPIOA, GPIO_Pin_7); // PA7, DEBUG

//   setPin(GPIOA, GPIO_Pin_6); // DEBUG
//   setPin(GPIOA, GPIO_Pin_7); // DEBUG
	
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

  printString("startup\n");
  updateBypassMode();

  codec.start();
}

void debugToggle(){
  togglePin(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
}

#ifdef __cplusplus
 extern "C" {
#endif

#if 0 // todo: remove
int frequency;
long lastcallback;
long collisions;
void audioCallback(int16_t *src, int16_t *dst, int16_t sz){
  frequency = (AUDIO_BLOCK_SIZE * 1000)/(getSysTicks() - lastcallback);
  lastcallback = getSysTicks();
//   setPin(GPIOA, GPIO_Pin_6); // PA6 DEBUG
  buffer.split(src);
  dest = dst;
  if(doProcessAudio)
    collisions++;
  doProcessAudio = true;
//   clearPin(GPIOA, GPIO_Pin_6); // PA6 DEBUG
}
#elif 0
void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz){
  for(int i=0; i<sz; ++i)
    dst[i] = src[i];
}
#else
__attribute__ ((section (".coderam")))
void audioCallback(uint16_t *src, uint16_t *dst, uint16_t sz){
  if(doProcessAudio)
    blink();
  source = src;
  dest = dst;
  doProcessAudio = true;
}
#endif

#ifdef __cplusplus
}
#endif
