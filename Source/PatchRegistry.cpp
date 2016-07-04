#include "PatchRegistry.h"
#include "FlashStorage.h"
#include "ProgramManager.h"
#include "ProgramHeader.h"
#include "DynamicPatchDefinition.hpp"
#include "message.h"

// #define REGISTER_PATCH(T, STR, UNUSED, UNUSED2) registerPatch(STR, Register<T>::construct)

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry(){}

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
  for(int i=0; i<storage.getBlocksTotal(); ++i){
    StorageBlock block = storage.getBlock(i);
    if(isPresetBlock(block)){
      ProgramHeader* header = (ProgramHeader*)block.getData();
      int pc = header->magic&0x00ff;
      registerPatch(pc, block);
    }
  }
}

void PatchRegistry::registerPatch(uint8_t index, StorageBlock block){
  if(--index < MAX_NUMBER_OF_PATCHES && isPresetBlock(block))
    blocks[index] = block;
}

void PatchRegistry::storePatch(uint8_t index, uint8_t* data, size_t size){
  if(size > storage.getFreeSize())
    return error(FLASH_ERROR, "Insufficient flash available");
  if(index > 0 && index <= MAX_NUMBER_OF_PATCHES && size > sizeof(ProgramHeader)){     
    ProgramHeader* header = (ProgramHeader*)data;
    if(header->magic == 0xDADAC0DE){ // if it is a patch, set the program id
      header->magic = (header->magic&0xffffff00) | (index&0xff);
      StorageBlock block = storage.append(data, size);
      if(block.verify()){
	debugMessage("Patch stored to flash");
	if(blocks[index-1].verify())
	  blocks[index-1].setDeleted(); // delete old patch
	registerPatch(index, block);
	program.loadProgram(index);
	program.resetProgram(false);
	return;
      }
    }
  }
  return error(PROGRAM_ERROR, "Invalid patch");
}

const char* PatchRegistry::getName(unsigned int index){
  PatchDefinition* def = getPatchDefinition(index);
  if(def == NULL)
    return emptyPatch.getName();
  return def->getName();
}

unsigned int PatchRegistry::getNumberOfPatches(){
  // +1 for the current / dynamic patch in slot 0
  // return nofPatches+1;
  return MAX_NUMBER_OF_PATCHES+1;
}

PatchDefinition* PatchRegistry::getPatchDefinition(unsigned int index){
  PatchDefinition *def = NULL;
  static DynamicPatchDefinition flashPatch;
  if(index == 0)
    def = dynamicPatchDefinition;
  else if(--index < MAX_NUMBER_OF_PATCHES){
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
