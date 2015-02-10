#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>

class ProgramManager {
private:
  volatile bool running = false;
  volatile bool doRunProgram = false;
  volatile bool doCopyProgram = false;
  volatile bool doRestartProgram = false;
  volatile uint8_t* programAddress;
  volatile uint32_t programLength;
  uint32_t msp; // Main stack pointer
public:
  bool isRunning(){
    return running;
  }
  void load(void* address, uint32_t length);
  bool verify();
  void start();
  void stop();
  void run();
  void exit();
  /* exit and restart program */
  void reset();

};

extern ProgramManager program;

#endif // __ProgramManager_H__
