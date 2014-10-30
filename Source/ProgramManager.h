#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>

class ProgramManager {
private:
  int programRuns = 0;
  volatile bool running = false;
  volatile bool doRunProgram = false;
  volatile void* programAddress;
  volatile uint32_t programLength;

public:
  bool isRunning(){
    return running;
  }

  void exit();

  void load(void* address, uint32_t length){
    programAddress = address;
    programLength = length;
    doRunProgram = true;
  }

  void run();
};

extern ProgramManager program;

#endif // __ProgramManager_H__
