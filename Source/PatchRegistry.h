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
  void reset();
  const char* getName(unsigned int index);
  PatchDefinition* getPatchDefinition(unsigned int index){
    if(index < nofPatches)
      return defs[index];
    return NULL;
  }
  unsigned int getNumberOfPatches();
  /* void registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels); */
  void registerPatch(PatchDefinition* def);
private:
  PatchDefinition* defs[MAX_NUMBER_OF_PATCHES];
  /* const char* names[MAX_NUMBER_OF_PATCHES]; */
  unsigned int nofPatches;
};

#endif // __PatchRegistry_h__
