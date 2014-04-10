#include <errno.h>
#include <string.h>
#include "MidiStatus.h"
#include "MidiController.h"
#include "CodecController.h"
#include "OpenWareMidiControl.h"
#include "PatchController.h"
#include "PatchRegistry.h"
#include "ApplicationSettings.h"
#include "fsmc_sram.h"
#include "owlcontrol.h"
#include "midicontrol.h"
#include "sysex.h"

void MidiController::init(uint8_t ch){
  channel = ch;
}

void MidiController::sendSettings(){
  sendCc(PATCH_PARAMETER_A, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_A)*127.0) & 0x7f);
  sendCc(PATCH_PARAMETER_B, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_B)*127.0) & 0x7f);
  sendCc(PATCH_PARAMETER_C, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_C)*127.0) & 0x7f);
  sendCc(PATCH_PARAMETER_D, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_D)*127.0) & 0x7f);
  sendCc(PATCH_PARAMETER_E, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_E)*127.0) & 0x7f);
  sendCc(PATCH_PARAMETER_F, (uint8_t)(patches.getCurrentPatchProcessor()->getParameterValue(PARAMETER_E)*127.0) & 0x7f);
  sendCc(PATCH_BUTTON, isPushButtonPressed() ? 127 : 0);
  sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
  sendCc(PATCH_MODE, settings.patch_mode << 5);
  sendCc(PATCH_SLOT_GREEN, settings.patch_green);
  sendCc(PATCH_SLOT_RED, settings.patch_red);
  sendCc(ACTIVE_SLOT, patches.getActiveSlot() == GREEN ? 0 : 127);
  sendCc(LEFT_INPUT_GAIN, codec.getInputGainLeft()<<2);
  sendCc(RIGHT_INPUT_GAIN, codec.getInputGainRight()<<2);
  sendCc(LEFT_OUTPUT_GAIN, codec.getOutputGainLeft());
  sendCc(RIGHT_OUTPUT_GAIN, codec.getOutputGainRight());
  sendCc(LEFT_INPUT_MUTE, codec.getInputMuteLeft() ? 127 : 0);
  sendCc(RIGHT_INPUT_MUTE, codec.getInputMuteRight() ? 127 : 0);
  sendCc(LEFT_OUTPUT_MUTE, codec.getOutputMuteLeft() ? 127 : 0);
  sendCc(RIGHT_OUTPUT_MUTE, codec.getOutputMuteRight() ? 127 : 0);
  sendCc(BYPASS, codec.getBypass() ? 127 : 0);
  sendCc(SAMPLING_RATE, (codec.getSamplingRate() >> 10) + 20);
  sendCc(SAMPLING_BITS, (codec.getFormat() << 4) + 20);
  sendCc(CODEC_MASTER, codec.isMaster() ? 127 : 0);
  sendCc(CODEC_PROTOCOL, codec.getProtocol() == I2S_PROTOCOL_PHILIPS ? 0 : 127);
  sendCc(SAMPLING_SIZE, settings.audio_blocksize>>4);
  sendCc(LEFT_RIGHT_SWAP, codec.getSwapLeftRight());
}

void MidiController::sendPatchParameterNames(){
  PatchProcessor* processor = patches.getCurrentPatchProcessor();
  for(int i=0; i<NOF_ADC_VALUES; ++i){
    PatchParameterId pid = (PatchParameterId)i;
    const char* name = processor->getParameterName(pid);
    if(name != NULL)
      sendPatchParameterName(pid, name);
    else
      sendPatchParameterName(pid, "");
  }
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
  uint8_t size = strlen(name);
  // uint8_t size = strnlen(name, 16);
  uint8_t buffer[size+3];
  buffer[0] = SYSEX_PRESET_NAME_COMMAND;
  buffer[1] = index;
  memcpy(buffer+2, name, size+1);
  sendSysEx(buffer, sizeof(buffer));
}

void MidiController::sendDeviceInfo(){
  sendFirmwareVersion();
}

void MidiController::sendFirmwareVersion(){
  char* version = getFirmwareVersion();
  uint8_t len = strlen(version);
  uint8_t buffer[len+1];
  buffer[0] = SYSEX_FIRMWARE_VERSION;
  memcpy(buffer+1, version, len);
  sendSysEx(buffer, sizeof(buffer));
}

void MidiController::sendDeviceId(){
  uint8_t buffer[15];
  buffer[0] = SYSEX_DEVICE_ID;
  uint8_t* deviceId = getDeviceId();
  data_to_sysex(deviceId, buffer+1, 3*4);
  sendSysEx(buffer, sizeof(buffer));
}

void MidiController::sendSelfTest(){
  // this code somehow leads to a HardFault once the interrupt has completed
  uint8_t buffer[2];
  buffer[0] = SYSEX_SELFTEST;
  buffer[1] = (isClockExternal() << 2) | ( SRAM_TestMemory() << 1) | settings.settingsInFlash();
  sendSysEx(buffer, sizeof(buffer));

  // int error = errno;
  // if(error != 0){
  //   buffer[2] = error;
  //   buffer[3] = error >> 7;
  //   sendSysEx(buffer, 4);
  // }else{
  //   sendSysEx(buffer, 2);
  // }
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

