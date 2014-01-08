#include "PatchController.h"
#include "ApplicationSettings.h"
#include "owlcontrol.h"

PatchController::PatchController(){
  green = new PatchProcessor(settings.patch_green);
  red = new PatchProcessor(settings.patch_red);
  setActiveSlot(GREEN);
}

PatchController::~PatchController(){
  delete green;
  delete red;
}

__attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  if(activeSlot == RED){
    red->setParameters(getAnalogValues());
    red->process(buffer);
  }else{
    green->setParameters(getAnalogValues());
    green->process(buffer);
  }
  if(green->index != settings.patch_green){
    delete green;
    green = new PatchProcessor(settings.patch_green);
  }
  if(red->index != settings.patch_red){
    delete red;
    red = new PatchProcessor(settings.patch_red);
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
