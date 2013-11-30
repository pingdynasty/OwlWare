#ifndef _MidiHandler_H_
#define _MidiHandler_H_

#include <string.h>
#include "owlcontrol.h"
#include "MidiReader.hpp"
#include "PatchRegistry.h"
#include "MidiController.h"
#include "ApplicationSettings.h"

class MidiHandler : public MidiReader {
private:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
public:
  MidiHandler() : MidiReader(buffer, sizeof(buffer))
  {}

  void sendSettings(){
    midi.sendCc(PATCH_PARAMETER_A, (getAnalogValue(0)>>5) & 0x7f);
    midi.sendCc(PATCH_PARAMETER_B, (getAnalogValue(1)>>5) & 0x7f);
    midi.sendCc(PATCH_PARAMETER_C, (getAnalogValue(2)>>5) & 0x7f);
    midi.sendCc(PATCH_PARAMETER_D, (getAnalogValue(3)>>5) & 0x7f);
    midi.sendCc(PATCH_PARAMETER_E, (getAnalogValue(4)>>5) & 0x7f);
    midi.sendCc(PATCH_BUTTON, getPushButton() ? 127 : 0);
    midi.sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
    midi.sendCc(PATCH_SLOT_GREEN, settings.patch_green);
    midi.sendCc(PATCH_SLOT_RED, settings.patch_red);
    midi.sendCc(ACTIVE_SLOT, getActiveSlot() == GREEN ? 0 : 127);
    midi.sendCc(LEFT_INPUT_GAIN, codec.getInputGainLeft()<<2);
    midi.sendCc(RIGHT_INPUT_GAIN, codec.getInputGainRight()<<2);
    midi.sendCc(LEFT_OUTPUT_GAIN, codec.getOutputGainLeft());
    midi.sendCc(RIGHT_OUTPUT_GAIN, codec.getOutputGainRight());
    midi.sendCc(LEFT_INPUT_MUTE, codec.getInputMuteLeft() ? 127 : 0);
    midi.sendCc(RIGHT_INPUT_MUTE, codec.getInputMuteRight() ? 127 : 0);
    midi.sendCc(LEFT_OUTPUT_MUTE, codec.getOutputMuteLeft() ? 127 : 0);
    midi.sendCc(RIGHT_OUTPUT_MUTE, codec.getOutputMuteRight() ? 127 : 0);
    midi.sendCc(BYPASS, codec.getBypass() ? 127 : 0);
    midi.sendCc(SAMPLING_RATE, (codec.getSamplingRate() >> 10) + 20);
    midi.sendCc(SAMPLING_BITS, (codec.getFormat() << 4) + 20);
    midi.sendCc(CODEC_MASTER, codec.isMaster() ? 127 : 0);
    midi.sendCc(CODEC_PROTOCOL, codec.getProtocol() == I2S_PROTOCOL_PHILIPS ? 0 : 127);
    midi.sendCc(SAMPLING_SIZE, settings.audio_blocksize>>4);
    midi.sendCc(LEFT_RIGHT_SWAP, codec.getSwapLeftRight());
    sendPatchNames();
    sendDeviceInfo();
  }

  void sendPatchNames(){
    for(int i=0; i<patches.getNumberOfPatches(); ++i)
      sendPatchName(i);
  }

  void sendPatchName(uint8_t index){
    std::string name = patches.getName(index);
    uint8_t buffer[name.size()+3];
    buffer[0] = SYSEX_PRESET_NAME_COMMAND;
    buffer[1] = index;
    memcpy(buffer+2, name.c_str(), name.size()+1);
    midi.sendSysEx(buffer, sizeof(buffer));
  }

  void sendDeviceInfo(){
    sendFirmwareVersion();
    sendDeviceId();
  }

  void sendFirmwareVersion(){
    char* version = getFirmwareVersion();
    uint8_t len = strlen(version);
    uint8_t buffer[len+1];
    buffer[0] = SYSEX_FIRMWARE_VERSION;
    memcpy(buffer+1, version, len);
    midi.sendSysEx(buffer, sizeof(buffer));
  }

  void sendDeviceId(){
    uint8_t buffer[3*4+1];
    buffer[0] = SYSEX_DEVICE_ID;
    getDeviceId((uint32_t*)(buffer+1), (uint32_t*)(buffer+4+1), (uint32_t*)(buffer+8+1));
    midi.sendSysEx(buffer, sizeof(buffer));
  }

  void handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
    switch(cc){
    case PATCH_BUTTON:
      if(value == 127)
	toggleActiveSlot();
      break;
    case LED:
      if(value < 42){
	setLed(NONE);
      }else if(value > 83){
	setLed(RED);
      }else{
	setLed(GREEN);
      }
      break;
    case PATCH_SLOT_GREEN:
      settings.patch_green = value;
      break;
    case PATCH_SLOT_RED:
      settings.patch_red = value;
      break;
    case ACTIVE_SLOT:
      setActiveSlot(value == 127 ? RED : GREEN);
      break;
    case LEFT_INPUT_GAIN:
      codec.setInputGainLeft(value>>2);
      break;
    case RIGHT_INPUT_GAIN:
      codec.setInputGainRight(value>>2);
      break;
    case LEFT_OUTPUT_GAIN:
      codec.setOutputGainLeft(value);
      break;
    case RIGHT_OUTPUT_GAIN:
      codec.setOutputGainRight(value);
      break;
    case LEFT_OUTPUT_MUTE:
      codec.setOutputMuteLeft(value == 127);
      break;
    case RIGHT_OUTPUT_MUTE:
      codec.setOutputMuteRight(value == 127);
      break;
    case BYPASS:
      codec.setBypass(value == 127);
      break;
    case LEFT_INPUT_MUTE:
      codec.setInputMuteLeft(value == 127);
      break;
    case RIGHT_INPUT_MUTE:
      codec.setInputMuteRight(value == 127);
      break;
    case SAMPLING_RATE:
      uint32_t frequency;
      if(value < 32){
	frequency = 8000;
      }else if(value < 64){
	frequency = 32000;
      }else if(value < 96){
	frequency = 48000;
      }else{
	frequency = 96000;
      }
      if(frequency != settings.audio_samplingrate){
	settings.audio_samplingrate = frequency;
	codec.stop();
	codec.init(settings);
	codec.start();
      }
      break;
    case SAMPLING_BITS: {
      I2SFormat format;
      if(value < 42){
	format = I2S_FORMAT_16bit;
      }else if(value < 84){
	format = I2S_FORMAT_24bit;
      }else{
	format = I2S_FORMAT_32bit;
      }      
      if(format != settings.audio_codec_format){
	settings.audio_codec_format = format;
	codec.stop();
	codec.init(settings);
	codec.start();
      }
      break;
    }
    case CODEC_MASTER: {
      bool master = value > 63;
      if(master != settings.audio_codec_master){
	settings.audio_codec_master = master;
	codec.stop();
	codec.init(settings);
	codec.start();
      }
      break;
    }
    case CODEC_PROTOCOL: {
      I2SProtocol protocol;
      if(value < 64){
	protocol = I2S_PROTOCOL_PHILIPS;
      }else{
	protocol = I2S_PROTOCOL_MSB;
      }
      if(protocol != settings.audio_codec_protocol){
	settings.audio_codec_protocol = protocol;
	codec.stop();
	codec.init(settings);
	codec.start();
      }
      break;
    }
    case SAMPLING_SIZE: {
      uint32_t blocksize = value << 4;
      if(settings.audio_blocksize != blocksize){
	settings.audio_blocksize = blocksize;
	codec.stop();
	codec.init(settings);
	codec.start();
      }
      break;
    }
    case LEFT_RIGHT_SWAP:
      codec.setSwapLeftRight(value == 127);
      break;
    case REQUEST_SETTINGS:
      switch(value){
      case PATCH_BUTTON:
	midi.sendCc(PATCH_BUTTON, getPushButton() ? 127 : 0);
	break;
      case LED:
	midi.sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
	break;
      case 127:
	sendSettings();
	break;
      }
      break;
    case SAVE_SETTINGS:
      if(value == 127){
	toggleLed();
	settings.saveToFlash();
	toggleLed();
      }
      break;
    case DEVICE_FIRMWARE_UPDATE:
      if(value == 127)
	jump_to_bootloader();
      break;
    case FACTORY_RESET:
      if(value == 127){
	toggleLed();
	settings.reset();
	// settings.clearFlash();
	toggleLed();
      }
      break;
    }
  }

  void handleSystemCommon(uint8_t){
  }

  void handleSysEx(uint8_t* data, uint8_t size){
    if(size < 3 || 
       data[1] != MIDI_SYSEX_MANUFACTURER || 
       data[2] != MIDI_SYSEX_DEVICE)
      return;
    switch(data[4]){
    case SYSEX_DFU_COMMAND:
      jump_to_bootloader();
      break;
    }
  }
};

#endif /* _MidiHandler_H_ */
