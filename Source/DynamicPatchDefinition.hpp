#ifndef __DynamicPatchDefinition_hpp__
#define __DynamicPatchDefinition_hpp__

#include "PatchDefinition.hpp"
#include "ProgramHeader.h"

class DynamicPatchDefinition : public PatchDefinition {
private:
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction;
  uint32_t* linkAddress;
  uint32_t* jumpAddress;
  uint32_t* programAddress;
  uint32_t programSize;
  ProgramHeader* header;
  char programName[24];
public:
  DynamicPatchDefinition() :
    PatchDefinition(programName, 2, 2) {}
  DynamicPatchDefinition(void* addr, uint32_t sz) :
    PatchDefinition(programName, 2, 2) {
    load(addr, sz);
  }
  bool load(void* addr, uint32_t sz){
    programAddress = (uint32_t*)addr;
    header = (ProgramHeader*)addr;
    linkAddress = header->linkAddress;
    programSize = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
    if(sz != programSize)
      return false;
    stackBase = header->stackBegin;
    stackSize = (uint32_t)header->stackEnd - (uint32_t)header->stackBegin;
    jumpAddress = header->jumpAddress;
    programVector = header->programVector;
    strncpy(programName, header->programName, sizeof(programName));
    programFunction = (ProgramFunction)jumpAddress;
    return true;
  }
  void copy(){
    /* copy program to ram */
    if((linkAddress == (uint32_t*)PATCHRAM && programSize <= 80*1024) ||
       (linkAddress == (uint32_t*)EXTRAM && programSize <= 1024*1024)){
      memcpy((void*)linkAddress, (void*)programAddress, programSize);
      // memmove((void*)linkAddress, (void*)programAddress, programSize);
      if(programAddress == (uint32_t*)EXTRAM)
	// avoid copying dynamic patch again after reset
	programAddress = linkAddress; 
    }else{
      programFunction = NULL;
    }
  }
  bool verify(){
    // check we've got an entry function
    if(programFunction == NULL)
      return false;
    // check magic
    if(*(uint32_t*)programAddress != 0xDADAC0DE)
      return false;
    // sanity-check stack base address and size
    uint32_t sb = (uint32_t)stackBase;
    if((sb >= PATCHRAM && sb+stackSize <= (PATCHRAM+80*1024)) ||
       (sb >= CCMRAM && sb+stackSize <= (CCMRAM+64*1024)) ||
       (sb >= EXTRAM && sb+stackSize <= (EXTRAM+80*1024)) ||
       (sb == 0 && stackSize == 0))
      return true;
    return false;
  }
  void run(){
    if(linkAddress != programAddress)
      copy();
    if(verify())
      programFunction();
  }
  uint32_t getProgramSize(){
    return programSize;
  }
  uint32_t* getLinkAddress(){
    return linkAddress;
  }
};


#endif // __DynamicPatchDefinition_hpp__
