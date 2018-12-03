#include "JotReverbPatch.hpp"
#include "FreeVerbPatch.hpp"
#include "PlateVerbPatch.hpp"
#include "MoogPatch.hpp"
#include "FourBandsEqPatch.hpp"
#include "ParametricEqPatch.hpp"
#include "mdaPorts/MdaBandistoPatch.hpp"
#include "OverdrivePatch.hpp"
#include "Faust/StereoWahPatch.hpp"
#include "Faust/PitchShifterPatch.hpp"
#include "PhaserPatch.hpp"
#include "EnvelopeFilterPatch.hpp"

/* #include "Faust/SmoothDelayPatch.hpp" */
/* #include "LpfDelayPatch.hpp" */
/* #include "SimpleStereoDelayPatch.hpp" */
/* #include "SlapBackEchoPatch.hpp" */
/* #include "PingPongDelayPatch.hpp" */
/* #include "DubDelayPatch.hpp" */
/* #include "TempoSyncedStereoDelayPatch.hpp" */
/* #include "TempoSyncedPingPongDelayPatch.hpp" */

/* #include "PsycheFilterPatch.hpp" */
/* #include "Qompression.hpp" */
/* #include "Guitarix/GuitarixOverdrivePatch.hpp" */
/* #include "Guitarix/GuitarixOscTubePatch.hpp" */
/* #include "Guitarix/GuitarixDistortion1Patch.hpp" */
/* #include "Guitarix/GuitarixBMfpPatch.hpp" */
/* #include "Guitarix/GuitarixCompressorPatch.hpp" */
/* // #include "Guitarix/GuitarixDunwahPatch.hpp" */
/* #include "Guitarix/GuitarixMoogPatch.hpp" */
/* #include "Guitarix/GuitarixFlangerGXPatch.hpp" */
/* #include "Guitarix/GuitarixTonePatch.hpp" */
/* #include "ComplexorPatch.hpp" */
/* #include "DigitalMayhemPatch.hpp" */
/* #include "GhostNotePatch.hpp" */
/* #include "Faust/HarpAutoPatch.hpp" */
/* #include "Contest/SirenPatch.hpp" */
/* #include "VibroFlangePatch.hpp" */
/* #include "mdaPorts/MdaStereoPatch.hpp" */
/* #include "mdaPorts/MdaTransientPatch.hpp" */
/* #include "OliLarkin/XFMPatch.hpp" */

/* #include "Faust/AutoWahPatch.hpp" */
/* #include "StereoPatch.hpp" */

/* // #include "RingModulatorPatch.hpp" */

/* #include "OliLarkin/BlipperPatch.hpp" */
/* #include "OliLarkin/DroneBoxPatch.hpp" */
/* #include "OliLarkin/StereoFreqShifterPatch.hpp" */
/* #include "OliLarkin/DualFreqShifterPatch.hpp" */
/* #include "OliLarkin/DualPitchShifterPatch.hpp" */
/* #include "OliLarkin/ThruZeroFlangerPatch.hpp" */
/* #include "OliLarkin/WeirdPhaserPatch.hpp" */
/* #include "OliLarkin/XFMPatch.hpp" */

/* #include "GainPatch.hpp" */
/* #include "SimpleStereoDelayPatch.hpp" */
/* #include "ParametricEqPatch.hpp" */
/* #include "OverdrivePatch.hpp" */
/* #include "PhaserPatch.hpp" */
/* #include "StateVariableFilterPatch.hpp" */
/* #include "ResonantFilterPatch.hpp" */
/* #include "LeakyIntegratorPatch.hpp" */
/* #include "FreeVerbPatch.hpp" */
/* #include "OctaveDownPatch.hpp" */
/* #include "StereoMixerPatch.hpp" */
/* #include "SynthPatch.hpp" */
/* #include "FourBandsEqPatch.hpp" */
/* #include "JotReverbPatch.hpp" */
/* /\\* #include "Contest/BiasedDelayPatch.hpp" *\\/ */
/* /\\* #include "Contest/DualTremoloPatch.hpp" *\/ */
/* #include "mdaPorts/MdaBandistoPatch.cpp" */
/* #include "Qompression.hpp" */
/* #include "PsycheFilterPatch.hpp" */
/* #include "TremoloPatch.hpp" */
/* #include "DigitalMayhemPatch.hpp" */
/* #include "ReverseReverbPatch.hpp" */
/* #include "SimpleDistortionPatch.hpp" */
/* #include "MoogStereoPatch.hpp" */
/* #include "LpfDelayPatch.hpp" */
/* #include "BittaPatch.hpp" */
/* #include "VidhaPatch.hpp" */
/* /\* #include "OverOverPatch.hpp" *\/ */
/* /\* #include "ToneFilterPatch.hpp" *\/ */
/* #include "PlateVerbPatch.hpp" */
/* #include "CompressorPatch.hpp" */

/* #include "Faust/FaustVerbPatch.hpp" */
/* #include "Faust/HarpPatch.hpp" */
/* #include "Faust/HarpAutoPatch.hpp" */
/* #include "Faust/EchoPatch.hpp" */
/* #include "Faust/SmoothDelayPatch.hpp" */
/* #include "Faust/StereoEchoPatch.hpp" */
/* #include "Faust/CrybabyPatch.hpp" */
/* #include "Faust/StereoWahPatch.hpp" */
/* #include "Faust/LowPassFilterPatch.hpp" */
/* #include "Faust/LowShelfPatch.hpp" */
/* #include "Faust/HighShelfPatch.hpp" */
/* #include "Faust/PitchShifterPatch.hpp" */

/* #include "Guitarix/GuitarixCompressorPatch.hpp" */
/* #include "Guitarix/GuitarixPhaserPatch.hpp" */
/* #include "Guitarix/GuitarixMoogPatch.hpp" */
/* #include "Guitarix/GuitarixOverdrivePatch.hpp" */
/* /\* #include "Guitarix/GuitarixPhaserMonoPatch.hpp" *\/ */
/* #include "Guitarix/GuitarixOscTubePatch.hpp" */
/* #include "Guitarix/GuitarixFlangerGXPatch.hpp" */
/* #include "Guitarix/GuitarixDunwahPatch.hpp" */
/* #include "Guitarix/GuitarixTonePatch.hpp" */
/* #include "Guitarix/GuitarixBMfpPatch.hpp" */
/* #include "Guitarix/GuitarixDistortion1Patch.hpp" */

/* #include "Contest/SirenPatch.hpp" */
/* #include "EnvelopeFilterPatch.hpp" */

/* // #include "Contest/ConnyPatch.hpp" */
/* #include "Faust/AutoWahPatch.hpp" */
/* // #include "Faust/QompanderPatch.hpp" */
/* // #include "SimpleDriveDelayPatch.hpp" */
/* // #include "Autotalent/AutotalentPatch.hpp" */
/* // #include "TemplatePatch.hpp" */
/* // #include "TemplatePatch.hpp" */
/* // #include "Contest/JumpDelay.hpp" /\* uses calloc and free *\/ */
/* // #include "Contest/SampleJitterPatch.hpp" /\* requires juce::Random *\/ */
/* // #include "Contest/SirenPatch.hpp" /\* causes assert_failed in DMA_GetFlagStatus() *\/ */
/* // #include "LpfDelayPhaserPatch.hpp" /\* not compatible with Windows yet *\/ */
/* // #include "WaveshaperPatch.hpp" /\* not compatible with Windows yet *\/ */
/* // #include "TestTonePatch.hpp" */
/* // #include "FlangerPatch.hpp" */
