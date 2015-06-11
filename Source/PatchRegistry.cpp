#include "PatchRegistry.h"
#include "FactoryPatches.h"
#include "ProgramManager.h"

// #define REGISTER_PATCH(T, STR, UNUSED, UNUSED2) registerPatch(STR, Register<T>::construct)

PatchRegistry::PatchRegistry() : nofPatches(0) {
  FactoryPatchDefinition::init();
  PatchDefinition* def;
  for(int i=0; i<4; ++i){
    def = program.getPatchDefinitionFromFlash(i);
    if(def != NULL)
      registerPatch(def);
  }
}

void PatchRegistry::reset(){
  nofPatches = 0;
}

const char* PatchRegistry::getName(unsigned int index){
  if(index == 0)
    return dynamicPatchDefinition == NULL ? "---" : dynamicPatchDefinition->getName();
  if(--index < nofPatches)
    return defs[index]->getName();
  return NULL;
}

unsigned int PatchRegistry::getNumberOfPatches(){
  return nofPatches; // + (dynamicPatchDefinition == NULL ? 0 : 1);
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
