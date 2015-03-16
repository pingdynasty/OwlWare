#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>

class ProgramManager {
private:
  /* bool running = false; */
  uint32_t* programAddress;
  uint32_t programLength;
  uint32_t* programStackPointer;
public:
  ProgramManager();
  /* bool isRunning(){ */
  /*   return running; */
  /* } */
  void load(void* address, uint32_t length);
  bool verify();
  void startProgram();
  void runProgram();
  void startManager();
  void runManager();
  void exit();
  /* exit and restart program */
  void reset();

  void audioReady();
  void programReady();
  void programStatus(int);

  bool saveProgram(uint8_t sector);
  bool loadProgram(uint8_t sector);
};

extern ProgramManager program;

#endif // __ProgramManager_H__
