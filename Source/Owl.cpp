#include <string.h>
#include "Owl.h"
#include "armcontrol.h"
#include "usbcontrol.h"
#include "owlcontrol.h"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "MidiReader.h"
#include "midicontrol.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include "ServiceCall.h"
#include "MidiStatus.h"
#include "bkp_sram.h"
#include "serial.h"
#include "bus.h"
#include "clock.h"
#include "device.h"
#include "codec.h"
#include "BitState.hpp"
#include "DigitalBusReader.h"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; \
if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
MidiReader midireader;
ApplicationSettings settings;
PatchRegistry registry;
DigitalBusReader bus;

// there are only really 2 timestamps needed: LED pushbutton and midi gate
uint16_t timestamps[NOF_BUTTONS]; 
BitState32 stateChanged;

bool getButton(uint8_t bid){
  return getProgramVector()->buttons & (1<<bid);
}

static void setButtonState(uint8_t bid){
  getProgramVector()->buttons |= (1<<bid);
}

static void clearButtonState(uint8_t bid){
  getProgramVector()->buttons &= ~(1<<bid);
}

static void setGate(){
#ifdef OWLMODULAR
  clearPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output high
#endif
}

static void clearGate(){
#ifdef OWLMODULAR
  setPin(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN); // OWL Modular digital output low
#endif
}

static void setButtonColour(LedPin led){
  switch(led){
  case GREEN:
    if(!getButton(BYPASS_BUTTON))
      setLed(GREEN);
    setButtonState(GREEN_BUTTON);
    clearButtonState(RED_BUTTON);
    break;
  case RED:
    setLed(RED);
    clearButtonState(GREEN_BUTTON);
    setButtonState(RED_BUTTON);
    break;
  default:
    setLed(NONE);
    clearButtonState(GREEN_BUTTON);
    clearButtonState(RED_BUTTON);
    break;
  }
}

// called from incoming button trigger irq
static void setButtonEvent(PatchButtonId bid){
  timestamps[bid] = getSampleCounter();
  stateChanged.set(bid);
  setButtonState(bid);
}

static void clearButtonEvent(PatchButtonId bid){
  timestamps[bid] = getSampleCounter();
  stateChanged.set(bid);
  clearButtonState(bid);
}

static void updateBypassMode(){
#if !defined OWLMODULAR && !defined OWLRACK
  if(isStompSwitchPressed()){
    setButtonState(BYPASS_BUTTON);
    setButtonColour(NONE);
  }else{
    clearButtonState(BYPASS_BUTTON);
    if(getButton(RED_BUTTON))
      setButtonColour(RED);
    else
      setButtonColour(GREEN);
  }
#endif
}

// called from midi irq
void togglePushButton(){
  if(getButton(PUSHBUTTON)){
    clearButtonEvent(PUSHBUTTON);
    setButtonColour(GREEN);
  }else{
    setButtonEvent(PUSHBUTTON);
    setButtonColour(RED);
  }
}

#ifdef OWLMODULAR
static void pushGateCallback(){
  if(isPushGatePressed()){
    if(!getButton(PUSHBUTTON)){
      setButtonEvent(PUSHBUTTON);
      setButtonColour(RED);
    }
  }else{
    if(getButton(PUSHBUTTON)){
      clearButtonEvent(PUSHBUTTON);
      setButtonColour(GREEN);
    }
  }
}
#else
static void footSwitchCallback(){
  DEBOUNCE(bypass, BYPASS_DEBOUNCE);
  updateBypassMode();
}
#endif

volatile uint32_t pushButtonPressed;
static void pushButtonCallback(){
  // DEBOUNCE(pushbutton, PUSHBUTTON_DEBOUNCE);
  if(isPushButtonPressed()){
    if(!getButton(PUSHBUTTON)){
      pushButtonPressed = getSysTicks();
      setButtonEvent(PUSHBUTTON);
      setGate();
      setButtonColour(RED);
      midi.sendCc(LED, 84);
    }
  }else{
    if(getButton(PUSHBUTTON)){
      pushButtonPressed = 0;
      clearButtonEvent(PUSHBUTTON);
      clearGate();
      setButtonColour(GREEN);
      midi.sendCc(LED, 42);
    }
  }
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
  // if(settings.program_index != index){
    settings.program_index = index;
    midi.sendPc(index);
    midi.sendPatchName(index);
  // }
}

#ifdef __cplusplus
 extern "C" {
#endif

   // called from program
   void onRegisterPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
     static PatchDefinition dynamicPatchDefinition;
     dynamicPatchDefinition.name = name;
     dynamicPatchDefinition.inputs = inputChannels;
     dynamicPatchDefinition.outputs = outputChannels;
     registry.setDynamicPatchDefinition(&dynamicPatchDefinition);
     midi.sendPatchName(0);
   }

   // called from program
   void onRegisterPatchParameter(uint8_t id, const char* name){
     midi.sendPatchParameterName((PatchParameterId)id, name);
   }

#define DWT_CYCCNT ((volatile unsigned int *)0xE0001004)
extern volatile ProgramVectorAudioStatus audioStatus;

   __attribute__ ((section (".coderam")))
   // called from program
   void onProgramReady(){
     // program.programReady();
     ProgramVector* vec = getProgramVector();
#ifdef DEBUG_DWT
     vec->cycles_per_block = *DWT_CYCCNT;
#endif /* DEBUG_DWT */
#ifdef DEBUG_AUDIO
     clearPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
#endif
     while(audioStatus != AUDIO_READY_STATUS);
     audioStatus = AUDIO_PROCESSING_STATUS;
#ifdef DEBUG_DWT
     *DWT_CYCCNT = 0; // reset the performance counter
#endif /* DEBUG_DWT */
#ifdef DEBUG_AUDIO
     setPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
#endif
     if(vec->buttonChangedCallback != NULL && stateChanged.getState()){
       int bid = stateChanged.getFirstSetIndex();
       do{
	 vec->buttonChangedCallback(bid, getButton((PatchButtonId)bid)?4095:0, timestamps[bid]);
	 timestamps[bid] = 0;
 	 stateChanged.clear(bid);
	 bid = stateChanged.getFirstSetIndex();
       }while(bid > 0); // bid 0 is bypass button which we ignore
     }
   }

   // called from program
   void onProgramStatus(ProgramVectorAudioStatus status){
     program.programStatus(status);
   }

   // called from program
   void onSetButton(uint8_t bid, uint16_t state, uint16_t samples){
     if(bid == PUSHBUTTON){
       if(state){
	 if(!getButton(PUSHBUTTON)){
	   setGate();
	   setButtonColour(RED);
	   setButtonState(bid);
	   midi.sendCc(LED, 84);
	 }
       }else{
	 if(getButton(PUSHBUTTON)){
	   clearGate();
	   setButtonColour(GREEN);
	   clearButtonState(bid);
	   midi.sendCc(LED, 42);
	 }
       }
     }else if(bid < NOF_BUTTONS){
       if(state){
	 setButtonState(bid);
	 if(bid == GREEN_BUTTON)
	   setButtonColour(GREEN);	 
	 else if(bid == RED_BUTTON)
	   setButtonColour(RED);
       }else{
	 clearButtonState(bid);
       }
     }else if(bid >= MIDI_NOTE_BUTTON){
       if(state)
	 midi.sendNoteOn(bid-MIDI_NOTE_BUTTON, (state>>5) & 0x7f);
       else
	 midi.sendNoteOff(bid-MIDI_NOTE_BUTTON, 0);
     }
#ifdef DIGITAL_BUS_OUTPUT
     bus_tx_button(bid, state);
#endif
   }

   // called from program
   void onSetPatchParameter(uint8_t pid, int16_t value){
     if(pid < NOF_PARAMETERS)
       getProgramVector()->parameters[pid] = value;
     switch(pid){
     case PARAMETER_A:
     case PARAMETER_B:
     case PARAMETER_C:
     case PARAMETER_D:
     case PARAMETER_E:
       midi.sendCc(PATCH_PARAMETER_A+pid, (value>>5) & 0x7f);
       break;
     case PARAMETER_F:
       midi.sendCc(PATCH_PARAMETER_F, (value>>5) & 0x7f);
       break;
     case PARAMETER_G:
       midi.sendCc(PATCH_PARAMETER_G, (value>>5) & 0x7f);
       // midi.sendPitchBend(value<<1);
       break;
     case PARAMETER_H:
       midi.sendCc(PATCH_PARAMETER_H, (value>>5) & 0x7f);
       break;
     default:
       if(pid >= PARAMETER_AA && pid <= PARAMETER_BH)
	 midi.sendCc(PATCH_PARAMETER_AA+(pid-PARAMETER_AA), (value>>5) & 0x7f);	 
     }
#ifdef DIGITAL_BUS_OUTPUT
     bus_tx_parameter(pid, value);
#endif
   }

   // called from midi irq or digital bus
   void setButton(uint8_t bid, uint16_t state){
     if(bid < NOF_BUTTONS){
       if(state)
	 setButtonEvent((PatchButtonId)bid);
       else
	 clearButtonEvent((PatchButtonId)bid);
     }else if(bid >= MIDI_NOTE_BUTTON){
       if(getProgramVector()->buttonChangedCallback != NULL)
	 getProgramVector()->buttonChangedCallback(bid, state, getSampleCounter());
     }
   }

   // called from midi irq or digital bus
   void setParameterValue(uint8_t pid, int16_t value){
     ASSERT(pid < getProgramVector()->parameters_size, "Parameter ID out of range");
     getProgramVector()->parameters[pid] = value;
   }

   int16_t getParameterValue(uint8_t pid){
     ASSERT(pid < getProgramVector()->parameters_size, "Parameter ID out of range");
     return getProgramVector()->parameters[pid];
   }

#ifdef __cplusplus
}
#endif

void setParameterValues(int16_t* values, int size){
  getProgramVector()->parameters = values;
  getProgramVector()->parameters_size = size;
}

int16_t* getParameterValues(){
  return getProgramVector()->parameters;
}

void setRemoteControl(bool remote){
  if(remote){
    adcStopDMA();
  }else{
    adcStartDMA();
  }
}

void updateProgramVector(ProgramVector* vector){
  vector->checksum = PROGRAM_VECTOR_CHECKSUM_V12;
#if defined OWLMODULAR
  vector->hardware_version = OWL_MODULAR_HARDWARE;
#elif defined OWLRACK
  vector->hardware_version = OWL_RACK_HARDWARE;
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
  vector->buttons = (1<<GREEN_BUTTON);
  vector->registerPatch = onRegisterPatch;
  vector->registerPatchParameter = onRegisterPatchParameter;
  vector->cycles_per_block = 0;
  vector->heap_bytes_used = 0;
  vector->programReady = onProgramReady;
  vector->programStatus = onProgramStatus;
  vector->serviceCall = serviceCall;
  vector->message = NULL;

  vector->setButton = onSetButton;
  vector->setPatchParameter = onSetPatchParameter;

  vector->buttonChangedCallback = NULL;
  vector->encoderChangedCallback = NULL;
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

#ifndef OWLRACK
  adcSetup();
#endif
  clockSetup();
#if defined OWLMODULAR
  setupSwitchA(pushGateCallback);
#elif !defined OWLRACK
  setupSwitchA(footSwitchCallback);
#endif
#ifndef OWLRACK
  setupSwitchB(pushButtonCallback);
#endif

  settings.init();
  midi.init(settings.midi_output_channel);
  midi_set_input_channel(settings.midi_input_channel);
  registry.init();

#ifdef OWLRACK

#endif

#ifdef EXPRESSION_PEDAL
  setupExpressionPedal();
#endif

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // DEBUG
  configureDigitalOutput(GPIOB, GPIO_Pin_1); // PB1, DEBUG LED
  debugClear();

#ifdef OWLMODULAR
  // Configure OWL Modular digital output  
  RCC_AHB1PeriphClockCmd(PUSH_GATE_OUT_CLK, ENABLE);
  configureDigitalOutput(PUSH_GATE_OUT_PORT, PUSH_GATE_OUT_PIN);
#endif
  clearButtonEvent(PUSHBUTTON);

#ifdef DEBUG_AUDIO
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // DEBUG
  configureDigitalOutput(GPIOA, GPIO_Pin_7); // PA7 DEBUG
  configureDigitalOutput(GPIOC, GPIO_Pin_5); // PC5 DEBUG
  clearPin(GPIOC, GPIO_Pin_5); // DEBUG
  clearPin(GPIOA, GPIO_Pin_7); // DEBUG
#endif /* DEBUG_AUDIO */

  usb_init();

// #if SERIAL_PORT == 1
//   setupSerialPort1(115200);
// #elif SERIAL_PORT == 2
//   setupSerialPort2(115200); // expression pedal
// #warning expression pedal jack configured as serial port
// #ifdef EXPRESSION_PEDAL
// #error invalid configuration
// #endif
// #elif SERIAL_PORT == 4
//   setupSerialPort4(115200); // Digital Bus
//   setupBus();
// #endif
#ifdef USART_PERIPH
  serial_setup(USART_BAUDRATE);
  bus_setup();
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

   void midi_set_output_channel(int8_t ch){
     midi.setOutputChannel(ch);
   }
#ifdef __cplusplus
}
#endif
