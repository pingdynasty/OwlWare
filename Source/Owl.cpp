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
#include "bkp_sram.h"

// #include "serial.h"
#include "clock.h"
#include "device.h"
#include "codec.h"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; \
if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

#define PATCHFLASH ((uint32_t)0x08040000)

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry registry;
volatile bool bypass = false;

// uint16_t getParameterValue(PatchParameterId pid){
//   return getProgramVector()->parameters[pid];
// }

bool getButton(PatchButtonId bid){
  return getProgramVector()->buttons & (1<<bid);
  // return false;
}

void setButton(PatchButtonId bid, bool on){
  if(on)
    getProgramVector()->buttons |= 1<<bid;
  else
    getProgramVector()->buttons &= ~(1<<bid);
}

void updateLed(){
  if(getButton(GREEN_BUTTON))  
    setLed(GREEN);
  else if(getButton(RED_BUTTON))
    setLed(RED);
  else
    setLed(NONE);
  midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
}

void updateBypassMode(){
#ifdef OWLMODULAR
  bypass = false;
#else
  if(isStompSwitchPressed()){
    setButton(BYPASS_BUTTON, true);
    bypass = true;
    setLed(NONE);    
  }else{
    setButton(BYPASS_BUTTON, false);
    bypass = false;
    updateLed();
  }
#endif
}

void footSwitchCallback(){
  DEBOUNCE(bypass, 200);
  updateBypassMode();
}

void toggleActiveSlot(){
  if(getLed() == GREEN){
    setLed(RED);
    setButton(RED_BUTTON, true);
    setButton(GREEN_BUTTON, false);
  }else{ // if(getLed() == RED){
    setLed(GREEN);
    setButton(GREEN_BUTTON, true);
    setButton(RED_BUTTON, false);
  }
}

void pushButtonCallback(){
  DEBOUNCE(pushbutton, 200);
  if(isPushButtonPressed()){
    setButton(PUSHBUTTON, true);
  }else{
    setButton(PUSHBUTTON, false);
  }
}

void exitProgram(){
  // disable audio processing
  // codec.stop(); // codec.start() doesn't recover well
  program.exit();
  // registry.reset();
  codec.clear();
  setLed(RED);
  registry.setDynamicPatchDefinition(NULL);
}

void updateProgramIndex(uint8_t index){
  settings.program_index = index;
  midi.sendPc(index);
}

// void resetProgram(){
//   program.reset();
//   registry.setDynamicPatchDefinition(NULL);
// }

// void run(){
//   program.load((uint32_t*)PATCHFLASH, 64*1024);
//   program.run(); // never returns
//   // for(;;);
// }

#ifdef __cplusplus
 extern "C" {
#endif

   PatchDefinition dynamicPatchDefinition;
   void registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
     dynamicPatchDefinition.name = name;
     dynamicPatchDefinition.inputs = inputChannels;
     dynamicPatchDefinition.outputs = outputChannels;
     registry.setDynamicPatchDefinition(&dynamicPatchDefinition);
   }

   void registerPatchParameter(uint8_t id, const char* name){
     midi.sendPatchParameterName((PatchParameterId)id, name);
   }

   // volatile bool doProcessAudio = false;
__attribute__ ((section (".coderam")))
   void programReady(){
     // doProcessAudio = false;
     // audioStatus = AUDIO_READY_STATUS;
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

void updateProgramVector(ProgramVector* smem){
  smem->checksum = sizeof(ProgramVector);
#ifdef OWLMODULAR
  smem->hardware_version = OWL_MODULAR_HARDWARE;
#else
  smem->hardware_version = OWL_PEDAL_HARDWARE;
#endif
  // smem->audio_input = NULL;
  // smem->audio_output = NULL;
  smem->audio_bitdepth = settings.audio_bitdepth;
  smem->audio_blocksize = settings.audio_blocksize;
  smem->audio_samplingrate = settings.audio_samplingrate;
  smem->parameters = getAnalogValues();
  smem->parameters_size = NOF_PARAMETERS;
  // todo: pass real-time updates from MidiHandler
  smem->buttons = (1<<GREEN_BUTTON);
  smem->registerPatch = registerPatch;
  smem->registerPatchParameter = registerPatchParameter;
  smem->cycles_per_block = 0;
  smem->heap_bytes_used = 0;
  smem->programReady = programReady;
  smem->programStatus = programStatus;
  smem->message = NULL;
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
  setupSwitchA(footSwitchCallback);
  setupSwitchB(pushButtonCallback);

  settings.init();
  midi.init(MIDI_CHANNEL);
  // patches.init();

#ifdef EXPRESSION_PEDAL
#ifndef OWLMODULAR
  setupExpressionPedal();
#endif
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

#ifdef OWLMODULAR
  configureDigitalInput(GPIOB, GPIO_Pin_6, GPIO_PuPd_NOPULL);  // PB6 OWL Modular digital input
  configureDigitalOutput(GPIOB, GPIO_Pin_7);  // PB7 OWL Modular digital output
  setPin(GPIOB, GPIO_Pin_7); // PB7 OWL Modular digital output
#endif
  // printString("startup\n");
  updateBypassMode();

  // set pointer to smem in the backup ram
  // uint32_t pointer = (uint32_t)&smem;
  // memcpy(BKPSRAM_GetMemoryAddress(), &pointer, 4);
  // ProgramVector* smemp = (ProgramVector*)BKPSRAM_GetMemoryAddress();
  // smemp = &smem;

  codec.setup();
  codec.init(settings);
  codec.start();

  program.loadProgram(settings.program_index);
  program.startProgram();
}

#ifdef __cplusplus
 extern "C" {
#endif

extern volatile ProgramVectorAudioStatus audioStatus;

__attribute__ ((section (".coderam")))
void audioCallback(int16_t *src, int16_t *dst){
#ifdef DEBUG_AUDIO
  togglePin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
#endif
  getProgramVector()->audio_input = src;
  getProgramVector()->audio_output = dst;
  // doProcessAudio = false;
  audioStatus = AUDIO_READY_STATUS;
  // program.audioReady();
}

#ifdef __cplusplus
}
#endif
