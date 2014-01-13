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
  void processParallel(AudioBuffer& buffer);
  PatchProcessor* green = NULL;
  PatchProcessor* red = NULL;
  uint8_t activeSlot = 0;
  uint8_t mode;
};

#endif // __PatchController_h__
