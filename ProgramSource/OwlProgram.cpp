#include <string.h>
#include <inttypes.h>
#include "SharedMemory.h"
#include "stm32f4xx.h"
#ifdef DEBUG_MEM
#include <malloc.h>
#endif /* DEBUG_MEM */

extern "C" {
  void program_setup(uint8_t pid);
  void program_processBlock();
  void program_run(void);
}

void program_run(void){
  for(;;){
    getSharedMemory()->programReady();
    // if(getSharedMemory()->status == AUDIO_READY_STATUS){
      program_processBlock();
  }
}

// void program_run(void){
//   if(getSharedMemory()->checksum != sizeof(SharedMemory)){
//     // problem!
//     // getSharedMemory()->status = AUDIO_ERROR_STATUS;
//     getSharedMemory()->error = CHECKSUM_ERROR_STATUS;
//     getSharedMemory()->programStatus(AUDIO_ERROR_STATUS);
//     // getSharedMemory()->exitProgram();
//     // return -1;
//   }

//   program_setup();
// #ifdef DEBUG_MEM
//   struct mallinfo minfo = mallinfo(); // never returns when -O1 or -O2 ?
//   /* ^ may cause OwlWare.sysex to trip to:
//    USART6_IRQHandler () at ./Source/startup.s:142
//    142	  b  Infinite_Loop */
//   // getSharedMemory()->heap_bytes_used = minfo.uordblks;
//   getSharedMemory()->heap_bytes_used = minfo.arena;
// #endif /* DEBUG_MEM */

//   for(;;){
//     getSharedMemory()->programReady();
//     // if(getSharedMemory()->status == AUDIO_READY_STATUS){
//       program_processBlock();
//   }
// }

#include "SharedMemory.h"
#include "SampleBuffer.hpp"
#include "PatchProcessor.h"
#include "basicmaths.h"
#include "solopatch.h"
#include "owlcontrol.h"

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
