#ifndef _MidiHandler_H_
#define _MidiHandler_H_

#include <string.h>
#include "device.h"
#include "OpenWareMidiControl.h"
#include "MidiReader.hpp"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "FirmwareLoader.hpp"
#include "ProgramManager.h"
#include "SharedMemory.h"

uint16_t midi_values[NOF_ADC_VALUES];

#define MAX_FACTORY_PROGRAM 32

class MidiHandler : public MidiReader {
private:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
public:
  MidiHandler() : MidiReader(buffer, sizeof(buffer)) {
    memset(midi_values, 0, sizeof(midi_values));
  }

  // void handleProgramChange(uint8_t status, uint8_t program){
  //   if(program < MAX_FACTORY_PROGRAM)
  //     loadFactoryPatch(program);
  //   else
  //     // time to erase 128kB flash sector, typ 875ms
  //     // Program/erase parallelism
  //     // (PSIZE) = x 32 : 1-2s
  //     loadProgram(program);
  // }

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
	// patches.setParameterValues(midi_values);
      }else{
	// patches.setParameterValues(getAnalogValues());
      }
      break;
    case PATCH_BUTTON:
      if(value == 127)
	// patches.toggleActiveSlot();
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
      settings.patch_mode = value >> 4;
      getSharedMemory()->parameters[PATCH_MODE_PARAMETER_ID] = settings.patch_mode;
      // patches.setActiveSlot(patches.getActiveSlot());
      break;
    case PATCH_SLOT_GREEN:
      settings.patch_green = value;
      getSharedMemory()->parameters[GREEN_PATCH_PARAMETER_ID] = value;
      // patches.setPatch(GREEN, value);
      break;
    case PATCH_SLOT_RED:
      settings.patch_red = value;      
      getSharedMemory()->parameters[RED_PATCH_PARAMETER_ID] = value;
      // patches.setPatch(RED, value);
      break;
    case ACTIVE_SLOT:
      getSharedMemory()->parameters[PATCH_MODE_PARAMETER_ID] |= (value == 127);
      // patches.setActiveSlot(value == 127 ? RED : GREEN);
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
	codec.init(settings);
	program.reset(); // changing sampling rate may require re-initialisation of patches
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
	codec.init(settings);
      }
      break;
    }
    case CODEC_MASTER: {
      bool master = value > 63;
      if(master != settings.audio_codec_master){
	settings.audio_codec_master = master;
	codec.init(settings);
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
	codec.init(settings);
      }
      break;
    }
    case SAMPLING_SIZE: {
      uint32_t blocksize = 1L << value;
      if(settings.audio_blocksize != blocksize && blocksize <= AUDIO_MAX_BLOCK_SIZE){
	settings.audio_blocksize = blocksize;
	codec.init(settings);
	program.reset(); // changing blocksize may require re-initialisation of patches
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
	program.reset();
	// patches.setActiveSlot(GREEN);
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
      // codec.stop();
      int32_t ret = loader.handleFirmwareUpload(data, size);
      if(ret < 0){
	// firmware upload error
	// midi.sendCc(DEVICE_STATUS, -ret);
	setLed(RED);
	// program.reset();
	// codec.start(); // doesn't synchronise the codec well
      }else if(ret > 0){
	// firmware upload complete
	// midi.sendCc(DEVICE_STATUS, 0x7f);
	setLed(NONE);
	program.load(loader.getData(), loader.getSize());
	if(program.verify())
	  program.startProgram();
	loader.clear();
      }else{
	// midi.sendCc(DEVICE_STATUS, 0);
	toggleLed();
	// if(program.isRunning())
	//   program.stop();
      }
      break;
    }
  }
};

#endif /* _MidiHandler_H_ */
