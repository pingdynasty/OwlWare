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
#include "FlashStorage.h"
#include "Owl.h"
#include <math.h> /* for ceilf */
#include "message.h"
#include <string.h>

uint32_t log2(uint32_t x){ 
  return x == 0 ? 0 : 31 - __builtin_clz (x); /* clz returns the number of leading 0's */
}

void MidiController::init(uint8_t ch){
  channel = ch;
}

void MidiController::sendPatchParameterValues(){
#ifdef OWLMODULAR
  sendCc(PATCH_PARAMETER_A, (uint8_t)(127-(getParameterValue(PARAMETER_A)>>5)) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(127-(getParameterValue(PARAMETER_B)>>5)) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(127-(getParameterValue(PARAMETER_C)>>5)) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(127-(getParameterValue(PARAMETER_D)>>5)) & 0x7f);
#else /* OWLMODULAR */
  sendCc(PATCH_PARAMETER_A, (uint8_t)(getParameterValue(PARAMETER_A)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(getParameterValue(PARAMETER_B)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(getParameterValue(PARAMETER_C)>>5) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(getParameterValue(PARAMETER_D)>>5) & 0x7f);
#endif /* OWLMODULAR */
  sendCc(PATCH_PARAMETER_E, (uint8_t)(getParameterValue(PARAMETER_E)>>5) & 0x7f);
}

void MidiController::sendSettings(){
  sendPc(settings.program_index);
  sendPatchParameterValues();
  sendCc(PATCH_BUTTON, getButton(PUSHBUTTON) ? 127 : 0);
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
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BITDEPTH, settings.audio_bitdepth);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_DATAFORMAT, settings.audio_dataformat);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE, settings.audio_blocksize);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_MASTER, settings.audio_codec_master);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_PROTOCOL, settings.audio_codec_protocol);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_BYPASS, settings.audio_codec_bypass);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_HALFSPEED, settings.audio_codec_halfspeed);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_CODEC_SWAP, settings.audio_codec_swaplr);
  sendConfigurationSetting((const char*)SYSEX_CONFIGURATION_PC_BUTTON, settings.program_change_button);
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
  uint8_t size = strnlen(name, 24);
  uint8_t buffer[size+2];
  buffer[0] = SYSEX_PARAMETER_NAME_COMMAND;
  buffer[1] = pid;
  memcpy(buffer+2, name, size);
  sendSysEx(buffer, sizeof(buffer));
}

void MidiController::sendPatchNames(){
  for(uint8_t i=0; i<registry.getNumberOfPatches(); ++i)
    sendPatchName(i);
  sendPc(settings.program_index);
}

void MidiController::sendPatchName(uint8_t index){
  const char* name = registry.getName(index);
  if(name != NULL){
    uint8_t size = strnlen(name, 24);
    uint8_t buffer[size+2];
    buffer[0] = SYSEX_PRESET_NAME_COMMAND;
    buffer[1] = index;
    memcpy(buffer+2, name, size);
    sendSysEx(buffer, sizeof(buffer));
  }
}

void MidiController::sendDeviceInfo(){
  sendFirmwareVersion();
  sendProgramMessage();
  sendProgramStats();
  sendDeviceStats();
}

void MidiController::sendDeviceStats(){
  char buffer[64];
  buffer[0] = SYSEX_DEVICE_STATS;
  char* p = &buffer[1];
#ifdef DEBUG_STACK
  p = stpcpy(p, (const char*)"Program Stack ");
  p = stpcpy(p, itoa(program.getProgramStackUsed(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, itoa(program.getProgramStackAllocation(), 10));
  p = stpcpy(p, (const char*)" Manager ");
  p = stpcpy(p, itoa(program.getManagerStackUsed(), 10));
  p = stpcpy(p, (const char*)"/");
  p = stpcpy(p, itoa(program.getManagerStackAllocation(), 10));
  p = stpcpy(p, (const char*)" Free ");
  p = stpcpy(p, itoa(program.getFreeHeapSize(), 10));
  sendSysEx((uint8_t*)buffer, p-buffer);
#endif /* DEBUG_STACK */
  p = stpcpy(p, (const char*)"Storage used ");
  p = stpcpy(p, itoa(storage.getTotalUsedSize(), 10));
  p = stpcpy(p, (const char*)" deleted ");
  p = stpcpy(p, itoa(storage.getDeletedSize(), 10));
  p = stpcpy(p, (const char*)" free ");
  p = stpcpy(p, itoa(storage.getFreeSize(), 10));
  p = stpcpy(p, (const char*)" total ");
  p = stpcpy(p, itoa(storage.getTotalAllocatedSize(), 10));
  sendSysEx((uint8_t*)buffer, p-buffer);
}

void MidiController::sendProgramStats(){
  char buffer[64];
  buffer[0] = SYSEX_PROGRAM_STATS;
  char* p = &buffer[1];
  uint8_t err = getErrorStatus();
  switch(err & 0xf0){
  case NO_ERROR: {
#ifdef DEBUG_DWT
    p = stpcpy(p, (const char*)"CPU: ");
    float percent = (program.getCyclesPerBlock()/settings.audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
    p = stpcpy(p, itoa(ceilf(percent*100), 10));
    p = stpcpy(p, (const char*)"% ");
#endif /* DEBUG_DWT */
#ifdef DEBUG_STACK
    p = stpcpy(p, (const char*)"Stack: ");
    int stack = program.getProgramStackUsed();
    p = stpcpy(p, itoa(stack, 10));
    p = stpcpy(p, (const char*)" Heap: ");
#else
    p = stpcpy(p, (const char*)"Memory: ");
#endif /* DEBUG_STACK */
    int mem = program.getHeapMemoryUsed();
    p = stpcpy(p, itoa(mem, 10));
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
  const char* msg;
  if(getErrorStatus())
    msg = getErrorMessage();
  else
    msg = getDebugMessage();
  if(msg != NULL){
    char buffer[64];
    buffer[0] = SYSEX_PROGRAM_MESSAGE;
    char* p = &buffer[1];
    p = stpncpy(p, msg, 62);
    sendSysEx((uint8_t*)buffer, p-buffer);
    debugMessage(NULL);
  }
}

void MidiController::sendFirmwareVersion(){
  char buffer[32];
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
  char buffer[32];
  buffer[0] = SYSEX_DEVICE_ID;
  char* p = &buffer[1];
  p = stpcpy(p, itoa(deviceId[0], 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, itoa(deviceId[1], 16, 8));
  p = stpcpy(p, ":");
  p = stpcpy(p, itoa(deviceId[2], 16, 8));
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

void MidiController::sendNoteOff(uint8_t note, uint8_t velocity){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_NOTE_OFF,
			  (uint8_t)(NOTE_OFF | channel),
			  note, velocity };
    midi_send_usb_buffer(packet, sizeof(packet));
  }
}

void MidiController::sendNoteOn(uint8_t note, uint8_t velocity){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_NOTE_ON,
			  (uint8_t)(NOTE_ON | channel),
			  note, velocity };
    midi_send_usb_buffer(packet, sizeof(packet));
  }
}

void MidiController::sendPitchBend(uint16_t value){
  if(midi_device_connected()){
    uint8_t packet[4] = { USB_COMMAND_PITCH_BEND_CHANGE,
			  (uint8_t)(PITCH_BEND_CHANGE | channel),
			  (uint8_t)(value & 0x7f), (uint8_t)((value>>7) & 0x7f) };
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

