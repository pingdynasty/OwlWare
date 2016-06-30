#include "PatchRegistry.h"
#include "FlashStorage.h"
#include "ProgramManager.h"
#include "ProgramHeader.h"
#include "DynamicPatchDefinition.hpp"

// #define REGISTER_PATCH(T, STR, UNUSED, UNUSED2) registerPatch(STR, Register<T>::construct)

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry() : nofPatches(0) {}

FlashStorage storage;

bool isPresetBlock(StorageBlock block){
  if(block.getDataSize() > sizeof(ProgramHeader)){
    ProgramHeader* header = (ProgramHeader*)block.getData();
    uint32_t size = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
    return header->magic == 0xDADAC0DE && size <= 80*1024;
  }
  return false;
}

void PatchRegistry::init() {
  nofPatches = 0;
  storage.init();

  static DynamicPatchDefinition flashdefs[STORAGE_MAX_BLOCKS];

  for(int i=0; i<STORAGE_MAX_BLOCKS; ++i){
    StorageBlock block = storage.getBlock(0);
    // if(block.getDataSize() > sizeof(ProgramHeader)){
    if(isPresetBlock(block)){
      // DynamicPatchDefinition def(block.getData(), block.getDataSize());
      // ProgramHeader* header = (ProgramHeader*)block.getData();
      // uint32_t size = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
      DynamicPatchDefinition* def = &flashdefs[i];
      if(def->load(block.getData(), block.getDataSize()) && def->verify())
	registerPatch(def);
    }
  }

  for(int i=0; i<MAX_USER_PATCHES; ++i){
    PatchDefinition* def = program.getPatchDefinitionFromFlash(i);
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
