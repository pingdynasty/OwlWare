#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>

class ProgramManager {
private:
  bool running = false;
  uint8_t* programAddress;
  uint32_t programLength;
  uint32_t msp; // Main stack pointer
public:
  bool isRunning(){
    return running;
  }
  void load(void* address, uint32_t length);
  bool verify();
  void start();
  /* void stop(); */
  void runPatch();
  void startManager();
  void runManager();
  void exit();
  /* exit and restart program */
  void reset();

};

extern ProgramManager program;

#endif // __ProgramManager_H__
