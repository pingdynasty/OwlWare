#ifndef __PatchProcessor_h__
#define __PatchProcessor_h__

#include <stdint.h>
#include "StompBox.h"
#include "device.h"

class PatchProcessor {
public:  
  PatchProcessor(uint8_t index);
  ~PatchProcessor();
  /* void setPatch(Patch* p){ */
  /*   patch = p; */
  /* } */
  /* void registerParameter(PatchParameterId pid, const std::string& name, const std::string& description = ""); */
  float getParameterValue(PatchParameterId pid);
  int getBlockSize();
  double getSampleRate();
  AudioBuffer* createMemoryBuffer(int channels, int samples);
  void setParameters(uint16_t *parameters);
  void process(AudioBuffer& buffer);
  Patch* patch;
  uint8_t index;
private:
  uint8_t bufferCount;
  uint16_t parameters[NOF_ADC_VALUES];
  AudioBuffer* buffers[MAX_BUFFERS_PER_PATCH];
};

#endif // __PatchProcessor_h__
