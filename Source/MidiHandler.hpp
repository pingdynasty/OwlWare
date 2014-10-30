#ifndef _MidiHandler_H_
#define _MidiHandler_H_

#include <string.h>
#include "device.h"
#include "OpenWareMidiControl.h"
#include "MidiReader.hpp"
#include "MidiController.h"
#include "CodecController.h"
#include "PatchController.h"
#include "ApplicationSettings.h"
#include "FirmwareLoader.hpp"
#include "ProgramManager.h"

uint16_t midi_values[NOF_ADC_VALUES];

class MidiHandler : public MidiReader {
private:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
public:
  MidiHandler() : MidiReader(buffer, sizeof(buffer)) {
    memset(midi_values, 0, sizeof(midi_values));
  }

  void handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
    switch(cc){
    case PATCH_PARAMETER_A:
      // scale from 7bit to 12bit value
      midi_values[PARAMETER_A] = value<<5;
      break;
    case PATCH_PARAMETER_B:
      midi_values[PARAMETER_B] = value<<5;
      break;
    case PATCH_PARAMETER_C:
      midi_values[PARAMETER_C] = value<<5;
      break;
    case PATCH_PARAMETER_D:
      midi_values[PARAMETER_D] = value<<5;
      break;
    case PATCH_PARAMETER_E:
      midi_values[PARAMETER_E] = value<<5;
      break;
    case PATCH_CONTROL:
      if(value == 127){
	memcpy(midi_values, getAnalogValues(), sizeof(midi_values));
	patches.setParameterValues(midi_values);
      }else{
	patches.setParameterValues(getAnalogValues());
      }
      break;
    case PATCH_BUTTON:
      if(value == 127)
	patches.toggleActiveSlot();
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
    case PATCH_MODE:
      settings.patch_mode = value >> 5;
      patches.setActiveSlot(patches.getActiveSlot());
      break;
    case PATCH_SLOT_GREEN:
      patches.setPatch(GREEN, value);
      break;
    case PATCH_SLOT_RED:
      patches.setPatch(RED, value);
      break;
    case ACTIVE_SLOT:
      patches.setActiveSlot(value == 127 ? RED : GREEN);
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
	patches.reset(); // changing sampling rate may require re-initialisation of patches
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
      uint32_t blocksize = 1L << value;
      if(settings.audio_blocksize != blocksize && blocksize <= AUDIO_MAX_BLOCK_SIZE){
	settings.audio_blocksize = blocksize;
	codec.stop();
	codec.init(settings);
	patches.reset(); // changing blocksize may require re-initialisation of patches
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
      case 3:
	midi.sendFirmwareVersion();
	break;
      case 4:
	midi.sendDeviceId();
	break;
      case 5:
	midi.sendSelfTest();
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
	patches.setActiveSlot(GREEN);
      }
      break;
    }
  }

  void handleSystemCommon(uint8_t){
  }

  FirmwareLoader loader;

  void handleSysEx(uint8_t* data, uint16_t size){
    if(size < 3 || 
       data[0] != MIDI_SYSEX_MANUFACTURER || 
       data[1] != MIDI_SYSEX_DEVICE)
      return;
    switch(data[2]){
    case SYSEX_DFU_COMMAND:
      jump_to_bootloader();
      break;
    case SYSEX_FIRMWARE_UPLOAD:
      int32_t ret = loader.handleFirmwareUpload(data, size);
      if(ret < 0){
	// firmware upload error
	// midi.sendCc(DEVICE_STATUS, -ret);
	setLed(RED);
      }else if(ret > 0){
	// firmware upload complete
	// midi.sendCc(DEVICE_STATUS, 0x7f);
	setLed(NONE);
	program.load(loader.getData(), loader.getSize());
	codec.start();
	// while(isProgramRunning()); // wait for program to exit
      }else{
	// midi.sendCc(DEVICE_STATUS, 0);
	toggleLed();
      }
      break;
    }
  }
};

#endif /* _MidiHandler_H_ */
