#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>
#include "PatchDefinition.hpp"
#include "ProgramVector.h"

class ProgramManager {
private:
  /* bool running = false; */
  // uint32_t* programAddress;
  // uint32_t programLength;
  // uint32_t* programStackBase;
  // uint32_t programStackSize;
  // char programName[16];
  void notifyProgram(uint32_t ulValue);
  void notifyProgramFromISR(uint32_t ulValue);
public:
  ProgramManager();
  /* bool isRunning(){ */
  /*   return running; */
  /* } */
  // void load(PatchDefinition* def);
  void loadProgram(uint8_t index);
  void loadStaticProgram(PatchDefinition* def);
  void loadDynamicProgram(void* address, uint32_t length);
  void startManager();
  void runManager();
  void startProgram(bool isr);
  void exitProgram(bool isr);
  /* exit and restart program */
  void resetProgram(bool isr);

  void audioReady();
  void programReady();
  void programStatus(int);

  uint32_t getProgramStackUsed();
  uint32_t getProgramStackAllocation();
  uint32_t getManagerStackUsed();
  uint32_t getManagerStackAllocation();
  uint32_t getFreeHeapSize();

  void eraseProgramFromFlash(uint8_t sector);
  void saveProgramToFlash(uint8_t sector, void* address, uint32_t length);
  PatchDefinition* getPatchDefinitionFromFlash(uint8_t sector);

  uint32_t getCyclesPerBlock();
  uint32_t getHeapMemoryUsed();
  uint8_t getProgramIndex();
};

extern ProgramManager program;

#endif // __ProgramManager_H__
