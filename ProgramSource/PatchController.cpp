#include "PatchController.h"
#include "owlcontrol.h"
// #include "CodecController.h"
#include "MemoryBuffer.hpp"
#include "ProgramVector.h"
#include "PatchRegistry.h"
#include "PatchProcessor.h"

PatchController::PatchController(){
}

PatchController::~PatchController(){
}

void PatchController::init(){
  parameterValues = getProgramVector()->parameters;
}

void PatchController::reset(){
  init();
}

PatchProcessor* getInitialisingPatchProcessor(){
  return patches.getInitialisingPatchProcessor();
}

PatchProcessor* PatchController::getInitialisingPatchProcessor(){
  return processor;
}

// __attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  processor.setParameterValues(parameterValues);
  processor.patch->processAudio(buffer);
}
