#ifndef _MidiHandler_HPP_
#define _MidiHandler_HPP_

#include <stdint.h>
#include <string.h>
#include "device.h"
#include "FirmwareLoader.hpp"

class MidiHandler {
private:
  int16_t midi_values[NOF_PARAMETERS];
  FirmwareLoader loader;
  // state variables to track monophonic note
  // uint8_t note;
  // uint16_t pitchbend;
  // uint16_t pitch;
  // float amplitude;
  // static constexpr float PBRANGE = 2/128.0f; // 2 semitones
public:
  MidiHandler(){
    // note = 0;
    // pitchbend = 8192;
    memset(midi_values, 0, NOF_PARAMETERS*sizeof(uint16_t));
  }

  void handleProgramChange(uint8_t status, uint8_t pc);
  void handleControlChange(uint8_t status, uint8_t cc, uint8_t value);
  void handleNoteOff(uint8_t status, uint8_t note, uint8_t velocity);
  void handleNoteOn(uint8_t status, uint8_t note, uint8_t velocity);
  void handlePitchBend(uint8_t status, uint16_t value);
  void handleSysEx(uint8_t* data, uint16_t size);

  void handleSystemCommon(uint8_t cmd){}
  void handleChannelPressure(uint8_t status, uint8_t value){}
  void handlePolyKeyPressure(uint8_t status, uint8_t note, uint8_t value){}
  void handleParameterChange(uint8_t pid, uint16_t value){}

private:
  void updateCodecSettings();
  void handleConfigurationCommand(uint8_t* data, uint16_t size);
  void handleFirmwareUploadCommand(uint8_t* data, uint16_t size);
  void handleFirmwareRunCommand(uint8_t* data, uint16_t size);
  void handleFirmwareFlashCommand(uint8_t* data, uint16_t size);
  void handleFirmwareStoreCommand(uint8_t* data, uint16_t size);

};

#endif /* _MidiHandler_HPP_ */
