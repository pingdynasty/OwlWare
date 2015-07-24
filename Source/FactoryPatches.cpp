#include "FactoryPatches.h"
#include "PatchProcessor.h"
#include "PatchRegistry.h"
#include "sramalloc.h"
#include "device.h"
#include "basicmaths.h"
#include "ProgramVector.h"

// #define STATIC_PROGRAM_STACK_SIZE   (64*1024)
// #define STATIC_PROGRAM_STACK_BASE   ((uint32_t*)CCMRAM)
#define STATIC_PROGRAM_STACK_BASE   0
#define STATIC_PROGRAM_STACK_SIZE   0

extern ProgramVector staticVector;

PatchProcessor *processor;
PatchProcessor* getInitialisingPatchProcessor(){
  return processor;
}

void FactoryPatchDefinition::run(){
  sram_init((char*)EXTRAM, 1024*1024);
  PatchProcessor proc; // 4kb : create on stack
  processor = &proc;
  // processor = new PatchProcessor();
  Patch* patch = create();
  processor->setPatch(patch);
  getProgramVector()->heap_bytes_used = sram_used();
  processor->run();
}

#include "factory.h"

template<class T> struct Register {
  static Patch* construct() {
    return new T();
  }
};

int FACTORY_PATCH_COUNT = 0;
static FactoryPatchDefinition factorypatches[MAX_FACTORY_PATCHES];
void registerPatch(char* nm, uint8_t ins, uint8_t outs, PatchCreator c){
  if(FACTORY_PATCH_COUNT < MAX_FACTORY_PATCHES)
    factorypatches[FACTORY_PATCH_COUNT++].setup(nm, ins, outs, c);
}

#define REGISTER_PATCH(T, STR, IN, OUT) registerPatch((char*)STR, IN, OUT, Register<T>::construct);

// #undefine REGISTER_PATCH
// #define REGISTER_PATCH(T, STR, IN, OUT) registerPatch(STR, IN, OUT, Register<T>::construct)

// void registerPatch(const char* name, uint8_t inputs, uint8_t outputs,
// 		   PatchCreator creator){
// }

void FactoryPatchDefinition::init(){
  FACTORY_PATCH_COUNT = 0;
#include "factory.cpp"
}

FactoryPatchDefinition::FactoryPatchDefinition() {
  stackBase = STATIC_PROGRAM_STACK_BASE;
  stackSize = STATIC_PROGRAM_STACK_SIZE;
  programVector = &staticVector;
}

FactoryPatchDefinition::FactoryPatchDefinition(char* name, uint8_t inputs, uint8_t outputs, PatchCreator c) :
  PatchDefinition(name, inputs, outputs), creator(c) {
  stackBase = STATIC_PROGRAM_STACK_BASE;
  stackSize = STATIC_PROGRAM_STACK_SIZE;
  // stackSize = 0;
  programVector = &staticVector;
}

void FactoryPatchDefinition::setup(char* nm, uint8_t ins, uint8_t outs, PatchCreator c){
  name = nm;
  inputs = ins;
  outputs = outs;
  creator = c;
  registry.registerPatch(this);
}
