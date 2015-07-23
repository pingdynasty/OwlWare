/* Help:
 REGISTER_PATCH(yourHppFileName,
                "NameThatWillBeDisplayed",
                numberInputChannels,
                numberOutputChannels);
 
*/
// sed 's/^REGISTER_PATCH.*"\(.*\)".*/\1/g' < Source/factory.cpp

REGISTER_PATCH(JotReverbPatch, "Jot Reverb", 2, 2);
REGISTER_PATCH(FreeVerbPatch, "FreeVerb", 1, 1);
REGISTER_PATCH(PlateVerbPatch, "Plate Reverb", 1, 1);
REGISTER_PATCH(FourBandsEqPatch, "Four Band EQ", 2, 2);
REGISTER_PATCH(ParametricEqPatch, "Parametric EQ", 2, 2);
REGISTER_PATCH(OverdrivePatch, "Overdrive", 2, 2);
REGISTER_PATCH(StereoWahPatch, "StereoWah", 2, 2);
REGISTER_PATCH(PitchShifterPatch, "Pitch Shifter", 2, 2);
REGISTER_PATCH(PhaserPatch, "Phaser", 1, 1);
REGISTER_PATCH(EnvelopeFilterPatch, "Envelope Filter", 2, 2);
REGISTER_PATCH(SmoothDelayPatch, "Smooth Delay", 1, 1);
REGISTER_PATCH(SimpleStereoDelayPatch, "Stereo Delay", 2, 2);
REGISTER_PATCH(PsycheFilterPatch, "Psyche Filter", 2, 2);
REGISTER_PATCH(QompressionPatch, "Qompression", 2, 2);
REGISTER_PATCH(MdaBandistoPatch, "Bandisto", 2, 2);
REGISTER_PATCH(VibroFlangePatch, "Vibro-Flange", 1, 1); /* 102% */
REGISTER_PATCH(GuitarixOverdrivePatch, "Guitarix/Overdrive", 1, 1);
REGISTER_PATCH(GuitarixOscTubePatch, "Guitarix/OscTube", 1, 1);
REGISTER_PATCH(GuitarixDistortion1Patch, "Guitarix/Distortion1", 1, 1);
REGISTER_PATCH(GuitarixBMfpPatch, "Guitarix/BigMuffFuzz", 1, 1);
REGISTER_PATCH(GuitarixCompressorPatch, "Guitarix/Compressor", 1, 1);
REGISTER_PATCH(GuitarixDunwahPatch, "Guitarix/Dunwah", 1, 1);
REGISTER_PATCH(GuitarixMoogPatch, "Guitarix/Moog Filter", 1, 1);
REGISTER_PATCH(GuitarixFlangerGXPatch, "Guitarix/FlangerGX", 1, 1);
REGISTER_PATCH(GuitarixTonePatch, "Guitarix/Tone", 1, 1);
REGISTER_PATCH(DroneBoxPatch, "OL/DroneBox", 2, 2);
REGISTER_PATCH(DualPitchShifterPatch, "OL/Dual Pitch Shifter", 2, 2);
REGISTER_PATCH(ThruZeroFlangerPatch, "OL/Thru Zero Flanger", 2, 2);
REGISTER_PATCH(BlipperPatch, "OL/Blipper", 2, 2); // PC-2 emulation
REGISTER_PATCH(WeirdPhaserPatch, "OL/Weird Phaser", 2, 2);
REGISTER_PATCH(LogisticFunctionPatch, "MK/Complexor", 2, 2);
REGISTER_PATCH(DigitalMayhemPatch, "RS/Digital Mayhem", 1, 1);
REGISTER_PATCH(GhostNotePatch,"RS/GhostNote", 1, 1);
REGISTER_PATCH(MoogStereoPatch, "Moog Drive", 2, 2);
REGISTER_PATCH(LpfDelayPatch, "Lowpass Delay", 1, 1);
REGISTER_PATCH(SirenPatch, "Siren", 0, 1);

// REGISTER_PATCH(HarpAutoPatch, "Faust/AutoHarp", 0, 1);
// REGISTER_PATCH(AutoWahPatch, "AutoWah", 1, 1); /* 104% */


// REGISTER_PATCH(XFMPatch, "OL/XFM Oscillator", 2, 2);
// REGISTER_PATCH(DualFreqShifterPatch, "OL/Dual Frequency Shifter", 2, 2);
// REGISTER_PATCH(StereoFreqShifterPatch, "OL/Stereo Frequency Shifter", 2, 2);

// REGISTER_PATCH(ReverseReverbPatch, "RS/Reverse Reverb", 1, 1);
// REGISTER_PATCH(GuitarixPhaserPatch, "Guitarix/Phaser", 2, 2);
// REGISTER_PATCH(FaustVerbPatch, "FreeVerb", 1, 1);

// REGISTER_PATCH(SirenPatch, "Siren", 0, 1);

// REGISTER_PATCH(VibroFlangePatch, "Vibro-Flange", 1, 1); /* 102% */
// REGISTER_PATCH(RingModulatorPatch, "Ring Modulator", 2, 2); /* 174% */
// REGISTER_PATCH(OctaveDownPatch, "Octave Pitch Shifter", 1, 1);

// REGISTER_PATCH(ToneFilterPatch, "ToneFilter", 1, 1);
// REGISTER_PATCH(CompressorPatch, "Compressor", 1, 1);

// REGISTER_PATCH(DroneBoxPatch, "Contest/DroneBox", 1, 1);
// REGISTER_PATCH(PatchSelectorPatch, "Patch Selector", 0, 0);

// REGISTER_PATCH(GainPatch, "Gain", 2, 2);
// REGISTER_PATCH(StateVariableFilterPatch, "State Variable Filter", 1, 1);
// REGISTER_PATCH(ResonantFilterPatch, "Resonant Low Pass Filter", 1, 1);
// REGISTER_PATCH(LeakyIntegratorPatch, "Leaky Integrator", 1, 1);
// REGISTER_PATCH(StereoMixerPatch, "Stereo Mixer", 2, 2);
// REGISTER_PATCH(SynthPatch, "Synthesizer", 1, 1);
// REGISTER_PATCH(BiasedDelayPatch, "Contest/BiasedDelay", 2, 2);
// REGISTER_PATCH(little_blo_bleep, "Contest/blo bleep", 2, 2);
// REGISTER_PATCH(BiasPatch, "Contest/Bias", 2, 2);
// REGISTER_PATCH(BitH8rPatch, "Contest/BitH8r", 2, 2);
// REGISTER_PATCH(MdaStereoPatch, "MDA/Stereo", 2, 2);
// REGISTER_PATCH(MdaTransientPatch, "MDA/Transient", 2, 2);
// REGISTER_PATCH(QompressionPatch, "Qompression", 2, 2);
// REGISTER_PATCH(SimpleDistortionPatch, "RS/Simple Distortion", 1, 1);
// REGISTER_PATCH(TremoloPatch, "Tremolo", 2, 2);
// REGISTER_PATCH(KarplusStrongPatch, "Karplus Strong", 0, 2);
// REGISTER_PATCH(BittaPatch, "Bitta", 1, 1);
// REGISTER_PATCH(VidhaPatch, "Vidha", 2, 2);
// REGISTER_PATCH(OverOverPatch, "OverOverDrive", 1, 1);

// REGISTER_PATCH(FaustVerbPatch, "Faust/FreeVerb", 1, 1);
// REGISTER_PATCH(HarpPatch, "Faust/Harp", 0, 1);
// REGISTER_PATCH(HarpAutoPatch, "Faust/AutoHarp", 0, 1);
// REGISTER_PATCH(EchoPatch, "Faust/1 Sec Echo", 1, 1);
// REGISTER_PATCH(CrybabyPatch, "Faust/Crybaby", 1, 1);
// REGISTER_PATCH(StereoEchoPatch, "Faust/StereoEcho", 2, 2);
// REGISTER_PATCH(LowPassFilterPatch, "Faust/Low Pass Filter", 1, 1);
// REGISTER_PATCH(LowShelfPatch, "Faust/Low Shelf Filter", 1, 1);
// REGISTER_PATCH(HighShelfPatch, "Faust/High Shelf Filter", 1, 1);

// REGISTER_PATCH(GuitarixPhaserMonoPatch, "Guitarix/PhaserMono", 1, 1);

/* TO BE WORKED ON */
// REGISTER_PATCH(ConnyPatch, "Contest/ConnyPatch", 2, 2);
// REGISTER_PATCH(QompanderPatch, "Faust/Qompander", 1, 1); // 208%
// REGISTER_PATCH(SweepWahPatch, "Faust/SweepWah", 1, 1);
// REGISTER_PATCH(SimpleDriveDelayPatch, "Drive Delay", 1, 1);
// REGISTER_PATCH(AutotalentPatch, "AutoTalent", 2, 2);
// REGISTER_PATCH(EnvelopeFilterPatch, "Envelope Filter", 1, 1);
// REGISTER_PATCH(TemplatePatch, "Template", 0, 0);
// REGISTER_PATCH(JumpDelay, "Contest/JumpDelay", 0, 0);
// REGISTER_PATCH(SampleJitterPatch, "Contest/SampleJitter", 0, 0);
// REGISTER_PATCH(SirenPatch, "Contest/Siren", 0, 0);
// REGISTER_PATCH(LpfDelayPhaserPatch, "Low Pass Filtered Delay with Phaser", 1, 1);
// REGISTER_PATCH(WaveshaperPatch, "Waveshaper", 2, 2);
// REGISTER_PATCH(TestTonePatch, "Test Tone", 0, 0);
// REGISTER_PATCH(FlangerPatch, "Flanger", 0, 0);
