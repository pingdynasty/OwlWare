#include "Owl.h"
#include "arm_math.h"
#include "armcontrol.h"
#include "usbcontrol.h"
#include "owlcontrol.h"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "SharedMemory.h"
#include "ProgramManager.h"

#include "serial.h"
#include "clock.h"
#include "device.h"

#define DEBOUNCE(nm, ms) if(true){static uint32_t nm ## Debounce = 0; \
if(getSysTicks() < nm ## Debounce+(ms)) return; nm ## Debounce = getSysTicks();}

#define PATCHFLASH ((uint32_t)0x08040000)

CodecController codec;
MidiController midi;
ApplicationSettings settings;
PatchRegistry registry;
volatile bool bypass = false;

__attribute__ ((section (".sharedram")))
volatile SharedMemory smem;

bool getButton(PatchButtonId bid){
  return smem.buttons & (1<<bid);
}

void setButton(PatchButtonId bid, bool on){
  if(on)
    smem.buttons |= 1<<bid;
  else
    smem.buttons &= ~(1<<bid);
}

void updateButtons(){
  setButton(GREEN_BUTTON, getLed() == GREEN);
  setButton(RED_BUTTON, getLed() == RED);
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
    setButton(PUSHBUTTON, true);
    bypass = true;
    setLed(NONE);
    
  }else{
    setButton(PUSHBUTTON, false);
    bypass = false;
    if(getButton(GREEN_BUTTON))
       setLed(GREEN);
    else if(getButton(RED_BUTTON))
       setLed(RED);
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
    smem.patch_selected_id = settings.patch_red;
  }else if(getLed() == RED){
    setLed(GREEN);
    smem.patch_selected_id = settings.patch_green;
  }
  updateButtons();
  // midi.sendPatchParameterNames(); // todo: this should probably be requested from client
}

void pushButtonCallback(){
  DEBOUNCE(pushbutton, 200);
  if(isPushButtonPressed() && settings.patch_mode != PATCHMODE_SINGLE)
    toggleActiveSlot();
}

void exitProgram(){
  // disable audio processing
  codec.stop();
  // patches.reset();
  program.exit();
}

void resetProgram(){
  program.reset();
}

void run(){
  program.load((uint32_t*)PATCHFLASH, 64*1024);
  program.run(); // never returns
}

int collisions = 0;
int errors = 0;

// volatile bool doProcessAudio = false;
// volatile bool collision = false;
// uint16_t* source;
// uint16_t* dest;

// #ifdef DEBUG_DWT
// uint32_t dwt_count = 0;
// #endif

// __attribute__ ((section (".coderam")))
// void run(){
// #ifdef DEBUG_DWT
//   volatile unsigned int *DWT_CYCCNT = (volatile unsigned int *)0xE0001004; //address of the register
//   volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
//   volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register
//   *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
//   *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter
// #endif
//   for(;;){
//     if(doProcessAudio){
// #ifdef DEBUG_AUDIO
//       setPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
// #endif
// #ifdef DEBUG_DWT
//       *DWT_CYCCNT = 0; // reset the counter
// #endif
//       buffer.split(source);
//       patches.process(buffer);
//       buffer.comb(dest);
//       if(collision){
// 	collision = false;
// #ifdef DEBUG_AUDIO
// 	debugToggle();
// #endif
//       }else{
// 	doProcessAudio = false;
//       }
// #ifdef DEBUG_AUDIO
//       clearPin(GPIOC, GPIO_Pin_5); // PC5 DEBUG
// #endif
// #ifdef DEBUG_DWT
//       dwt_count = *DWT_CYCCNT;
// #endif
//     }
//   }
// }

#ifdef __cplusplus
 extern "C" {
#endif

void registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
  // hello!
  registry.registerPatch(name, inputChannels, outputChannels);
}

void registerPatchParameter(uint8_t id, const char* name){
  
}

#ifdef __cplusplus
}
#endif

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

  codec.setup();
  codec.init(settings);

  printString("startup\n");
  updateBypassMode();

  smem.checksum = sizeof(smem);
  smem.status = AUDIO_IDLE_STATUS;
  smem.audio_input = NULL;
  smem.audio_output = NULL;
  smem.audio_bitdepth = settings.audio_bitdepth;
  smem.audio_blocksize = 0;
  smem.audio_samplingrate = settings.audio_samplingrate;
  smem.patch_selected_id = settings.patch_green;
  smem.patch_mode = settings.patch_mode;
  smem.parameters = getAnalogValues();
  smem.parameters_size = NOF_ADC_VALUES;
  smem.buttons = 0;
  smem.error = 0;
  smem.registerPatch = registerPatch;
  smem.registerPatchParameter = registerPatchParameter;

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
  if(smem.status == AUDIO_ERROR_STATUS){
    errors++;
  }else if(smem.status != AUDIO_PROCESSED_STATUS){
    // oops
    collisions++;
  }
  smem.status = AUDIO_READY_STATUS;
  smem.audio_input = src;
  smem.audio_output = dst;
  smem.audio_blocksize = sz;
  // the blocksize here is the number of halfwords, ie 16bit fixed width ints, for both channels
}

#ifdef __cplusplus
}
#endif
