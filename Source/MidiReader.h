#ifndef _MIDIREADER_H_
#define _MIDIREADER_H_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"

class MidiReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
  static const int size = MIDI_MAX_MESSAGE_SIZE;
  int pos;
public:
  MidiReader() : pos(0) {}  
  void readMidiFrame(uint8_t* frame);
};


#endif /* _MIDIREADER_H_ */
