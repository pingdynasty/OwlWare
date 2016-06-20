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
#include "MidiStatus.h"
#include "bkp_sram.h"

// #include "serial.h"
#include "clock.h"
#include "device.h"
#include "codec.h"
#include "BitState.hpp"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; \
if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry registry;

// there are only really 2 timestamps needed: LED pushbutton and midi gate
uint16_t timestamps[NOF_BUTTONS]; 
BitState32 stateChanged;

bool getButton(PatchButtonId bid){
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
  stateChanged.clear(bid);
  clearButtonState(bid);
}

static void setPushbutton(){
  setGate();
  setButtonEvent(PUSHBUTTON);
  setButtonColour(RED);
}

static void clearPushbutton(){
  clearGate();
  clearButtonEvent(PUSHBUTTON);
  setButtonColour(GREEN);
}

static void updateBypassMode(){
  if(isStompSwitchPressed()){
    setButtonState(BYPASS_BUTTON);
    setButtonColour(NONE);
  }else{
    clearButtonState(BYPASS_BUTTON);
    clearButtonEvent(BYPASS_BUTTON);
    if(getButton(RED_BUTTON))
      setButtonColour(RED);
    else
      setButtonColour(GREEN);
  }
}

void togglePushButton(){
  if(getLed() == GREEN)
    setPushbutton();
  else // if(getLed() == RED){
    clearPushbutton();
}

#ifdef OWLMODULAR
static void pushGateCallback(){
  if(isPushGatePressed()){
    setButtonEvent(PUSHBUTTON);
    setButtonColour(RED);
    // we don't call setPushButton() because we don't want to set gate
  }else{
    clearButtonEvent(PUSHBUTTON);
    setButtonColour(GREEN);
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
    pushButtonPressed = getSysTicks();
    setPushbutton();
  }else{
    pushButtonPressed = 0;
    clearPushbutton();
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

#ifdef DEBUG_DWT
extern volatile uint32_t *DWT_CYCCNT;
#endif /* DEBUG_DWT */
extern volatile ProgramVectorAudioStatus audioStatus;

   __attribute__ ((section (".coderam")))
   // called from program
   void onProgramReady(){
     // program.programReady();
#ifdef DEBUG_DWT
     getProgramVector()->cycles_per_block = *DWT_CYCCNT;
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
     if(getProgramVector()->buttonChangedCallback != NULL && stateChanged.getState()){
       int bid = stateChanged.getFirstSetIndex();
       do{
	 getProgramVector()->buttonChangedCallback(bid, 4095, timestamps[bid]);
	 timestamps[bid] = 0;
	 bid = stateChanged.getFirstSetIndex();
       }while(bid > 0); // bid 0 is bypass button which we ignore
     }
     stateChanged.clear();
   }

   // called from program
   void onProgramStatus(ProgramVectorAudioStatus status){
     program.programStatus(status);
   }

   // called from program
   void onSetButton(uint8_t bid, uint16_t state, uint16_t samples){
     if(bid == PUSHBUTTON){
       if(state){
	 // we don't call setPushbutton() because we don't want to 
	 // call setButtonEvent (and set timestamp / stateChanged)
	 if(!getButton(PUSHBUTTON)){
	   setGate();
	   setButtonColour(RED);
	   setButtonState(bid);
	   midi.sendCc(LED, RED);
	 }
       }else{
	 if(getButton(PUSHBUTTON)){
	   clearGate();
	   setButtonColour(GREEN);
	   clearButtonState(bid);
	   midi.sendCc(LED, GREEN);
	 }
       }
     }else if(bid < NOF_BUTTONS){
       if(state)
	 setButtonState(bid);
       else
	 clearButtonState(bid);
     }else if(bid >= MIDI_NOTE_BUTTON){
       if(state)
	 midi.sendNoteOn(bid-MIDI_NOTE_BUTTON, (state>>5) & 0x7f);
       else
	 midi.sendNoteOff(bid-MIDI_NOTE_BUTTON, 0);
     }
   }

   // called from program
   void onSetPatchParameter(uint8_t pid, uint16_t value){     
     // if(pid < NOF_PARAMETERS){
     //   getProgramVector()->parameters[pid] = value;
     // }
     switch(pid){
     case PARAMETER_A:
     case PARAMETER_B:
     case PARAMETER_C:
     case PARAMETER_D:
     case PARAMETER_E:
     // case PARAMETER_F:
     // case PARAMETER_G:
     // case PARAMETER_H:
       // getProgramVector()->parameters[pid] = value;
       break;
       // case PARAMETER_MIDI_PITCH:
       // case PARAMETER_MIDI_AMPLITUDE:
     case PARAMETER_F: // CC1
       midi.sendCc(MIDI_CC_MODULATION, (value>>5) & 0x7f);
       break;
     // case PARAMETER_MIDI_BREATH:     // CC2
     //   midi.sendCc(2, (value>>5) & 0x7f);
     //   break;
     // case PARAMETER_MIDI_VOLUME:     // CC7
     //   midi.sendCc(7, (value>>5) & 0x7f);
     //   break;
     // case PARAMETER_MIDI_BALANCE:    // CC8
     //   midi.sendCc(8, (value>>5) & 0x7f);
     //   break;
     // case PARAMETER_MIDI_PAN:        // CC10
     //   midi.sendCc(10, (value>>5) & 0x7f);
     //   break;
     // case PARAMETER_MIDI_EXPRESSION: // CC11
     //   midi.sendCc(11, (value>>5) & 0x7f);
     //   break;
     case PARAMETER_G:    // CC12
       midi.sendCc(MIDI_CC_EFFECT_CTRL_1, (value>>5) & 0x7f);
       break;
     case PARAMETER_H:    // CC13
       midi.sendCc(MIDI_CC_EFFECT_CTRL_2, (value>>5) & 0x7f);
       break;
     default:
       if(pid >= PARAMETER_AA && pid <= PARAMETER_BH){
	 midi.sendCc(PATCH_PARAMETER_AA+(pid-PARAMETER_AA), (value>>5) & 0x7f);	 
       }else if(pid >= PARAMETER_MIDI_NOTE){
	 if(value == 0)
	   midi.sendNoteOff(pid-PARAMETER_MIDI_NOTE, 0);
	 else
	   midi.sendNoteOn(pid-PARAMETER_MIDI_NOTE, value);
       }
     }
   }

   // called from midi irq
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

   // called from midi irq
   void setParameter(uint8_t pid, uint16_t value){
     ASSERT(pid < getProgramVector()->parameters_size, "Parameter ID out of range");
     getProgramVector()->parameters[pid] = value;
   }

   uint16_t getParameterValue(uint8_t pid){
     ASSERT(pid < getProgramVector()->parameters_size, "Parameter ID out of range");
     return getProgramVector()->parameters[pid];
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
  clearButtonEvent(PUSHBUTTON);

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
