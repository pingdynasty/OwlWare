#ifndef _MidiHandler_HPP_
#define _MidiHandler_HPP_

#include <string.h>
#include "device.h"
#include "owlcontrol.h"
#include "OpenWareMidiControl.h"
#include "MidiController.h"
#include "CodecController.h"
#include "ApplicationSettings.h"
#include "FirmwareLoader.hpp"
#include "ProgramManager.h"
#include "Owl.h"

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
  MidiHandler() : MidiReader(buffer, sizeof(buffer)) {
    // note = 0;
    // pitchbend = 8192;
    memset(midi_values, 0, NOF_PARAMETERS*sizeof(uint16_t));
  }

  void handleSystemCommon(uint8_t);
  void handleProgramChange(uint8_t, uint8_t);
  void handleChannelPressure(uint8_t, uint8_t);
  void handleControlChange(uint8_t, uint8_t, uint8_t);
  void handleNoteOff(uint8_t, uint8_t, uint8_t);
  void handleNoteOn(uint8_t, uint8_t, uint8_t);
  void handlePitchBend(uint8_t, uint16_t);
  void handlePolyKeyPressure(uint8_t, uint8_t, uint8_t);
  void handleSysEx(uint8_t* data, uint16_t size);
  void handleParameterChange(uint8_t pid, uint16_t value);
};

#endif /* _MidiHandler_HPP_ */
