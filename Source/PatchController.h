#ifndef __PatchController_h__
#define __PatchController_h__

#include "PatchProcessor.h"

class PatchController;
extern PatchController patches;

class PatchController {
public:  
  PatchController();
  ~PatchController();
  void init();
  void process(AudioBuffer& buffer);
  void setPatch(uint8_t slot, uint8_t index);
  uint8_t getActiveSlot();
  void toggleActiveSlot();
  void setActiveSlot(uint8_t slot);
  PatchProcessor* getCurrentPatchProcessor();
private:
  void processParallel(AudioBuffer& buffer);
  PatchProcessor green;
  PatchProcessor red;
  uint8_t activeSlot = 0;
  uint8_t mode;
};

#endif // __PatchController_h__
