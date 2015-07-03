#include "PatchRegistry.h"
#include "FactoryPatches.h"
#include "ProgramManager.h"

// #define REGISTER_PATCH(T, STR, UNUSED, UNUSED2) registerPatch(STR, Register<T>::construct)

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry() : nofPatches(0) {}

void PatchRegistry::init() {
  nofPatches = 0;
  FactoryPatchDefinition::init();
  PatchDefinition* def;
  for(int i=0; i<MAX_USER_PATCHES; ++i){
    def = program.getPatchDefinitionFromFlash(i);
    if(def == NULL)
      registerPatch(&emptyPatch);
    else
      registerPatch(def);
  }
}

const char* PatchRegistry::getName(unsigned int index){
  if(index == 0)
    return dynamicPatchDefinition == NULL ? emptyPatch.getName() : dynamicPatchDefinition->getName();
  if(--index < nofPatches)
    return defs[index]->getName();
  return NULL;
}

unsigned int PatchRegistry::getNumberOfPatches(){
  // +1 for the current / dynamic patch in slot 0
  return nofPatches+1;
}

PatchDefinition* PatchRegistry::getPatchDefinition(unsigned int index){
  PatchDefinition* def;
  if(index == 0)
    def = dynamicPatchDefinition;
  else if(--index < nofPatches)
    def = defs[index];
  if(def == &emptyPatch)
    def = NULL;
  return def;
}

void PatchRegistry::registerPatch(PatchDefinition* def){
  if(nofPatches < MAX_NUMBER_OF_PATCHES)
    defs[nofPatches++] = def;
}

// void PatchRegistry::registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
//   if(nofPatches < MAX_NUMBER_OF_PATCHES){
//     names[nofPatches] = name;
//     nofPatches++;
//   }
// }
