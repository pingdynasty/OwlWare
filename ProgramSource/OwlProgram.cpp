#include <string.h>
#include <inttypes.h>
#include "SharedMemory.h"
#include "stm32f4xx.h"
#ifdef DEBUG_MEM
#include <malloc.h>
#endif /* DEBUG_MEM */

extern "C" {
  void program_register();
  void program_setup(uint8_t pid);
  void program_processBlock();
  void program_run(void);
}

void program_run(void){
  for(;;){
    getSharedMemory()->programReady();
    program_processBlock();
  }
}

#include "SharedMemory.h"
#include "SampleBuffer.hpp"
#include "PatchProcessor.h"
#include "basicmaths.h"
#include "solopatch.h"
#include "owlcontrol.h"

typedef Patch* (*PatchCreator)(); // function pointer to create Patch

PatchCreator* creators[32];

PatchProcessor processor;
PatchProcessor* getInitialisingPatchProcessor(){
  return &processor;
}

#define REGISTER_PATCH(T, STR, IN, OUT) registerPatch(STR, IN, OUT, new T)

void registerPatch(const char* name, uint8_t inputs, uint8_t outputs, Patch* patch){
  if(getSharedMemory()->registerPatch != NULL)
    getSharedMemory()->registerPatch(name, inputs, outputs);
  processor.setPatch(patch);
}

void program_setup(uint8_t pid){
#include "solopatch.cpp"
}

SampleBuffer buffer;
void program_processBlock(){
  buffer.split(getSharedMemory()->audio_input, getSharedMemory()->audio_blocksize);
  processor.setParameterValues(getSharedMemory()->parameters);
  processor.patch->processAudio(buffer);
  buffer.comb(getSharedMemory()->audio_output);
}
