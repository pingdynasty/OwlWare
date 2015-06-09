#ifndef __DynamicPatchDefinition_hpp__
#define __DynamicPatchDefinition_hpp__

#include "PatchDefinition.hpp"
#include "ProgramHeader.h"

class DynamicPatchDefinition : public PatchDefinition {
  typedef void (*ProgramFunction)(void);
  ProgramHeader* header;
public:
  DynamicPatchDefinition() :
    PatchDefinition(programName, 2, 2) {}
  DynamicPatchDefinition(void* addr, uint32_t sz) :
    PatchDefinition(programName, 2, 2) {
    load(addr, sz);
  }
  void load(void* addr, uint32_t sz){
    programAddress = (uint32_t*)addr;
    programSize = sz;
    header = (ProgramHeader*)addr;
    stackBase = header->stackBegin;
    stackSize = (uint32_t)header->stackEnd - (uint32_t)header->stackBegin;
    jumpAddress = header->jumpAddress;
    linkAddress = header->linkAddress;
    programVector = header->programVector;
    strncpy(programName, header->programName, sizeof(programName));
    programFunction = NULL;

    // stackBase = (uint32_t*)*(programAddress+3); // stack base pointer (low end of heap/stack)
    // stackSize = *(programAddress+4) - *(programAddress+3);
    // strncpy(programName, (char*)(programAddress+5), sizeof(programName));
    // jumpAddress = (uint32_t*)*(programAddress+1); // main pointer
    // linkAddress = (uint32_t*)*(programAddress+2); // link base address
    // programFunction = NULL;
  }
  void copy(){
    /* copy program to ram */
    if((linkAddress == (uint32_t*)PATCHRAM && programSize <= 80*1024) ||
       (linkAddress == (uint32_t*)EXTRAM && programSize <= 1024*1024)){
      memcpy((void*)linkAddress, (void*)programAddress, programSize);
      // memmove((void*)linkAddress, (void*)programAddress, programSize);
      programFunction = (ProgramFunction)jumpAddress;
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
  ProgramVector* getProgramVector(){
    return programVector;
  }
private:
  char programName[16];
  ProgramFunction programFunction;
  uint32_t* linkAddress;
  uint32_t* jumpAddress;
  uint32_t* programAddress;
  uint32_t programSize;
  ProgramVector* programVector;
};


#endif // __DynamicPatchDefinition_hpp__
