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
#include "Owl.h"

class MidiHandler : public MidiReader {
private:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
  int16_t midi_values[NOF_PARAMETERS];
  FirmwareLoader loader;
  // state variables to track monophonic note
  // uint8_t note;
  // uint16_t pitchbend;
  // uint16_t pitch;
  // float amplitude;
  // static constexpr float PBRANGE = 2/128.0f; // 2 semitones
public:
  MidiHandler() : MidiReader(buffer, sizeof(buffer)) {
    // note = 0;
    // pitchbend = 8192;
    memset(midi_values, 0, NOF_PARAMETERS*sizeof(uint16_t));
  }

  void handlePitchBend(uint8_t status, uint16_t value){
    // pitchbend = value;
    // setParameter(PARAMETER_G, value>>2);
    setParameter(PARAMETER_G, ((int16_t)value - 8192)>>1);
  }

  void handleNoteOn(uint8_t status, uint8_t note, uint8_t velocity){
    // note = n;
    // pitch = note*4095/127 + (pitchbend-8192)*(4095/63)/16383;
    // amplitude = velocity/127.0f;
    // setButton(MIDI_GATE_BUTTON, true);
    // setParameter(PARAMETER_MIDI_AMPLITUDE, amplitude);
    // setParameter(PARAMETER_MIDI_PITCH, pitch);
    setButton(MIDI_NOTE_BUTTON+note, velocity<<5);
  }

  void handleNoteOff(uint8_t status, uint8_t note, uint8_t velocity){
    // amplitude = 0;
    // setButton(MIDI_GATE_BUTTON, false);
    setButton(MIDI_NOTE_BUTTON+note, 0);
  }

  void handleProgramChange(uint8_t status, uint8_t pid){
    if(pid == 0 && loader.isReady()){
      program.loadDynamicProgram(loader.getData(), loader.getSize());
      loader.clear();
      program.startProgram(true);
    }else{
      program.loadProgram(pid);
      program.resetProgram(true);
    }
  }

  void handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
    switch(cc){
#ifdef OWLMODULAR
    case PATCH_PARAMETER_A:
      midi_values[PARAMETER_A] = (127-value)<<5; // invert and scale from 7bit to 12bit value
      break;
    case PATCH_PARAMETER_B:
      midi_values[PARAMETER_B] = (127-value)<<5;
      break;
    case PATCH_PARAMETER_C:
      midi_values[PARAMETER_C] = (127-value)<<5;
      break;
    case PATCH_PARAMETER_D:
      midi_values[PARAMETER_D] = (127-value)<<5;
      break;
    case PATCH_PARAMETER_E:
      midi_values[PARAMETER_E] = value<<5;
      break;
#else /* OWLMODULAR */
    case PATCH_PARAMETER_A:
      midi_values[PARAMETER_A] = value<<5; // scale from 7bit to 12bit value
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
#endif /* OWLMODULAR */
    case PATCH_CONTROL:
      if(value == 127){
	memcpy(midi_values, getAnalogValues(), NOF_PARAMETERS*sizeof(uint16_t));
	setParameterValues(midi_values, NOF_PARAMETERS);
      }else{
	setParameterValues(getAnalogValues(), NOF_PARAMETERS);
      }
      break;
    case PATCH_BUTTON:
      if(value == 127){
	togglePushButton();
	midi.sendCc(LED, getLed() == GREEN ? 42 : 84);
      }
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
    case LEFT_INPUT_MUTE:
      codec.setInputMuteLeft(value == 127);
      break;
    case RIGHT_INPUT_MUTE:
      codec.setInputMuteRight(value == 127);
      break;
    case REQUEST_SETTINGS:
      switch(value){
      case 0:
	midi.sendDeviceInfo();
	break;
      case SYSEX_PRESET_NAME_COMMAND:
	// program.sendMidiData(value, true);
	midi.sendPatchNames();
	break;
      case SYSEX_PARAMETER_NAME_COMMAND:
	midi.sendPatchParameterNames();
	break;
      case SYSEX_FIRMWARE_VERSION:
	midi.sendFirmwareVersion();
	break;
      case SYSEX_DEVICE_ID:
	midi.sendDeviceId();
	break;
      case SYSEX_DEVICE_STATS:
	midi.sendDeviceStats();
	break;
      case SYSEX_PROGRAM_MESSAGE:
	midi.sendProgramMessage();
	break;
      case SYSEX_PROGRAM_STATS:
	midi.sendProgramStats();
	break;
      case PATCH_BUTTON:
	midi.sendCc(PATCH_BUTTON, isPushButtonPressed() ? 127 : 0);
	break;
      case LED:
	switch(getLed()){
	case NONE:
	  midi.sendCc(LED, 0);
	  break;
	case GREEN:
	  midi.sendCc(LED, 42);
	  break;
	case RED:
	  midi.sendCc(LED, 84);
	  break;
	}
	break;
      case PATCH_PARAMETER_A:
      case PATCH_PARAMETER_B:
      case PATCH_PARAMETER_C:
      case PATCH_PARAMETER_D:
      case PATCH_PARAMETER_E:
	midi.sendPatchParameterValues();
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
    case FACTORY_RESET:
      if(value == 127){
	settings.reset();
	program.eraseProgramFromFlash(-1);
	updateCodecSettings();
      }
      break;
    case MIDI_CC_MODULATION:
      setParameter(PARAMETER_F, value<<5);
      break;
    // case MIDI_CC_BREATH:
    //   setParameter(PARAMETER_MIDI_BREATH, value<<5);
    //   break;
    // case MIDI_CC_VOLUME:
    //   setParameter(PARAMETER_MIDI_VOLUME, value<<5);
    //   break;
    // case MIDI_CC_BALANCE:
    //   setParameter(PARAMETER_MIDI_BALANCE, value<<5);
    //   break;
    // case MIDI_CC_PAN:
    //   setParameter(PARAMETER_MIDI_PAN, value<<5);
    //   break;
    // case MIDI_CC_EXPRESSION:
    //   setParameter(PARAMETER_MIDI_EXPRESSION, value<<5);
    //   break;
    case MIDI_CC_EFFECT_CTRL_1:
      setParameter(PARAMETER_G, value<<5);
      break;
    case MIDI_CC_EFFECT_CTRL_2:
      setParameter(PARAMETER_H, value<<5);
      break;
    default:
      if(cc >= PATCH_PARAMETER_AA && cc <= PATCH_PARAMETER_BH)
	setParameter(PARAMETER_AA+(cc-PATCH_PARAMETER_AA), value<<5);
      break;
    }
  }

  void updateCodecSettings(){
    codec.softMute(true);
    codec.stop();
    codec.init(settings);
    codec.start();
    program.resetProgram(true);
  }

  void handleConfigurationCommand(uint8_t* data, uint16_t size){
    if(size < 4)
      return;
    char* p = (char*)data;
    int32_t value = strtol(p+2, NULL, 16);
    if(strncmp(SYSEX_CONFIGURATION_AUDIO_RATE, p, 2) == 0){
      settings.audio_samplingrate = value;
    }else if(strncmp(SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE, p, 2) == 0){
      settings.audio_blocksize = value;
    }else if(strncmp(SYSEX_CONFIGURATION_AUDIO_DATAFORMAT, p, 2) == 0){
      settings.audio_dataformat = value;
    }else if(strncmp(SYSEX_CONFIGURATION_CODEC_PROTOCOL, p, 2) == 0){
      settings.audio_codec_protocol = (I2SProtocol)value;
    }else if(strncmp(SYSEX_CONFIGURATION_CODEC_MASTER, p, 2) == 0){
      settings.audio_codec_master = value;
    }else if(strncmp(SYSEX_CONFIGURATION_CODEC_SWAP, p, 2) == 0){
      settings.audio_codec_swaplr = value;
    }else if(strncmp(SYSEX_CONFIGURATION_CODEC_BYPASS, p, 2) == 0){
      settings.audio_codec_bypass = value;
    }else if(strncmp(SYSEX_CONFIGURATION_CODEC_HALFSPEED, p, 2) == 0){
      settings.audio_codec_halfspeed = value;
      // settings.audio_codec_halfspeed = (p[2] == '1' ? true : false);
    }else if(strncmp(SYSEX_CONFIGURATION_PC_BUTTON, p, 2) == 0){
      settings.program_change_button = value;
    }else if(strncmp(SYSEX_CONFIGURATION_INPUT_OFFSET, p, 2) == 0){
      settings.input_offset = value;
    }else if(strncmp(SYSEX_CONFIGURATION_INPUT_SCALAR, p, 2) == 0){
      settings.input_scalar = value;
    }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_OFFSET, p, 2) == 0){
      settings.output_offset = value;
    }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_SCALAR, p, 2) == 0){
      settings.output_scalar = value;
    }
    updateCodecSettings();
  }

  void handleFirmwareUploadCommand(uint8_t* data, uint16_t size){
    int32_t ret = loader.handleFirmwareUpload(data, size);
    if(ret > 0){
      // firmware upload complete: wait for run or store
      setLed(NONE);
    }else if(ret == 0){
      toggleLed();
    }// else error
  }

  void handleFirmwareRunCommand(uint8_t* data, uint16_t size){
    if(loader.isReady()){
      program.loadDynamicProgram(loader.getData(), loader.getSize());
      loader.clear();
      program.startProgram(true);
    }else{
      setErrorMessage(PROGRAM_ERROR, "No program to run");
    }      
  }

  void handleFirmwareFlashCommand(uint8_t* data, uint16_t size){
    if(loader.isReady() && size == 5){
      uint32_t checksum = loader.decodeInt(data);
      if(checksum == loader.getChecksum()){
	program.saveProgramToFlash(-1, loader.getData(), loader.getSize());
	loader.clear();
      }else{
	setErrorMessage(PROGRAM_ERROR, "Invalid FLASH checksum");
      }
    }else{
      setErrorMessage(PROGRAM_ERROR, "Invalid FLASH command");
    }
  }

  void handleFirmwareStoreCommand(uint8_t* data, uint16_t size){
    if(loader.isReady() && size == 5){
      uint32_t slot = loader.decodeInt(data);
      if(slot >= 0 && slot < MAX_USER_PATCHES){
	program.saveProgramToFlash(slot, loader.getData(), loader.getSize());
	loader.clear();
      }else{
	setErrorMessage(PROGRAM_ERROR, "Invalid program slot");
      }
    }else{
      setErrorMessage(PROGRAM_ERROR, "No program to store");
    }
  }

  void handleSysEx(uint8_t* data, uint16_t size){
    if(size < 3 || 
       data[0] != MIDI_SYSEX_MANUFACTURER || 
       data[1] != MIDI_SYSEX_DEVICE)
      return;
    switch(data[2]){
    case SYSEX_CONFIGURATION_COMMAND:
      handleConfigurationCommand(data+3, size-3);
      break;
    case SYSEX_DFU_COMMAND:
      jump_to_bootloader();
      break;
    case SYSEX_FIRMWARE_UPLOAD:
      handleFirmwareUploadCommand(data, size);
      break;
    case SYSEX_FIRMWARE_RUN:
      handleFirmwareRunCommand(data+3, size-3);
      break;
    case SYSEX_FIRMWARE_STORE:
      handleFirmwareStoreCommand(data+3, size-3);
      break;
    case SYSEX_FIRMWARE_FLASH:
      handleFirmwareFlashCommand(data+3, size-3);
      break;
    }
  }
};

#endif /* _MidiHandler_H_ */
