#include <string.h>
#include "Owl.h"
#include "armcontrol.h"
#include "usbcontrol.h"
#include "owlcontrol.h"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "ServiceCall.h"
#include "bkp_sram.h"

// #include "serial.h"
#include "clock.h"
#include "device.h"
#include "codec.h"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; \
if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry registry;
volatile bool bypass = false;

bool getButton(PatchButtonId bid){
  return getProgramVector()->buttons & (1<<bid);
}

void setGate(){
#ifdef OWLMODULAR
  clearPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output high
#endif
}

void clearGate(){
#ifdef OWLMODULAR
  setPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output low
#endif
}

void setButton(PatchButtonId bid){
  ProgramVector* pv = getProgramVector();
  pv->buttons |= (1<<bid);
  pv->parameters[PARAMETER_F+bid] = getSampleCounter();
// #ifdef OWLMODULAR
//   if(bid == PUSHBUTTON)
//     clearPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output high
// #endif
}

void clearButton(PatchButtonId bid){
  getProgramVector()->buttons &= ~(1<<bid);
// #ifdef OWLMODULAR
//   if(bid == PUSHBUTTON)
//     setPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output low
// #endif
}

void setButton(PatchButtonId bid, bool on){
  if(on)
    setButton(bid);
  else
    clearButton(bid);
}

void buttonChanged(PatchButtonId bid, bool on){
  switch(bid){
  case PUSHBUTTON:
    togglePushButton();
    break;
  case GREEN_BUTTON:
    setButton(GREEN_BUTTON, on);
    setButton(RED_BUTTON, !on);
    setLed(on ? GREEN : RED);
    break;
  case RED_BUTTON:
    setButton(RED_BUTTON, on);
    setButton(GREEN_BUTTON, !on);
    setLed(on ? RED : GREEN);
    break;
  }
}

void updateBypassMode(){
#ifdef OWLMODULAR
  bypass = false;
#else
  if(isStompSwitchPressed()){
    setButton(BYPASS_BUTTON);
    bypass = true;
    setLed(NONE);    
  }else{
    clearButton(BYPASS_BUTTON);
    bypass = false;
    if(getButton(RED_BUTTON))
      setLed(RED);
    else
      setLed(GREEN);
  }
#endif
}

void togglePushButton(){
  if(getLed() == GREEN){
    setGate();
    setLed(RED);
    setButton(RED_BUTTON);
    clearButton(GREEN_BUTTON);
  }else{ // if(getLed() == RED){
    clearGate();
    setLed(GREEN);
    setButton(GREEN_BUTTON);
    clearButton(RED_BUTTON);
  }
  midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
}

#ifdef OWLMODULAR
void pushGateCallback(){
  if(isPushGatePressed()){
    setButton(PUSHBUTTON);
    setLed(RED);
    setButton(RED_BUTTON);
    clearButton(GREEN_BUTTON);
  }else{
    clearButton(PUSHBUTTON);
    setLed(GREEN);
    setButton(GREEN_BUTTON);
    clearButton(RED_BUTTON);
  }
}
#else
void footSwitchCallback(){
  DEBOUNCE(bypass, BYPASS_DEBOUNCE);
  updateBypassMode();
}
#endif

volatile uint32_t pushButtonPressed;
void pushButtonCallback(){
  // DEBOUNCE(pushbutton, PUSHBUTTON_DEBOUNCE);
  if(isPushButtonPressed()){
    pushButtonPressed = getSysTicks();
    setButton(PUSHBUTTON);
    setGate();
    // setPushButton(RED);
    setLed(RED);
    setButton(RED_BUTTON);
    clearButton(GREEN_BUTTON);
  }else{
    pushButtonPressed = 0;
    clearButton(PUSHBUTTON);
    clearGate();
    // setPushButton(GREEN);
    setLed(GREEN);
    setButton(GREEN_BUTTON);
    clearButton(RED_BUTTON);
  }
  DEBOUNCE(pushbutton, PUSHBUTTON_DEBOUNCE);
  midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
}

void exitProgram(bool isr){
  // disable audio processing
  codec.softMute(true);
  codec.clear();
  program.exitProgram(isr);
  registry.setDynamicPatchDefinition(NULL);
  setLed(RED);
}

void updateProgramIndex(uint8_t index){
  if(settings.program_index != index){
    settings.program_index = index;
    midi.sendPc(index);
    midi.sendPatchName(index);
  }
}

#ifdef __cplusplus
 extern "C" {
#endif

   void setErrorMessage(int8_t err, const char* msg){
     setErrorStatus(err);
     setLed(RED);
     ProgramVector* vec = getProgramVector();
     if(vec != NULL)
       vec->message = (char*)msg;
   }

   PatchDefinition dynamicPatchDefinition;
   void registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
     dynamicPatchDefinition.name = name;
     dynamicPatchDefinition.inputs = inputChannels;
     dynamicPatchDefinition.outputs = outputChannels;
     registry.setDynamicPatchDefinition(&dynamicPatchDefinition);
     midi.sendPatchName(0);
   }

   void registerPatchParameter(uint8_t id, const char* name){
     midi.sendPatchParameterName((PatchParameterId)id, name);
   }

__attribute__ ((section (".coderam")))
   void programReady(){
     // while(audioStatus != AUDIO_READY_STATUS);
     program.programReady();
   }

   void programStatus(ProgramVectorAudioStatus status){
     program.programStatus(status);
   }

   void setParameter(int pid, uint16_t value){
     getAnalogValues()[pid] = value;
   }

#ifdef __cplusplus
}
#endif

void setParameterValues(uint16_t* values, int size){
  getProgramVector()->parameters = values;
  getProgramVector()->parameters_size = size;
}

void updateProgramVector(ProgramVector* vector){
  vector->checksum = sizeof(ProgramVector);
#ifdef OWLMODULAR
  vector->hardware_version = OWL_MODULAR_HARDWARE;
#else
  vector->hardware_version = OWL_PEDAL_HARDWARE;
#endif
  vector->audio_input = NULL;
  vector->audio_output = NULL;
  vector->audio_bitdepth = settings.audio_bitdepth;
  vector->audio_blocksize = settings.audio_blocksize;
  vector->audio_samplingrate = settings.audio_samplingrate;
  vector->parameters = getAnalogValues();
  vector->parameters_size = NOF_PARAMETERS;
  // todo: pass real-time updates from MidiHandler
  vector->buttons = (1<<GREEN_BUTTON);
  vector->registerPatch = registerPatch;
  vector->registerPatchParameter = registerPatchParameter;
  vector->cycles_per_block = 0;
  vector->heap_bytes_used = 0;
  vector->programReady = programReady;
  vector->programStatus = programStatus;
  vector->serviceCall = serviceCall;
  vector->message = NULL;
}

void setup(){
//   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // 0 bits for preemption, 4 bits for subpriority
  /* Set up interrupt controller: 2 bits for priority (0-3),
   * 2 bits for sub-priority (0-3). Priorities control which
   * interrupts are allowed to preempt one another.
   */
  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /* Increase SysTick priority to be higher than USB interrupt
   * priority. USB code stalls inside interrupt and we can't let
   * this throw off the SysTick timer.
   */
  // NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, SYSTICK_PRIORITY, SYSTICK_SUBPRIORITY));
  NVIC_SetPriority(AUDIO_I2S_DMA_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));
  NVIC_SetPriority(AUDIO_I2S_EXT_DMA_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));
  NVIC_SetPriority(CODEC_I2S_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_4, 1, 0));
  NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 3, 0));
  NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 5, 0));

  updateProgramVector(getProgramVector());

  ledSetup();
  setLed(RED);
  BKPSRAM_Init();

  /* check if we need to DFU boot */
  configureDigitalInput(SWITCH_B_PORT, SWITCH_B_PIN, GPIO_PuPd_UP);
  if(isPushButtonPressed())
    jump_to_bootloader();

  adcSetup();
  clockSetup();
#ifdef OWLMODULAR
  setupSwitchA(pushGateCallback);
#else
  setupSwitchA(footSwitchCallback);
#endif
  setupSwitchB(pushButtonCallback);

  settings.init();
  midi.init(MIDI_CHANNEL);
  registry.init();

#ifdef EXPRESSION_PEDAL
#ifndef OWLMODULAR
  setupExpressionPedal();
#endif
#endif

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // DEBUG
  configureDigitalOutput(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
  debugClear();

#ifdef OWLMODULAR
  // Configure OWL Modular digital output  
  RCC_AHB1PeriphClockCmd(PUSH_GATE_OUT_CLK, ENABLE);
  configureDigitalOutput(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN);
#endif
  clearButton(PUSHBUTTON);

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
  codec.softMute(true);

  program.loadProgram(settings.program_index);
  program.startProgram(false);

  updateBypassMode();

  codec.start();
}

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef BUTTON_PROGRAM_CHANGE
#define PROGRAM_CHANGE_PUSHBUTTON_MS 2000
#include "clock.h"
#endif /* BUTTON_PROGRAM_CHANGE */

extern volatile ProgramVectorAudioStatus audioStatus;
__attribute__ ((section (".coderam")))
void audioCallback(int16_t *src, int16_t *dst){
#ifdef DEBUG_AUDIO
  togglePin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
#endif
  getProgramVector()->audio_input = src;
  getProgramVector()->audio_output = dst;
  // program.audioReady();
  audioStatus = AUDIO_READY_STATUS;

#ifdef BUTTON_PROGRAM_CHANGE
  if(pushButtonPressed && (getSysTicks() > pushButtonPressed+PROGRAM_CHANGE_PUSHBUTTON_MS)
     && settings.program_change_button){
    if(isPushButtonPressed()){
      setLed(NONE);
      program.startProgramChange(true);
    }
    pushButtonPressed = 0; // prevent re-trigger
  }
#endif /* BUTTON_PROGRAM_CHANGE */
}

#ifdef __cplusplus
}
#endif
