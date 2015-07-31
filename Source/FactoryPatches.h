#ifndef __FactoryPatches_h__
#define __FactoryPatches_h__

#include "PatchDefinition.hpp"
#include "StompBox.h"

typedef Patch* (*PatchCreator)(); // function pointer to create Patch
class FactoryPatchDefinition : public PatchDefinition {
public:
  FactoryPatchDefinition();
  FactoryPatchDefinition(char* name, uint8_t inputs, uint8_t outputs, PatchCreator c);
  void setup(char* name, uint8_t inputs, uint8_t outputs, PatchCreator c);
  void run();
  static void init();
private:
  Patch* create() {
    return (*creator)();
  }
  PatchCreator creator;
};

#endif // __FactoryPatches_h__
