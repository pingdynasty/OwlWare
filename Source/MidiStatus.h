#ifndef _MidiStatus_h_
#define _MidiStatus_h_

enum MidiStatus {
  STATUS_BYTE			= 0x80,
  NOTE_OFF			= 0x80,
  NOTE_ON			= 0x90,
  POLY_KEY_PRESSURE		= 0xA0,
  CONTROL_CHANGE		= 0xB0,
  PROGRAM_CHANGE		= 0xC0,
  CHANNEL_PRESSURE		= 0xD0,
  PITCH_BEND_CHANGE		= 0xE0,
  SYSTEM_COMMON			= 0xF0,
  SYSEX				= 0xF0,
  TIME_CODE_QUARTER_FRAME       = 0xF1,
  SONG_POSITION_PTR             = 0xF2,
  SONG_SELECT                   = 0xF3,
  RESERVED_F4                   = 0xF4,
  RESERVED_F5                   = 0xF5,
  TUNE_REQUEST                  = 0xF6,
  SYSEX_EOX                     = 0xF7,		
  SYSTEM_REAL_TIME		= 0xF8,
  TIMING_CLOCK		        = 0xF8,
  RESERVED_F9                   = 0xF9,
  START                         = 0xFA,
  CONTINUE                      = 0xFB,
  STOP                          = 0xFC,
  RESERVED_FD                   = 0xFD,
  ACTIVE_SENSING                = 0xFE,
  SYSTEM_RESET                  = 0xFF,
  MIDI_CHANNEL_MASK		= 0x0F,
  MIDI_STATUS_MASK		= 0xF0
};

enum MidiControlChange {
  MIDI_CC_MODULATION    = 0x01,
  MIDI_CC_BREATH        = 0x02,
  MIDI_CC_VOLUME        = 0x07,
  MIDI_CC_BALANCE       = 0x08,
  MIDI_CC_PAN           = 0x010,
  MIDI_CC_EXPRESSION    = 0x011,
  MIDI_CC_EFFECT_CTRL_1 = 0x012,
  MIDI_CC_EFFECT_CTRL_2 = 0x013,
};

enum UsbMidi {
  USB_COMMAND_MISC                = 0x00,	/* reserved */
  USB_COMMAND_CABLE_EVENT         = 0x01,	/* reserved */
  USB_COMMAND_2BYTE_SYSTEM_COMMON = 0x02,	/* e.g. MTC, SongSelect */
  USB_COMMAND_3BYTE_SYSTEM_COMMON = 0x03,	/* e.g. SPP */
  USB_COMMAND_SYSEX               = 0x04,
  USB_COMMAND_SYSEX_EOX1          = 0x05,
  USB_COMMAND_SYSEX_EOX2          = 0x06,
  USB_COMMAND_SYSEX_EOX3          = 0x07,
  USB_COMMAND_NOTE_OFF            = 0x08,
  USB_COMMAND_NOTE_ON             = 0x09,
  USB_COMMAND_POLY_KEY_PRESSURE   = 0x0A,
  USB_COMMAND_CONTROL_CHANGE	  = 0x0B,
  USB_COMMAND_PROGRAM_CHANGE	  = 0x0C,
  USB_COMMAND_CHANNEL_PRESSURE	  = 0x0D,
  USB_COMMAND_PITCH_BEND_CHANGE	  = 0x0E,
  USB_COMMAND_SINGLE_BYTE	  = 0x0F
};

#endif /* _MidiStatus_h_ */
