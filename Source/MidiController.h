#ifndef __MIDI_CONTROLLER_H
#define __MIDI_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "OpenWareMidiControl.h"

class MidiController;
extern MidiController midi;

class MidiController {
private:
  uint8_t channel;

public:
  void init(uint8_t channel);
  void sendPc(uint8_t pc);
  void sendCc(uint8_t cc, uint8_t value);
  void sendSysEx(uint8_t* data, uint16_t size);
  void sendSettings();
  void sendConfigurationSetting(const char* name, uint32_t value);
  void sendPatchParameterNames();
  void sendPatchParameterName(PatchParameterId pid, const char* name);
  void sendPatchNames();
  void sendPatchName(uint8_t index);
  void sendDeviceInfo();
  void sendDeviceStats();
  void sendProgramStats();
  void sendFirmwareVersion();
  void sendDeviceId();
  void sendSelfTest();
  void sendProgramMessage();
};

#endif /* __MIDI_CONTROLLER_H */
