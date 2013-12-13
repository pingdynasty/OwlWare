#include "PatchRegistry.h"

#define registerParameter(...)

#include "basicmaths.h"
#include "OwlPatches/includes.h"

#define REGISTER_PATCH(T, STR) registerPatch(STR, Register<T>::construct)

PatchRegistry::PatchRegistry() : nofPatches(0) {
#include "OwlPatches/patches.cpp"
}

const char* PatchRegistry::getName(unsigned int index){
  if(index < getNumberOfPatches())
    return names[index];
  return "";  
}

unsigned int PatchRegistry::getNumberOfPatches(){
  return nofPatches;
}

Patch* PatchRegistry::create(unsigned int index) {
  if(index < getNumberOfPatches())
    return (*creators[index])();
  return NULL;
}

void PatchRegistry::registerPatch(const char* name, PatchCreator creator){
  if(nofPatches < MAX_NUMBER_OF_PATCHES){
    names[nofPatches] = name;
    creators[nofPatches] = creator;
    nofPatches++;
  }
}
