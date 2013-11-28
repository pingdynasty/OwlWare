#include "patchcontrol.h"
#include "device.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif /* min */
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif /* max */
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif /* abs */

#define registerParameter(...)

#include "OwlPatches/CopyPatch.hpp"
#include "OwlPatches/StereoGainPatch.hpp"
#include "OwlPatches/StereoMixerPatch.hpp"
#include "OwlPatches/SimpleDelayPatch.hpp"
#include "OwlPatches/ParametricEqPatch.hpp"
#include "OwlPatches/PhaserPatch.hpp"
#include "OwlPatches/ResonantFilterPatch.hpp"
#include "OwlPatches/StateVariableFilterPatch.hpp"
#include "OwlPatches/LeakyIntegratorPatch.hpp"
#include "OwlPatches/DroneBoxPatch.hpp"
#include "OwlPatches/LpfDelayPatch.hpp"
#include "OwlPatches/LpfDelayPhaserPatch.hpp"
#include "OwlPatches/TestTonePatch.hpp"
#include "OwlPatches/FixedDelayPatch.hpp"

#include "OwlPatches/OverdrivePatch.hpp"
#include "OwlPatches/FlangerPatch.hpp"

// #include "OwlPatches/ConnyPatch.hpp"
// #include "OwlPatches/SirenPatch.hpp"

#define NOF_PATCHES 16
Patch* patches[NOF_PATCHES];

CopyPatch patch0;
StereoGainPatch patch1;
StereoMixerPatch patch2;
ParametricEqPatch patch3;
PhaserPatch patch4;
ResonantFilterPatch patch5;
StateVariableFilterPatch patch6;
LeakyIntegratorPatch patch7;
DroneBoxPatch patch8;
SimpleDelayPatch patch9;
LpfDelayPatch patch10;
LpfDelayPhaserPatch patch11;
TestTonePatch patch12;
FixedDelayPatch patch13;
FlangerPatch patch14;
OverdrivePatch patch15;

// ConnyPatch patch4;
// SirenPatch patch9;

void setupPatches(){
  patches[0] = &patch0;
  patches[1] = &patch1;
  patches[2] = &patch2;
  patches[3] = &patch3;
  patches[4] = &patch4;
  patches[5] = &patch5;
  patches[6] = &patch6;
  patches[7] = &patch7;
  patches[8] = &patch8;
  patches[9] = &patch9;
  patches[10] = &patch10;
  patches[11] = &patch11;
  patches[12] = &patch12;
  patches[13] = &patch13;
  patches[14] = &patch14;
  patches[15] = &patch15;
}

Patch* getPatch(uint8_t index){
  assert_param(index < NOF_PATCHES);
  return patches[index];
}

uint8_t getNumberOfPatches(){
  return NOF_PATCHES;
}
