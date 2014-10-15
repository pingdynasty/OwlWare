#ifndef __PatchController_h__
#define __PatchController_h__

#include "PatchProcessor.h"
#include "owlcontrol.h"

class PatchController;
extern PatchController patches;

class PatchController {
public:  
  PatchController();
  ~PatchController();
  void init();
  void reset();
  void process(AudioBuffer& buffer);
  void setPatch(LedPin slot, uint8_t index);
  LedPin getActiveSlot();
  void toggleActiveSlot();
  void setActiveSlot(LedPin slot);
  void setParameterValues(uint16_t* values);
  void initialisePatch(LedPin slot);
  PatchProcessor* getInitialisingPatchProcessor();
  PatchProcessor* getActivePatchProcessor();
  PatchProcessor* getCurrentPatchProcessor(){
    /* deprecated: the FAUST OWL target depends on this method */
    return getActivePatchProcessor();
  }
private:
  void processParallel(AudioBuffer& buffer);
  PatchProcessor* initialisingProcessor;
  PatchProcessor green;
  PatchProcessor red;
  LedPin activeSlot = NONE;
  uint8_t mode;
  uint16_t* parameterValues;
};

#endif // __PatchController_h__
