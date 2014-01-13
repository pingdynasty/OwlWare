#ifndef _MidiHandler_H_
#define _MidiHandler_H_

#include "OpenWareMidiControl.h"
#include "MidiReader.hpp"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"

class MidiHandler : public MidiReader {
private:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
public:
  MidiHandler() : MidiReader(buffer, sizeof(buffer))
  {}

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
      setActiveSlot(GREEN);
      break;
    case PATCH_SLOT_RED:
      settings.patch_red = value;
      setActiveSlot(RED);
      break;
    case ACTIVE_SLOT:
      setActiveSlot(value == 127 ? RED : GREEN);
      break;
    case LEFT_INPUT_GAIN:
      settings.inputGainLeft = value>>2;
      codec.setInputGainLeft(settings.inputGainLeft);
      break;
    case RIGHT_INPUT_GAIN:
      settings.inputGainRight = value>>2;
      codec.setInputGainRight(settings.inputGainRight);
      break;
    case LEFT_OUTPUT_GAIN:
      settings.outputGainLeft = value;
      codec.setOutputGainLeft(value);
      break;
    case RIGHT_OUTPUT_GAIN:
      settings.outputGainRight = value;
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
      case 0:
	midi.sendDeviceInfo();
	break;
      case 1:
	midi.sendPatchNames();
	break;
      case 2:
	midi.sendPatchParameterNames();
	break;
      case PATCH_BUTTON:
	midi.sendCc(PATCH_BUTTON, isPushButtonPressed() ? 127 : 0);
	break;
      case LED:
	midi.sendCc(LED, getLed() == NONE ? 0 : getLed() == GREEN ? 42 : 84);
	break;
      case 127:
	midi.sendSettings();
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
	settings.reset();
	settings.clearFlash();
	codec.init(settings);
	setActiveSlot(GREEN);
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
