#include "PatchProcessor.h"
#include "PatchRegistry.h"
#include "device.h"
#include <string.h>
#include "ProgramVector.h"
#include "basicmaths.h"
#include "owlcontrol.h"

PatchProcessor::PatchProcessor() 
  : patch(NULL){}
 // todo: initialise suitable SampleBuffer: 16/24 bit, max size

PatchProcessor::~PatchProcessor(){
  clear();
}

void PatchProcessor::run(){
  if(patch == NULL)
    return;
  ProgramVector* vector = getProgramVector();
  ASSERT(vector->audio_input != NULL, "Audio input must not be NULL");
  ASSERT(vector->audio_output != NULL, "Audio output must not be NULL");
  ASSERT(vector->audio_blocksize != 0, "Audio blocksize must not be 0");
  ASSERT(vector->audio_samplingrate != 0, "Audio samplingrate must not be 0");
  for(;;){
    vector->programReady();
    buffer.split(vector->audio_input, vector->audio_blocksize);
    setParameterValues(vector->parameters);
    patch->processAudio(buffer);
    buffer.comb(vector->audio_output);
  }
}

void PatchProcessor::clear(){
  delete patch;
  patch = NULL;
}

void PatchProcessor::setPatch(Patch* p){
  patch = p;
}

float PatchProcessor::getParameterValue(PatchParameterId pid){
  if(pid < NOF_ADC_VALUES)
    return parameterValues[pid]/4096.0f;
  else
    return 0.0f;
}

#define SMOOTH_HYSTERESIS
#define SMOOTH_FACTOR 3
__attribute__ ((section (".coderam")))
void PatchProcessor::setParameterValues(uint16_t *params){
  /* Implements an exponential moving average (leaky integrator) to smooth ADC values
   * y(n) = (1-alpha)*y(n-1) + alpha*y(n)
   * with alpha=0.5, fs=48k, bs=128, then w0 ~= 18hz
   */
  for(int i=0; i<NOF_ADC_VALUES; ++i)
#ifdef SMOOTH_HYSTERESIS
    if(abs(params[i]-parameterValues[i]) > 7)
#endif
      // 16 = half a midi step (4096/128=32)
#ifdef OWLMODULAR
      parameterValues[i] = (parameterValues[i]*SMOOTH_FACTOR + 0x1000 - params[i])/(SMOOTH_FACTOR+1);
#else /* OWLMODULAR */
      parameterValues[i] = (parameterValues[i]*SMOOTH_FACTOR + params[i])/(SMOOTH_FACTOR+1);
#endif /* OWLMODULAR */
  // for(int i=NOF_ADC_VALUES; i<NOF_PARAMETERS; ++i)
  //   // todo!
}
