#ifndef __MIDI_CONTROLLER_H
#define __MIDI_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

class MidiController;
extern MidiController midi;

class MidiController {
private:
  uint8_t channel;

public:
  void init(uint8_t channel);
  void sendCc(uint8_t cc, uint8_t value);
  void sendSysEx(uint8_t* data, uint16_t size);
};

#endif /* __MIDI_CONTROLLER_H */
