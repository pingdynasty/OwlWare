#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__

#include <string>
#include "device.h"
#include "PatchDefinition.hpp"
#include "StorageBlock.h"

class PatchRegistry;
extern PatchRegistry registry;

class PatchRegistry {
public:
  PatchRegistry();
  void init();
  const char* getName(unsigned int index);
  PatchDefinition* getPatchDefinition(unsigned int index);
  unsigned int getNumberOfPatches();
  void registerPatch(uint8_t pid, StorageBlock block);
  void setDynamicPatchDefinition(PatchDefinition* def){
    dynamicPatchDefinition = def;
  }
private:
  bool isPresetBlock(StorageBlock block);
  StorageBlock blocks[MAX_NUMBER_OF_PATCHES];
  /* PatchDefinition* defs[MAX_NUMBER_OF_PATCHES]; */
  unsigned int nofPatches;
  PatchDefinition* dynamicPatchDefinition;
};

#endif // __PatchRegistry_h__
