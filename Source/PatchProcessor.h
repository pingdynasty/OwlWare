#ifndef __PatchProcessor_h__
#define __PatchProcessor_h__

#include <string>
#include "StompBox.h"

#define MAX_BUFFERS_PER_PATCH 4

class PatchProcessor {
public:  
  PatchProcessor(Patch* patch);
  ~PatchProcessor();
  /* void setPatch(Patch* p){ */
  /*   patch = p; */
  /* } */
  /* void registerParameter(PatchParameterId pid, const std::string& name, const std::string& description = ""); */
  float getParameterValue(PatchParameterId pid);
  int getBlockSize();
  double getSampleRate();
  AudioBuffer* createMemoryBuffer(int channels, int samples);
private:
  Patch* patch;
  uint8_t bufferCount;
  AudioBuffer* buffers[MAX_BUFFERS_PER_PATCH];
};

#endif // __PatchProcessor_h__
