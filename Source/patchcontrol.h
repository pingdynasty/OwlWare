#ifndef __patchcontrol_h__
#define __patchcontrol_h__

#include <inttypes.h>
#include "StompBox.h"

void setupPatches();
uint8_t getNumberOfPatches();
Patch* getPatch(uint8_t index);

#endif // __patchcontrol_h__
