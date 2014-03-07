#include "PatchController.h"
#include "CodecController.h"
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
}

PatchController::~PatchController(){
}

void PatchController::init(){
  setActiveSlot(RED);
  red.setPatch(settings.patch_red);
  setActiveSlot(GREEN);
  green.setPatch(settings.patch_green);
}

__attribute__ ((section (".coderam")))
void PatchController::processParallel(AudioBuffer& buffer){
  MemoryBuffer left(buffer.getSamples(0), 1, buffer.getSize());
  MemoryBuffer right(buffer.getSamples(1), 1, buffer.getSize());
  green.patch->processAudio(left);
  red.patch->processAudio(right);
}

__attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  if(activeSlot == GREEN && green.index != settings.patch_green){
    memset(buffer.getSamples(0), 0, buffer.getChannels()*buffer.getSize()*sizeof(float));
    // green must be active slot when patch constructor is called
    green.setPatch(settings.patch_green);
    codec.softMute(false);
    debugClear();
    return;
  }else if(activeSlot == RED && red.index != settings.patch_red){
    memset(buffer.getSamples(0), 0, buffer.getChannels()*buffer.getSize()*sizeof(float));
    // red must be active slot when constructor is called
    red.setPatch(settings.patch_red);
    codec.softMute(false);
    debugClear();
    return;
  }
  switch(mode){
  case SINGLE_MODE:
  case DUAL_GREEN_MODE:
    green.setParameterValues(getAnalogValues());
    green.patch->processAudio(buffer);
    break;
  case DUAL_RED_MODE:
    red.setParameterValues(getAnalogValues());
    red.patch->processAudio(buffer);
    break;
  case SERIES_GREEN_MODE:
    green.setParameterValues(getAnalogValues());
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case SERIES_RED_MODE:
    red.setParameterValues(getAnalogValues());
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case PARALLEL_GREEN_MODE:
    green.setParameterValues(getAnalogValues());
    processParallel(buffer);
    break;
  case PARALLEL_RED_MODE:
    red.setParameterValues(getAnalogValues());
    processParallel(buffer);
    break;
  }
}

void PatchController::setPatch(LedPin slot, uint8_t index){
  codec.softMute(true);
  if(slot == RED){
    settings.patch_red = index;
  }else{
    settings.patch_green = index;
  }
  setActiveSlot(slot);
}

LedPin PatchController::getActiveSlot(){
  return activeSlot;
}

void PatchController::setActiveSlot(LedPin slot){
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
  activeSlot = slot;
  setLed(slot);
}

void PatchController::toggleActiveSlot(){
  if(activeSlot == GREEN)
    setActiveSlot(RED);
  else
    setActiveSlot(GREEN);
}

PatchProcessor* PatchController::getCurrentPatchProcessor(){
  if(activeSlot == RED)
    return &red;
  return &green;
}
