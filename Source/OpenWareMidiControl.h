#ifndef OPENWAREMIDICONTROL_H_INCLUDED
#define OPENWAREMIDICONTROL_H_INCLUDED

#define MIDI_SYSEX_MANUFACTURER        0x7d     /* Educational or development use only */
#define MIDI_SYSEX_DEVICE              0x52     /* OWL Open Ware Laboratory */
#define MIDI_SYSEX_VERSION             0x03     /* Revision */

enum PatchParameterId {
  PARAMETER_A,
  PARAMETER_B,
  PARAMETER_C,
  PARAMETER_D,
  PARAMETER_E,
  PARAMETER_F,
  PARAMETER_G,
  PARAMETER_H,

  PARAMETER_AA,
  PARAMETER_AB,
  PARAMETER_AC,
  PARAMETER_AD,
  PARAMETER_AE,
  PARAMETER_AF,
  PARAMETER_AG,
  PARAMETER_AH,

  PARAMETER_BA,
  PARAMETER_BB,
  PARAMETER_BC,
  PARAMETER_BD,
  PARAMETER_BE,
  PARAMETER_BF,
  PARAMETER_BG,
  PARAMETER_BH,

  // need to map parameters in a non-implementation-specific way
  // PARAMETER_AA
  // PARAMETER_AB...
  // PARAMETER_BA
  // PARAMETER_BB...
  /* PARAMETER_MIDI_MODULATION, // CC1 */
  /* PARAMETER_MIDI_BREATH,     // CC2 */
  /* PARAMETER_MIDI_VOLUME,     // CC7 */
  /* PARAMETER_MIDI_BALANCE,    // CC8 */
  /* PARAMETER_MIDI_PAN,        // CC10 */
  /* PARAMETER_MIDI_EXPRESSION, // CC11 */
  /* PARAMETER_MIDI_EFFECT_CTRL_1,    // CC12 */
  /* PARAMETER_MIDI_EFFECT_CTRL_2,    // CC13 */
  /* PARAMETER_MIDI_PITCH, */
  /* PARAMETER_MIDI_AMPLITUDE, */

  /*
  // CC16-19 General Purpose Controllers 1-4
  // Sound controller 1 to 10, CC70 to CC79
  PARAMETER_MIDI_CC70, // Sound Variation
  PARAMETER_MIDI_CC71, // Timbre / Harmonic Content
  PARAMETER_MIDI_CC72, // Release
  PARAMETER_MIDI_CC73, // Attack
  PARAMETER_MIDI_CC74, // Brightness
  PARAMETER_MIDI_CC75,
  PARAMETER_MIDI_CC76,
  PARAMETER_MIDI_CC77,
  PARAMETER_MIDI_CC78,
  PARAMETER_MIDI_CC79,
  // CC80-83 General Purpose Controllers 5-8
  // generic on/off switch CC80 to CC83
  // Effect 1 depth CC91 Reverb
  // Effect 2 depth CC92 Tremolo
  // Effect 3 depth CC93 Chorus
  // Effect 4 depth CC94 Detune
  // Effect 5 depth CC95 Phaser
  */
  PARAMETER_MIDI_NOTE = 0x80 // values over 127 are mapped to note numbers
};

enum PatchButtonId {
  BYPASS_BUTTON,
  PUSHBUTTON,
  GREEN_BUTTON,
  RED_BUTTON,
  MIDI_GATE_BUTTON,
  MIDI_NOTE_BUTTON = 0x80 // values over 127 are mapped to note numbers
};

#define SYSEX_CONFIGURATION_AUDIO_RATE            "FS"
#define SYSEX_CONFIGURATION_AUDIO_BITDEPTH        "BD"
#define SYSEX_CONFIGURATION_AUDIO_DATAFORMAT      "DF"
#define SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE       "BS"
#define SYSEX_CONFIGURATION_CODEC_PROTOCOL        "PT"
#define SYSEX_CONFIGURATION_CODEC_MASTER          "MS"
#define SYSEX_CONFIGURATION_CODEC_SWAP            "SW"
#define SYSEX_CONFIGURATION_CODEC_BYPASS          "BY"
#define SYSEX_CONFIGURATION_CODEC_HALFSPEED       "HS"
#define SYSEX_CONFIGURATION_PC_BUTTON             "PC"
#define SYSEX_CONFIGURATION_INPUT_OFFSET          "IO"
#define SYSEX_CONFIGURATION_INPUT_SCALAR          "IS"
#define SYSEX_CONFIGURATION_OUTPUT_OFFSET         "OO"
#define SYSEX_CONFIGURATION_OUTPUT_SCALAR         "OS"

enum OpenWareMidiSysexCommand {
  SYSEX_PRESET_NAME_COMMAND       = 0x01,
  SYSEX_PARAMETER_NAME_COMMAND    = 0x02,
  SYSEX_CONFIGURATION_COMMAND     = 0x03,
  SYSEX_DFU_COMMAND               = 0x7e,
  SYSEX_FIRMWARE_UPLOAD           = 0x10,
  SYSEX_FIRMWARE_STORE            = 0x11,
  SYSEX_FIRMWARE_RUN              = 0x12,
  SYSEX_FIRMWARE_FLASH            = 0x13,
  SYSEX_FIRMWARE_VERSION          = 0x20,
  SYSEX_DEVICE_ID                 = 0x21,
  SYSEX_PROGRAM_MESSAGE           = 0x22,
  SYSEX_DEVICE_STATS              = 0x23,
  SYSEX_PROGRAM_STATS             = 0x24
};

/*
 MIDI Control Change Mappings
*/
enum OpenWareMidiControl {
  PATCH_PARAMETER_A      = 20, /* Parameter A */
  PATCH_PARAMETER_B      = 21, /* Parameter B */
  PATCH_PARAMETER_C      = 22, /* Parameter C */
  PATCH_PARAMETER_D      = 23, /* Parameter D */
  PATCH_PARAMETER_E      = 24, /* Expression pedal / input */
  PATCH_PARAMETER_F      = 1,  /* Extended parameter Modulation CC */
  PATCH_PARAMETER_G      = 12, /* Extended parameter Effect Ctrl 1 CC */
  PATCH_PARAMETER_H      = 13, /* Extended parameter Effect Ctrl 2 CC */

  PATCH_BUTTON           = 25, /* LED Pushbutton: 0=not pressed, 127=pressed */
  PATCH_CONTROL          = 26, /* Remote control: 0=local, 127=MIDI */
  LED                    = 30, /* set/get LED value: 
				* 0-41 = off
				* 42-83 = green
				* 84-127 = red 
				*/
  LEFT_INPUT_GAIN        = 32, /* left channel input gain, -34.5dB to +12dB (92 = 0dB) */
  RIGHT_INPUT_GAIN       = 33,
  LEFT_OUTPUT_GAIN       = 34, /* left channel output gain, -73dB to +6dB (121 = 0dB) */
  RIGHT_OUTPUT_GAIN      = 35,
  LEFT_INPUT_MUTE        = 36, /* mute left input (127=muted) */
  RIGHT_INPUT_MUTE       = 37,
  LEFT_OUTPUT_MUTE       = 38, /* mute left output (127=muted) */
  RIGHT_OUTPUT_MUTE      = 39,
  BYPASS                 = 40, /* codec bypass mode (127=bypass) */
  REQUEST_SETTINGS       = 67, /* load settings from device (127=all settings) (30 for LED) (more to come) */
  SAVE_SETTINGS          = 68, /* save settings to device */
  FACTORY_RESET          = 70, /* reset all settings */
  DEVICE_STATUS          = 71,

  /* PATCH_PARAMETER_F      = 72, */
  /* PATCH_PARAMETER_G      = 73, */
  /* PATCH_PARAMETER_H      = 74, */
  PATCH_PARAMETER_AA     = 75,
  PATCH_PARAMETER_AB     = 76,
  PATCH_PARAMETER_AC     = 77,
  PATCH_PARAMETER_AD     = 78,
  PATCH_PARAMETER_AE     = 79,
  PATCH_PARAMETER_AF     = 80,
  PATCH_PARAMETER_AG     = 81,
  PATCH_PARAMETER_AH     = 82,
  PATCH_PARAMETER_BA     = 83,
  PATCH_PARAMETER_BB     = 84,
  PATCH_PARAMETER_BC     = 85,
  PATCH_PARAMETER_BD     = 86,
  PATCH_PARAMETER_BE     = 87,
  PATCH_PARAMETER_BF     = 88,
  PATCH_PARAMETER_BG     = 89,
  PATCH_PARAMETER_BH     = 90
};

#endif  // OPENWAREMIDICONTROL_H_INCLUDED
