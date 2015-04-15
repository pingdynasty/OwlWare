#include "PatchController.h"
#include "owlcontrol.h"
// #include "CodecController.h"
#include "MemoryBuffer.hpp"
#include "SharedMemory.h"
#include "PatchRegistry.h"
#include "PatchProcessor.h"

#define SINGLE_MODE          1
#define DUAL_GREEN_MODE      2
#define DUAL_RED_MODE        3
#define SERIES_GREEN_MODE    4
#define SERIES_RED_MODE      5
#define PARALLEL_GREEN_MODE  6
#define PARALLEL_RED_MODE    7

PatchController::PatchController(){
}

PatchController::~PatchController(){
}

void PatchController::init(){
  parameterValues = getSharedMemory()->parameters;
  // setActiveSlot(GREEN);
  initialisePatch(GREEN, getGreenPatchId());
  initialisePatch(RED, getRedPatchId());
}

void PatchController::reset(){
  init();
}

// __attribute__ ((section (".coderam")))
void PatchController::processParallel(AudioBuffer& buffer){
  MemoryBuffer left(buffer.getSamples(0), 1, buffer.getSize());
  MemoryBuffer right(buffer.getSamples(1), 1, buffer.getSize());
  green.patch->processAudio(left);
  red.patch->processAudio(right);
}

// __attribute__ ((section (".coderam")))
void PatchController::initialisePatch(LedPin slot, uint8_t index){
  // the initialisingProcessor must be set
  // so that it can be picked up by a call to getInitialisingProcessor() from the Patch constructor
  if(slot == RED){
    initialisingProcessor = &red;
    Patch* patch = registry.create(index);
    red.setPatch(patch);
    red.index = index;
  }else{
    initialisingProcessor = &green;
    Patch* patch = registry.create(index);
    green.setPatch(patch);
    green.index = index;
  }
}

PatchProcessor* getInitialisingPatchProcessor(){
  return patches.getInitialisingPatchProcessor();
}

PatchProcessor* PatchController::getInitialisingPatchProcessor(){
  return initialisingProcessor;
}

// __attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  mode = getPatchMode();
  if(isButtonPressed(RED_BUTTON)){
    mode |= 1;
    if(red.index != getRedPatchId()){
      initialisePatch(RED, getRedPatchId());
      return;
    }
  }else{
    mode &= ~1;
    if(green.index != getGreenPatchId()){
      initialisePatch(GREEN, getGreenPatchId());
      return;
    }
  }
  switch(mode){
  case SINGLE_GREEN_MODE:
  case DUAL_GREEN_MODE:
    green.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    break;
  case SINGLE_RED_MODE:
  case DUAL_RED_MODE:
    red.setParameterValues(parameterValues);
    red.patch->processAudio(buffer);
    break;
  case SERIES_GREEN_MODE:
    green.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case SERIES_RED_MODE:
    red.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case PARALLEL_GREEN_MODE:
    green.setParameterValues(parameterValues);
    processParallel(buffer);
    break;
  case PARALLEL_RED_MODE:
    red.setParameterValues(parameterValues);
    processParallel(buffer);
    break;
  }
}

// void PatchController::setPatch(LedPin slot, uint8_t index){
//   // codec.softMute(true);
//   if(slot == RED){
//     settings.patch_red = index;
//   }else{
//     settings.patch_green = index;
//   }
//   setActiveSlot(slot);
// }

// LedPin PatchController::getActiveSlot(){
//   return activeSlot;
// }

// void PatchController::setActiveSlot(LedPin slot){
//   switch(getPatchMode()){
//   case(PATCHMODE_SINGLE):
//     mode = SINGLE_MODE;
//     break;
//   case(PATCHMODE_DUAL):
//     mode = slot == RED ? DUAL_RED_MODE : DUAL_GREEN_MODE;
//     break;
//   case(PATCHMODE_SERIES):
//     mode = slot == RED ? SERIES_RED_MODE : SERIES_GREEN_MODE;
//     break;
//   case(PATCHMODE_PARALLEL):
//     mode = slot == RED ? PARALLEL_RED_MODE : PARALLEL_GREEN_MODE;
//     break;
//   }
//   activeSlot = slot;
//   setLed(slot);
// }

// void PatchController::toggleActiveSlot(){
//   if(activeSlot == GREEN)
//     setActiveSlot(RED);
//   else
//     setActiveSlot(GREEN);
// }

// Patch* PatchController::getActivePatch(){
//   if(mode & 1)
//     return &red;
//   return &green;
// }

// PatchProcessor* PatchController::getActivePatchProcessor(){
//   if(mode & 1)
//     return &red;
//   return &green;
// }
