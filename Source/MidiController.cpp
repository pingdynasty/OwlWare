#include <errno.h>
#include <string.h>
#include "owlcontrol.h"
#include "midicontrol.h"
#include "MidiStatus.h"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "ProgramVector.h"
#include "ProgramManager.h"
#include <math.h> /* for ceilf */

uint32_t log2(uint32_t x){ 
  return x == 0 ? 0 : 31 - __builtin_clz (x); /* clz returns the number of leading 0's */
}

void MidiController::init(uint8_t ch){
  channel = ch;
}

void MidiController::sendSettings(){
  sendPc(settings.program_index);
  sendCc(PATCH_PARAMETER_A, (uint8_t)(getAnalogValue(PARAMETER_A)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(getAnalogValue(PARAMETER_B)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(getAnalogValue(PARAMETER_C)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(getAnalogValue(PARAMETER_D)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_E, (uint8_t)(getAnalogValue(PARAMETER_E)>>5) & 0x7f);
  sendCc(PATCH_BUTTON, isPushButtonPressed() ? 127 : 0);
  sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
  sendCc(LEFT_INPUT_GAIN, codec.getInputGainLeft()<<2);
  sendCc(RIGHT_INPUT_GAIN, codec.getInputGainRight()<<2);
  sendCc(LEFT_OUTPUT_GAIN, codec.getOutputGainLeft());
  sendCc(RIGHT_OUTPUT_GAIN, codec.getOutputGainRight());
  sendCc(LEFT_INPUT_MUTE, codec.getInputMuteLeft() ? 127 : 0);
  sendCc(RIGHT_INPUT_MUTE, codec.getInputMuteRight() ? 127 : 0);
  sendCc(LEFT_OUTPUT_MUTE, codec.getOutputMuteLeft() ? 127 : 0);
  sendCc(RIGHT_OUTPUT_MUTE, codec.getOutputMuteRight() ? 127 : 0);
  sendCc(BYPASS, codec.getBypass() ? 127 : 0);

  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_RATE, settings.audio_samplingrate);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_WIDTH, settings.audio_bitdepth);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE, settings.audio_blocksize);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_MASTER, settings.audio_codec_master);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_PROTOCOL, settings.audio_codec_protocol);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_BYPASS, settings.audio_codec_bypass);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_HALFSPEED, settings.audio_codec_halfspeed);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_SWAP, settings.audio_codec_swaplr);
}

void MidiController::sendPatchParameterNames(){
  // PatchProcessor* processor = patches.getActivePatchProcessor();
  // for(int i=0; i<NOF_ADC_VALUES; ++i){
  //   PatchParameterId pid = (PatchParameterId)i;
  //   const char* name = processor->getParameterName(pid);
  //   if(name != NULL)
  //     sendPatchParameterName(pid, name);
  //   else
  //     sendPatchParameterName(pid, "");
  // }
}

void MidiController::sendPatchParameterName(PatchParameterId pid, const char* name){
  uint8_t size = strlen(name);
  uint8_t buffer[size+3];
  buffer[0] = SYSEX_PARAMETER_NAME_COMMAND;
  buffer[1] = pid;
  memcpy(buffer+2, name, size+1);
  sendSysEx(buffer, sizeof(buffer));
}

void MidiController::sendPatchNames(){
  for(unsigned int i=0; i<registry.getNumberOfPatches(); ++i)
    sendPatchName(i);
}

void MidiController::sendPatchName(uint8_t index){
  const char* name = registry.getName(index);
  if(name != NULL){
    uint8_t size = strlen(name);
    // uint8_t size = strnlen(name, 16);
    uint8_t buffer[size+3];
    buffer[0] = SYSEX_PRESET_NAME_COMMAND;
    buffer[1] = index;
    memcpy(buffer+2, name, size+1);
    sendSysEx(buffer, sizeof(buffer));
  }
}

void MidiController::sendDeviceInfo(){
  sendFirmwareVersion();
  sendProgramMessage();
  sendProgramStats();
#ifdef DEBUG_STACK
  sendDeviceStats();
#endif /* DEBUG_STACK */
}

#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif /* abs */
char* itoa(int val, int base){
  static char buf[13] = {0};
  int i = 11;
  unsigned int part = abs(val);
  do{
    buf[i--] = "0123456789abcdef"[part % base];
    part /= base;
  }while(part && i);
  if(val < 0)
    buf[i--] = '-';
  return &buf[i+1];
}

#include <string.h>

#ifdef DEBUG_STACK
void MidiController::sendDeviceStats(){
  char buffer[64];
  buffer[0] = SYSEX_DEVICE_STATS;
  char* p = &buffer[1];
  p = stpcpy(p, (const char*)"Program stack: ");
  p = stpcpy(p, itoa(program.getProgramStackSize(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, itoa(program.getProgramStackAllocation(), 10));
  sendSysEx((uint8_t*)buffer, p-buffer);
}
#endif /* DEBUG_STACK */

void MidiController::sendProgramStats(){
  char buffer[64];
  buffer[0] = SYSEX_PROGRAM_STATS;
  char* p = &buffer[1];
  uint8_t err = getErrorStatus();
  switch(err & 0xf0){
  case NO_ERROR: {
    p = stpcpy(p, (const char*)"CPU ");
    float percent = (program.getCyclesPerBlock()/settings.audio_blocksize) / (float)3500;
    p = stpcpy(p, itoa(ceilf(percent*100), 10));
    p = stpcpy(p, (const char*)"% Heap ");
    percent = program.getHeapMemoryUsed()/(float)(1024*1024);
    p = stpcpy(p, itoa(ceilf(percent*100), 10));
    p = stpcpy(p, (const char*)"%");
    break;
  }
  case MEM_ERROR:
    p = stpcpy(p, (const char*)"Memory Error 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  case BUS_ERROR:
    p = stpcpy(p, (const char*)"Bus Error 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  case USAGE_ERROR:
    p = stpcpy(p, (const char*)"Usage Error 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  case NMI_ERROR:
    p = stpcpy(p, (const char*)"Non-maskable Interrupt 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  case HARDFAULT_ERROR:
    p = stpcpy(p, (const char*)"HardFault Error 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  case PROGRAM_ERROR:
    p = stpcpy(p, (const char*)"Missing or Invalid Program 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  default:
    p = stpcpy(p, (const char*)"Unknown Error 0x");
    p = stpcpy(p, itoa(err, 16));
    break;
  }
  sendSysEx((uint8_t*)buffer, p-buffer);
}

void MidiController::sendProgramMessage(){
  ProgramVector* smem = getProgramVector();
  if(smem != NULL && smem->message != NULL){
    char buffer[64];
    buffer[0] = SYSEX_PROGRAM_MESSAGE;
    char* p = &buffer[1];
    p = stpncpy(p, smem->message, 63);
    sendSysEx((uint8_t*)buffer, p-buffer);    
  }
}

void MidiController::sendFirmwareVersion(){
  char buffer[64];
  buffer[0] = SYSEX_FIRMWARE_VERSION;
  char* p = &buffer[1];
  p = stpcpy(p, getFirmwareVersion());
  sendSysEx((uint8_t*)buffer, p-buffer);
}

void MidiController::sendConfigurationSetting(const char* name, uint32_t value){
  char buffer[16];
  buffer[0] = SYSEX_CONFIGURATION_COMMAND;
  char* p = &buffer[1];
  p = stpcpy(p, name);
  p = stpcpy(p, itoa(value, 16));
  sendSysEx((uint8_t*)buffer, p-buffer);
}

void MidiController::sendDeviceId(){
  uint32_t* deviceId = getDeviceId();
  char buffer[36];
  buffer[0] = SYSEX_DEVICE_ID;
  char* p = &buffer[1];
  for(int i=0; i<3; i++){
    p = stpcpy(p, itoa(deviceId[i], 16));
    p = stpcpy(p, ".");
  }
  sendSysEx((uint8_t*)buffer, p-buffer);
}

void MidiController::sendSelfTest(){
  // this code somehow leads to a HardFault once the interrupt has completed
  // uint8_t buffer[2];
  // buffer[0] = SYSEX_SELFTEST;
  // bool hse = isClockExternal();
  // bool mem = SRAM_TestMemory();
  // bool epr = settings.settingsInFlash();
  // buffer[1] = (hse << 2) | ( mem << 1) | epr;
  // // buffer[1] = (hse << 2) | ( SRAM_TestMemory() << 1) | settings.settingsInFlash();
  // sendSysEx(buffer, sizeof(buffer));

  // untested
  // int error = errno;
  // if(error != 0){
  //   buffer[2] = error;
  //   buffer[3] = error >> 7;
  //   sendSysEx(buffer, 4);
  // }else{
  //   sendSysEx(buffer, 2);
  // }
}

void MidiController::sendPc(uint8_t pc){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_PROGRAM_CHANGE,
			  (uint8_t)(PROGRAM_CHANGE | channel),
			  pc, 0 };
    midi_send_usb_buffer(packet, sizeof(packet));
  }
}

void MidiController::sendCc(uint8_t cc, uint8_t value){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_CONTROL_CHANGE,
			  (uint8_t)(CONTROL_CHANGE | channel),
			  cc, value };
    midi_send_usb_buffer(packet, sizeof(packet));
  }
}

/**
 * 
 */
void MidiController::sendSysEx(uint8_t* data, uint16_t size){
  /* USB-MIDI devices transmit sysex messages in 4-byte packets which
   * contain a status byte and up to 3 bytes of the message itself.
   * If the message ends with fewer than 3 bytes, a different code is
   * sent. Go through the sysex 3 bytes at a time, including the leading
   * 0xF0 and trailing 0xF7.
   */
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_SYSEX, SYSEX, MIDI_SYSEX_MANUFACTURER, MIDI_SYSEX_DEVICE };
    midi_send_usb_buffer(packet, sizeof(packet));
    int count = size/3;
    uint8_t* src = data;
    while(count-- > 0){
      packet[1] = (*src++ & 0x7f);
      packet[2] = (*src++ & 0x7f);
      packet[3] = (*src++ & 0x7f);
      midi_send_usb_buffer(packet, sizeof(packet));
    }
    count = size % 3;
    switch(count){
    case 0:
      packet[0] = USB_COMMAND_SYSEX_EOX1;
      packet[1] = SYSEX_EOX;
      packet[2] = 0;
      packet[3] = 0;
      break;
    case 1:
      packet[0] = USB_COMMAND_SYSEX_EOX2;
      packet[1] = (*src++ & 0x7f);
      packet[2] = SYSEX_EOX;
      packet[3] = 0;
      break;
    case 2:
      packet[0] = USB_COMMAND_SYSEX_EOX3;
      packet[1] = (*src++ & 0x7f);
      packet[2] = (*src++ & 0x7f);
      packet[3] = SYSEX_EOX;
      break;
    }
    midi_send_usb_buffer(packet, sizeof(packet));
  }
}

