#include "PatchController.h"
#include "ApplicationSettings.h"
#include "owlcontrol.h"

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
void PatchController::process(AudioBuffer& buffer){
  if(activeSlot == RED){
    red->setParameterValues(getAnalogValues());
    red->patch->processAudio(buffer);
    if(red->index != settings.patch_red){
      delete red; // red must be active slot when constructor is called
      red = new PatchProcessor(settings.patch_red);
    }
  }else{
    green->setParameterValues(getAnalogValues());
    green->patch->processAudio(buffer);
    if(green->index != settings.patch_green){
      delete green; // green must be active slot when constructor is called
      green = new PatchProcessor(settings.patch_green);
    }
  }
}

uint8_t PatchController::getActiveSlot(){
  return activeSlot;
}

void PatchController::setActiveSlot(uint8_t slot){
  // codec.softMute(true);
  activeSlot = slot;
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
