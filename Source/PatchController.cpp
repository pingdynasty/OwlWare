#include "PatchController.h"
#include "ApplicationSettings.h"
#include "MemoryBuffer.hpp"
#include "owlcontrol.h"

#define SINGLE_MODE          1
#define DUAL_GREEN_MODE      2
#define DUAL_RED_MODE        3
#define SERIES_GREEN_MODE    4
#define SERIES_RED_MODE      5
#define PARALLEL_GREEN_MODE  6
#define PARALLEL_RED_MODE    7

PatchController::PatchController(){
  setActiveSlot(RED);
  red = new PatchProcessor(settings.patch_red);
  setActiveSlot(GREEN);
  green = new PatchProcessor(settings.patch_green);
}

PatchController::~PatchController(){
  delete green;
  delete red;
}

__attribute__ ((section (".coderam")))
void PatchController::processParallel(AudioBuffer& buffer){
  MemoryBuffer left(buffer.getSamples(0), 1, buffer.getSize());
  MemoryBuffer right(buffer.getSamples(1), 1, buffer.getSize());
  green->patch->processAudio(buffer);
  red->patch->processAudio(buffer);
}

__attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  switch(mode){
  case SINGLE_MODE:
  case DUAL_GREEN_MODE:
    green->setParameterValues(getAnalogValues());
    green->patch->processAudio(buffer);
    break;
  case DUAL_RED_MODE:
    red->setParameterValues(getAnalogValues());
    red->patch->processAudio(buffer);
    break;
  case SERIES_GREEN_MODE:
    green->setParameterValues(getAnalogValues());
    green->patch->processAudio(buffer);
    red->patch->processAudio(buffer);
    break;
  case SERIES_RED_MODE:
    red->setParameterValues(getAnalogValues());
    green->patch->processAudio(buffer);
    red->patch->processAudio(buffer);
    break;
  case PARALLEL_GREEN_MODE:
    green->setParameterValues(getAnalogValues());
    processParallel(buffer);
    break;
  case PARALLEL_RED_MODE:
    red->setParameterValues(getAnalogValues());
    processParallel(buffer);
    break;
  }
  if(activeSlot == GREEN && green->index != settings.patch_green){
    delete green; // green must be active slot when constructor is called
    green = new PatchProcessor(settings.patch_green);
  }else if(activeSlot == RED && red->index != settings.patch_red){
      delete red; // red must be active slot when constructor is called
      red = new PatchProcessor(settings.patch_red);
  }
}

uint8_t PatchController::getActiveSlot(){
  return activeSlot;
}

void PatchController::setActiveSlot(uint8_t slot){
  // codec.softMute(true);
  activeSlot = slot;
  switch(settings.patch_mode){
  case(PATCHMODE_SINGLE):
    mode = SINGLE_MODE;
    break;
  case(PATCHMODE_DUAL):
    mode = slot == RED ? DUAL_RED_MODE : DUAL_GREEN_MODE;
    break;
  case(PATCHMODE_SERIES):
    mode = slot == RED ? SERIES_RED_MODE : SERIES_GREEN_MODE;
    break;
  case(PATCHMODE_PARALLEL):
    mode = slot == RED ? PARALLEL_RED_MODE : PARALLEL_GREEN_MODE;
    break;
  }
}

void PatchController::toggleActiveSlot(){
  if(activeSlot == GREEN)
    setActiveSlot(RED);
  else
    setActiveSlot(GREEN);
}

PatchProcessor* PatchController::getCurrentPatchProcessor(){
  if(activeSlot == RED)
    return red;
  return green;
}
