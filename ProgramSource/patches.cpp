/* Help:
 REGISTER_PATCH(yourHppFileName,
                "NameThatWillBeDisplayed",
                numberInputChannels,
                numberOutputChannels);
 
 The first two registered patch are the default green and red patches in compiled the firmware.
 
*/

REGISTER_PATCH(GainPatch, "Gain", 2, 2);
REGISTER_PATCH(GainPatch, "Moar Gain", 2, 2);



/*
TO BE WORKED ON
*/
// REGISTER_PATCH(JotReverbPatch,"JotReverbPatch", 2, 2);
// REGISTER_PATCH(SimpleDriveDelayPatch, "Drive Delay", 1, 1);
// REGISTER_PATCH(AutotalentPatch, "AutoTalent", 2, 2);
// REGISTER_PATCH(EnvelopeFilterPatch, "Envelope Filter", 1, 1);
// REGISTER_PATCH(TemplatePatch, "Template", 0, 0);
// REGISTER_PATCH(JumpDelay, "Contest/JumpDelay", 0, 0);
// REGISTER_PATCH(SampleJitterPatch, "Contest/SampleJitterPatch", 0, 0);
// REGISTER_PATCH(SirenPatch, "Contest/SirenPatch", 0, 0);
// REGISTER_PATCH(LpfDelayPatch, "Low Pass Filtered Delay", 1, 1);
// REGISTER_PATCH(LpfDelayPhaserPatch, "Low Pass Filtered Delay with Phaser", 1, 1);
// REGISTER_PATCH(WaveshaperPatch, "Waveshaper", 2, 2);
// REGISTER_PATCH(TestTonePatch, "Test Tone", 0, 0);
// REGISTER_PATCH(FlangerPatch, "Flanger", 0, 0);
