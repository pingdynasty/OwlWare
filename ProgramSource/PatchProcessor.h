#ifndef __PatchProcessor_h__
#define __PatchProcessor_h__

#include <stdint.h>
#include "StompBox.h"
#include "SampleBuffer.hpp"
#include "device.h"

class PatchProcessor {
public:  
  PatchProcessor();
  ~PatchProcessor();
  void clear();
  void setPatch(Patch* patch);
  void run();
  float getParameterValue(PatchParameterId pid);
  void setParameterValues(uint16_t *parameters);
private:
  Patch* patch;
  SampleBuffer buffer;
  uint16_t parameterValues[NOF_ADC_VALUES];
};

#endif // __PatchProcessor_h__
