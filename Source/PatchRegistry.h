#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__

#include <string>
#include "device.h"

class PatchRegistry;
extern PatchRegistry registry;

class PatchRegistry {
public:
  PatchRegistry();
  const char* getName(unsigned int index);
  unsigned int getNumberOfPatches();
  void registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels);
private:
  const char* names[MAX_NUMBER_OF_PATCHES];
  int nofPatches;
};

#endif // __PatchRegistry_h__
