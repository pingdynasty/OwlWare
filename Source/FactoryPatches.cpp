#include "FactoryPatches.h"
#include "PatchProcessor.h"
#include "PatchRegistry.h"
#include "sramalloc.h"
#include "device.h"
#include "basicmaths.h"
#include "ProgramVector.h"

PatchProcessor *processor;
PatchProcessor* getInitialisingPatchProcessor(){
  return processor;
}

void FactoryPatchDefinition::run(){
  sram_init((char*)EXTRAM, 1024*1024);
  processor = new PatchProcessor();
  Patch* patch = create();
  processor->setPatch(patch);
  getProgramVector()->heap_bytes_used = sram_used();
  processor->run();
}

#include "includes.h"

// #define STATIC_PROGRAM_STACK_SIZE   (32*1024)
// uint8_t spHeap[ STATIC_PROGRAM_STACK_SIZE ] CCM;

template<class T> struct Register {
  static Patch* construct() {
    return new T();
  }
};

#define REGISTER_PATCH(T, STR, IN, OUT) factorypatches[i++].setup((char*)STR, IN, OUT, Register<T>::construct)

#define MAX_FACTORY_PATCHES 32
FactoryPatchDefinition factorypatches[MAX_FACTORY_PATCHES];

// #undefine REGISTER_PATCH
// #define REGISTER_PATCH(T, STR, IN, OUT) registerPatch(STR, IN, OUT, Register<T>::construct)

// void registerPatch(const char* name, uint8_t inputs, uint8_t outputs,
// 		   PatchCreator creator){
// }

int FACTORY_PATCH_COUNT;
void FactoryPatchDefinition::init(){
  int i=0;
#include "patches.cpp"
  FACTORY_PATCH_COUNT = i;
}

FactoryPatchDefinition::FactoryPatchDefinition() {
  // stackBase = (uint32_t*)ucHeap;
  stackBase = NULL;
  stackSize = 0;
}

FactoryPatchDefinition::FactoryPatchDefinition(char* name, uint8_t inputs, uint8_t outputs, PatchCreator c) :
  PatchDefinition(name, inputs, outputs), creator(c) {
  // stackBase = (uint32_t*)ucHeap;
  stackBase = NULL;
  stackSize = 0;
}

void FactoryPatchDefinition::setup(char* nm, uint8_t ins, uint8_t outs, PatchCreator c){
  name = nm;
  inputs = ins;
  outputs = outs;
  creator = c;
  registry.registerPatch(this);
}
