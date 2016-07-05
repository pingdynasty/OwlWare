#include "PatchRegistry.h"
#include "FlashStorage.h"
#include "ProgramManager.h"
#include "ResourceHeader.h"
#include "ProgramHeader.h"
#include "DynamicPatchDefinition.hpp"
#include "message.h"

static PatchDefinition emptyPatch("---", 0, 0);

PatchRegistry::PatchRegistry(){}

void PatchRegistry::init() {
  storage.init();
  for(int i=0; i<storage.getBlocksTotal(); ++i){
    StorageBlock block = storage.getBlock(i);
    if(block.verify() && block.getDataSize() > 4){
      uint32_t magic = *(uint32_t*)block.getData();
      int id = magic&0x00ff;
      if(id > 0 && id <= MAX_NUMBER_OF_PATCHES)
	patchblocks[id-1] = block;
      else if(id > MAX_NUMBER_OF_PATCHES && 
	      id <= MAX_NUMBER_OF_PATCHES+MAX_NUMBER_OF_RESOURCES)
	resourceblocks[id-1-MAX_NUMBER_OF_PATCHES] = block;
    }
  }
}

ResourceHeader* PatchRegistry::getResource(uint8_t index){
  index = index - 1 - MAX_NUMBER_OF_PATCHES;
  if(index < MAX_NUMBER_OF_RESOURCES && resourceblocks[index].verify())
    return (ResourceHeader*)resourceblocks[index].getData();
  return NULL;
}

ResourceHeader* PatchRegistry::getResource(const char* name){
  for(int i=0; i<MAX_NUMBER_OF_RESOURCES; ++i){
    if(resourceblocks[i].verify()){
      ResourceHeader* hdr = (ResourceHeader*)resourceblocks[i].getData();
      if(strcmp(name, hdr->name) == 0)
	return hdr;
    }
  }
  return NULL;
}

void PatchRegistry::store(uint8_t index, uint8_t* data, size_t size){
  if(size > storage.getFreeSize() + storage.getDeletedSize())
    return error(FLASH_ERROR, "Insufficient flash available");
  if(size < 4)
    return error(FLASH_ERROR, "Invalid resource size");
  if(size > storage.getFreeSize())
    storage.defrag((uint8_t*)EXTRAM, 1024*1024);
  uint32_t* magic = (uint32_t*)data;
  if(*magic == 0xDADAC0DE && index > 0 && index <= MAX_NUMBER_OF_PATCHES){
    // if it is a patch, set the program id
    *magic = (*magic&0xffffff00) | (index&0xff);
    StorageBlock block = storage.append(data, size);
    if(block.verify()){
      debugMessage("Patch stored to flash");
      index = index - 1;
      if(patchblocks[index].verify())
	patchblocks[index].setDeleted(); // delete old patch
      patchblocks[index] = block;
      program.loadProgram(index);
      program.resetProgram(false);
    }
  }else if(*magic == 0xDADADEED && index > MAX_NUMBER_OF_PATCHES &&
	   index <= MAX_NUMBER_OF_PATCHES+MAX_NUMBER_OF_RESOURCES){
    // if it is data, set the resource id
    *magic = (*magic&0xffffff00) | (index&0xff);
    StorageBlock block = storage.append(data, size);
    if(block.verify()){
      debugMessage("Resource stored to flash");
      index = index - 1 - MAX_NUMBER_OF_PATCHES;
      if(resourceblocks[index].verify())
	resourceblocks[index].setDeleted();
      resourceblocks[index] = block;
    }
  }else{
    error(PROGRAM_ERROR, "Invalid magic");
  }
}

const char* PatchRegistry::getResourceName(unsigned int index){
  ResourceHeader* hdr = getResource(index);
  if(hdr == NULL)
    return emptyPatch.getName();
  return hdr->name;
}

const char* PatchRegistry::getPatchName(unsigned int index){
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
    if(patchblocks[index].verify()){
      flashPatch.load(patchblocks[index].getData(), patchblocks[index].getDataSize());
      if(flashPatch.verify())
	def = &flashPatch;
    }
  }
  if(def == &emptyPatch)
    def = NULL;
  return def;
}
