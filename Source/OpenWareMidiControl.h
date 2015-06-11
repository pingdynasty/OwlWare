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
  PARAMETER_F
};

enum PatchButtonId {
  BYPASS_BUTTON,
  PUSHBUTTON,
  GREEN_BUTTON,
  RED_BUTTON
};

#define SYSEX_CONFIGURATION_AUDIO_RATE            "FS"
#define SYSEX_CONFIGURATION_AUDIO_WIDTH           "WH"
#define SYSEX_CONFIGURATION_AUDIO_BLOCKSIZE       "BS"
#define SYSEX_CONFIGURATION_CODEC_PROTOCOL        "PT"
#define SYSEX_CONFIGURATION_CODEC_MASTER          "MS"
#define SYSEX_CONFIGURATION_CODEC_SWAP            "SW"
#define SYSEX_CONFIGURATION_CODEC_BYPASS          "BY"
#define SYSEX_CONFIGURATION_CODEC_HALFSPEED       "HS"

enum OpenWareMidiSysexCommand {
  SYSEX_PRESET_NAME_COMMAND       = 0x01,
  SYSEX_PARAMETER_NAME_COMMAND    = 0x02,
  SYSEX_CONFIGURATION_COMMAND     = 0x03,
  SYSEX_DFU_COMMAND               = 0x7e,
  SYSEX_FIRMWARE_UPLOAD           = 0x10,
  SYSEX_FIRMWARE_STORE            = 0x11,
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

  CODEC_MASTER           = 41, /* codec mode, slave or master:
				* 0-63 = slave
				* 64-127 = master
				*/
  CODEC_PROTOCOL         = 42, /* codec protocol: 
				* 0-63 = I2S Philips
				* 64-127 = MSB
				*/
  SAMPLING_RATE          = 60, /* sampling rate
				* 0-31 = 8kHz
				* 32-63 = 32kHz
				* 64-95 = 48kHz
				* 96-127 = 96kHz
				*/
  SAMPLING_BITS          = 61, /* sampling bits
				* 0-41 = 16bit
				* 42-83 = 24bit
				* 84-127 = 32bit
				*/
  SAMPLING_SIZE          = 62, /* block size in samples */

  LEFT_RIGHT_SWAP        = 63, /* swap left/right channels */

  REQUEST_SETTINGS       = 67, /* load settings from device (127=all settings) (30 for LED) (more to come) */
  SAVE_SETTINGS          = 68, /* save settings to device */
  DEVICE_FIRMWARE_UPDATE = 69, /* enter Device Firmware Upgrade mode */
  FACTORY_RESET          = 70, /* reset all settings */
  DEVICE_STATUS          = 71
};

#endif  // OPENWAREMIDICONTROL_H_INCLUDED
