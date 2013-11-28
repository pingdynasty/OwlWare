#ifndef OPENWAREMIDICONTROL_H_INCLUDED
#define OPENWAREMIDICONTROL_H_INCLUDED

#define MIDI_SYSEX_MANUFACTURER        0x7d     /* Educational or development use only */
#define MIDI_SYSEX_DEVICE              0x52     /* OWL Open Ware Laboratory */
#define MIDI_SYSEX_VERSION             0x03     /* Revision */
#define MIDI_MAX_MESSAGE_SIZE          0x0f

enum OpenWareMidiSysexCommand {
  SYSEX_PRESET_NAME_COMMAND     = 0x01,
  SYSEX_DFU_COMMAND             = 0x7e
};

/*
 MIDI Control Change Mappings
*/
enum OpenWareMidiControl {
  PATCH_PARAMETER_A      = 20, /*  */
  PATCH_PARAMETER_B      = 21, /*  */
  PATCH_PARAMETER_C      = 22, /*  */
  PATCH_PARAMETER_D      = 23, /*  */
  PATCH_PARAMETER_E      = 24, /*  */

  PATCH_SLOT_A           = 28, /* load patch into slot A */
  PATCH_SLOT_B           = 29, /* load patch into slot B */

  LED                    = 30, /* set/get LED value: 
				  0-41 = off
				  42-83 = green
				  84-127 = red */
  ACTIVE_PATCH           = 31, /* currently active patch, 0-number of available patches */

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
				  0-63 = slave
				  64-127 = master
			       */
  CODEC_PROTOCOL         = 42, /* codec protocol: 
				  0-63 = I2S Philips
				  64-127 = MSB
			       */
  SAMPLING_RATE          = 60, /* sampling rate
				  0-31 = 8kHz
				  32-63 = 32kHz
				  64-95 = 48kHz
				  96-127 = 96kHz
			       */
  SAMPLING_BITS          = 61, /* sampling bits
				  0-41 = 16bit
				  42-83 = 24bit
				  84-127 = 32bit
			       */
  SAMPLING_SIZE          = 62, /* block size in samples */

  LEFT_RIGHT_SWAP        = 63, /* swap left/right channels */

  REQUEST_SETTINGS       = 67, /* load settings from device */
  SAVE_SETTINGS          = 68, /* save settings to device */
  DEVICE_FIRMWARE_UPDATE = 69, /* enter Device Firmware Upgrade mode */
  FACTORY_RESET          = 70  /* reset all settings */
};

#endif  // OPENWAREMIDICONTROL_H_INCLUDED
