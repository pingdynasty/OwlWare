#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__

#include <string>
#include "device.h"
#include "PatchDefinition.hpp"

class PatchRegistry;
extern PatchRegistry registry;

class PatchRegistry {
public:
  PatchRegistry();
  void init();
  const char* getPatchName(unsigned int index);
  PatchDefinition* getPatchDefinition(unsigned int index);
  unsigned int getNumberOfPatches();
  void registerPatch(PatchDefinition* def);
  void setDynamicPatchDefinition(PatchDefinition* def){
    dynamicPatchDefinition = def;
  }
private:
  PatchDefinition* defs[MAX_NUMBER_OF_PATCHES];
  unsigned int nofPatches;
  PatchDefinition* dynamicPatchDefinition;
};

#endif // __PatchRegistry_h__
