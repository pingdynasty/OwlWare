#ifndef _MIDIREADER_H_
#define _MIDIREADER_H_

#include <inttypes.h>
#include "MidiStatus.h"
#include "MidiHandler.h"

enum MidiReaderStatus {
  READY_STATUS, INCOMPLETE_STATUS, ERROR_STATUS
};

class MidiReader : public MidiHandler {
protected:
  uint8_t buffer[MIDI_MAX_MESSAGE_SIZE];
  static const int size = MIDI_MAX_MESSAGE_SIZE;
  MidiReaderStatus status; // state, should be status: READY / INCOMPLETE / ERROR
  unsigned char runningStatus;
  int pos;
private:
public:
  MidiReader() : 
    status(READY_STATUS), 
    runningStatus(0), 
    pos(0) {
  }

  ~MidiReader(){
  }

  void clear(){
    runningStatus = buffer[0];
    pos = 0;
    status = READY_STATUS;
  }

  unsigned char* getMessage(int& length){
    length = pos;
    return buffer;
  }

  MidiReaderStatus read(unsigned char data);
};


#endif /* _MIDIREADER_H_ */
