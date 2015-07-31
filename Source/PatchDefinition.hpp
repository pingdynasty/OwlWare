#ifndef __PatchDefinition_hpp__
#define __PatchDefinition_hpp__

#include <stdint.h>
#include <string.h>
#include "ProgramVector.h"

class PatchDefinition {
public:
  PatchDefinition(const char* nm, uint8_t ins, uint8_t outs) :
    name(nm), inputs(ins), outputs(outs) {
  }
  PatchDefinition()
  { // : name(NULL), inputs(0), outputs(0) {
  }
  virtual ~PatchDefinition(){}
  // ProgramFunction getProgramFunction(){
  //   return function;
  // }
  virtual void run(){}
  // uint32_t* getAddress(){
  //   return address;
  // }
  // uint32_t getSize(){
  //   return size;
  // }
  uint32_t* getStackBase(){
    return stackBase;
  }
  uint32_t getStackSize(){
    return stackSize;
  }
  const char* getName(){
    return name;
  }
  ProgramVector* getProgramVector(){
    return programVector;
  }
  // void clear();
// protected:
  const char* name;
  uint8_t inputs;
  uint8_t outputs;
  uint32_t* stackBase;
  uint32_t stackSize;
  ProgramVector* programVector;
};

#endif // __PatchDefinition_hpp__
