#ifndef __PatchController_h__
#define __PatchController_h__

#include "PatchProcessor.h"

class PatchController;
extern PatchController patches;

class PatchController {
public:  
  PatchController();
  ~PatchController();
  void toggleActiveSlot();
  void process(AudioBuffer& buffer);
  uint8_t getActiveSlot();
  void setActiveSlot(uint8_t slot);
  PatchProcessor* getCurrentPatchProcessor();
private:
  PatchProcessor* green = NULL;
  PatchProcessor* red = NULL;
  Patch* activePatch = NULL;
  Patch* nextPatch = NULL;
  uint8_t activeSlot = 0;  
};

#endif // __PatchController_h__
