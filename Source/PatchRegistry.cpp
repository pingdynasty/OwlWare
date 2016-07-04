#include "PatchRegistry.h"
#include "FlashStorage.h"
#include "ProgramManager.h"
#include "ProgramHeader.h"
#include "DynamicPatchDefinition.hpp"

// #define REGISTER_PATCH(T, STR, UNUSED, UNUSED2) registerPatch(STR, Register<T>::construct)

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry() : nofPatches(0) {}

bool PatchRegistry::isPresetBlock(StorageBlock block){
  if(block.verify() && block.getDataSize() > sizeof(ProgramHeader)){
    ProgramHeader* header = (ProgramHeader*)block.getData();
    uint32_t size = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
    return (header->magic&0xffffff00) == 0xDADAC000 && size <= 80*1024;
  }
  return false;
}

void PatchRegistry::init() {
  storage.init();
  for(int i=0; i<MAX_NUMBER_OF_PATCHES; ++i)
    blocks[i] = storage.getLastBlock();
  nofPatches = MAX_NUMBER_OF_PATCHES;
  for(int i=0; i<STORAGE_MAX_BLOCKS; ++i){
    StorageBlock block = storage.getBlock(i);
    // if(block.getDataSize() > sizeof(ProgramHeader)){
    if(isPresetBlock(block)){
      ProgramHeader* header = (ProgramHeader*)block.getData();
      int pc = header->magic&0x00ff;
      if(pc > 0 && pc < MAX_NUMBER_OF_PATCHES)
	blocks[pc] = block;
    }
  }
}

void PatchRegistry::registerPatch(uint8_t index, StorageBlock block){
  if(--index < MAX_NUMBER_OF_PATCHES && isPresetBlock(block))
    blocks[index] = block;
}

const char* PatchRegistry::getName(unsigned int index){
  PatchDefinition* def = getPatchDefinition(index);
  if(def == NULL)
    return emptyPatch.getName();
  return def->getName();
}

unsigned int PatchRegistry::getNumberOfPatches(){
  // +1 for the current / dynamic patch in slot 0
  return nofPatches+1;
}

PatchDefinition* PatchRegistry::getPatchDefinition(unsigned int index){
  PatchDefinition *def = NULL;
  static DynamicPatchDefinition flashPatch;
  if(index == 0)
    def = dynamicPatchDefinition;
  else if(--index <= nofPatches){
    // if(blocks[index].getDataSize() > 0){
    if(isPresetBlock(blocks[index])){
      flashPatch.load(blocks[index].getData(), blocks[index].getDataSize());
      if(flashPatch.verify())
	def = &flashPatch;
    }
  }
  if(def == &emptyPatch)
    def = NULL;
  return def;
}

// void PatchRegistry::registerPatch(PatchDefinition* def){
//   if(nofPatches < MAX_NUMBER_OF_PATCHES)
//     defs[nofPatches++] = def;
// }

// void PatchRegistry::registerPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
//   if(nofPatches < MAX_NUMBER_OF_PATCHES){
//     names[nofPatches] = name;
//     nofPatches++;
//   }
// }
