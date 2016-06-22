/*
discovery process:
- wait
- to initiate: generate 24bit token (from UUID or random) and send disc: (0x10 | 0) token

- if receive disc with different token:
    if token < mine accept defeat and pass it on
    otherwise resend disc

- if receive disc (that I didn't send) pass it on as is

- when receive disc that I sent, claim UID 0 and start enum process.

enumeration process:
- to initiate, send ENUM with UID=0
- on receipt (and UID not set), claim UID+1 and send ENUM with UID+1
- downstream UID = local UID+1 (or 0)
- on receipt with UID already set, restart discovery process

identification process:
(optional)
- send 0x30 | UID with UUID (in 6 messages)
- on receipt with UID != mine and UID != upstream, pass along.

communication process:
- send midi, parameter, or data with UID
- on receipt with UID != mine and UID != upstream, pass along.

DISCOVER: 0x10, 24bit random ID
ENUM:     0x20|UID, 8bit version, 16bit parameters
IDENT:    0x30|UID, VERSION, PRODUCT, UUID 8bit
IDENT:    0x30|UID, UUID 24bit (x5) (out of sequence messages possible?)

- what to do with USB MIDI messages? no room for UID?

- todo: enumerate parameter id's

*/

class BusHandler {
public:
  virtual void handleSystemCommon(uint8_t){}
  virtual void handleSystemCommon(uint8_t, uint8_t){}
  virtual void handleProgramChange(uint8_t, uint8_t){}
  virtual void handleChannelPressure(uint8_t, uint8_t){}
  virtual void handlePolyKeyPressure(uint8_t, uint8_t, uint8_t){}
  virtual void handleControlChange(uint8_t, uint8_t, uint8_t){}
  virtual void handleNoteOff(uint8_t, uint8_t, uint8_t){}
  virtual void handleNoteOn(uint8_t, uint8_t, uint8_t){}
  virtual void handlePitchBend(uint8_t, uint16_t){}
  virtual void handleSysEx(uint8_t* data, uint16_t size){}
  virtual void handleParameterChange(uint8_t pid, uint16_t value){}
};

class DigitalBus {
private:
  uint8_t uid;
  uint8_t downstream;
  uint32_t token;
  uint8_t peers;
  uint16_t parameterOffset;
  uint8_t UUID[16];
  static const uint8_t VERSION = 0x01; // protocol version
  static const uint8_t PRODUCT = 0x01;  // product id
  static const uint8_t PARAMETERS = 5; // number of parameters defined by this product
  static const uint8_t NO_UID = 0xff;
  static const uint32_t NO_TOKEN = 0xffffffff;
public:
  DigitalBusDiscovery() : uid(NO_UID), downstream(NO_UID), token(NO_TOKEN), peers(0), parameterOffset(0) {}

  // send a 4-byte message
  void sendMessage(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);
  void writeFrame(uint8_t* frame);

  uint32_t generateToken(){
    uint32_t tok = (VERSION << 16) | (UUID[15] << 8) | UUID[14];
    tok ^= (UUID[13] << 16) | (UUID[12] << 8) | UUID[11];
    tok ^= (UUID[10] << 16) | (UUID[9] << 8) | UUID[8];
    tok ^= (UUID[7] << 16) | (UUID[6] << 8) | UUID[5];
    tok ^= (UUID[4] << 16) | (UUID[3] << 8) | UUID[2];
    tok ^= (UUID[1] << 16) | (UUID[0] << 8) | PRODUCT;
    return tok;
  }
  void startDiscover(){
    // while(token == NO_TOKEN) // todo: what if generated token == NO_TOKEN?
    token = generateToken();
    uid = 0; // start by assuming we will be UID 0
    sendDiscover(token);    
    // on sending discover, assume we are UID 0. if we receive a token < ours, set UID != 0.
  }
  void sendDiscover(uint8_t seq, uint32_t token){
    sendMessage(OWL_COMMAND_DISCOVER, (token>>16)&0x0f, (token>>8)&0x0f, token&0x0f);
  }
  void handleDiscover(uint8_t seq, uint32_t other){
    // on receipt of other token, add +1 to seq and pass it on, but send own token first.
    // once we get our own token back, the seq tells us how many peers there are.
    // lowest token then takes precedence.
    if(token == NO_TOKEN)
      startDiscover();
    if(other == token){
      // that's our token.
      peers = seq;
      if(uid == 0)
	startEnum(); // this will push out more messages before the DISCOVERs have finished coming through
    }else if(other < token){
      uid = NO_UID; // we will not be UID 0
      sendDiscover(seq++, token);
    }else{
      sendDiscover(seq++, token);
    }
  }
  void startEnum(){
    parameterOffset = 0;
    sendEnum(uid, VERSION, PRODUCT, parameterOffset+PARAMETERS);
  }
  void sendEnum(uint8_t id, uint8_t version, uint8_t product, uint8_t params){
    sendMessage(OWL_COMMAND_ENUM|id, version, params>>8, params & 0x0f);
  }
  void handleEnum(uint8_t id, uint8_t version, uint8_t product, uint8_t params){
    if(uid == NO_UID){
      // our UID has not been set yet
      // set it and pass on incremented value
      uid = id+1;
      parameterOffset = params;
      sendEnum(uid, VERSION, PRODUCT, parameterOffset+PARAMETERS);
      // note that only one ENUM should be received as they are not propagated.
      // downstream UID will be (uid+1 > peers) ? 0 : uid+1
    }else if(uid == 0){
      startIdent();
    }else if(uid == id){
      // we are talking to ourselves: ignore
    }else{
      // something must have gone wrong, we already have a UID
      // but this might be the second round?
    }
  }
  void startIdent(){
    sendIdent(uid, VERSION, PRODUCT, UUID);
  }
  void sendIdent(uint8_t id, uint8_t version, uint8_t device, uint8_t* uuid){
    sendMessage(OWL_COMMAND_IDENT|uid, VERSION, PRODUCT, uuid[15]);
    sendMessage(OWL_COMMAND_IDENT|uid, uuid[14], uuid[13], uuid[12]);
    sendMessage(OWL_COMMAND_IDENT|uid, uuid[11], uuid[10], uuid[9]);
    sendMessage(OWL_COMMAND_IDENT|uid, uuid[8], uuid[7], uuid[6]);
    sendMessage(OWL_COMMAND_IDENT|uid, uuid[5], uuid[4], uuid[3]);
    sendMessage(OWL_COMMAND_IDENT|uid, uuid[2], uuid[1], uuid[0]);
  }
  /* void handleIdent(uint8_t id, uint8_t version, uint8_t device, uint8_t* uuid){ */
  /*   if(id != uid && id != uid+1) */
  /*     sendIdent(id, version, device, uuid); // pass it on */
  /* } */
  void handleIdent(uint8_t id, uint8_t d1, uint8_t d2, uint8_t d3){
    if(id != uid && id != uid+1)
      sendIdent(id, d1, d2, d3); // pass it on
  }

  void sendParameterChange(uint8_t pid, uint16_t value){
    sendMessage(OWL_COMMAND_PARAM|uid, pid, value>>8, value&0x0f);
  }

  void handleParameterChange(uint8_t pid, uint16_t value){
    setParameter(pid, value);
  }

  // read a 4-byte data frame
  void readFrame(uint8_t* frame){
    switch(frame[0]){
    case USB_COMMAND_MISC:
    case USB_COMMAND_CABLE_EVENT:
      // ignore
      break;
    case USB_COMMAND_SINGLE_BYTE:
      handleSystemCommon(frame[1]);
      break;
    case USB_COMMAND_SYSEX_EOX1:
      handleSysex(&frame[1], 1);
      break;
    case USB_COMMAND_2BYTE_SYSTEM_COMMON:
      handleSystemCommon(frame[1], frame[2]);
      break;
    case USB_COMMAND_SYSEX_EOX2:
      handleSysex(&frame[1], 2);
      break;
    case USB_COMMAND_PROGRAM_CHANGE:
      handleProgramChange(frame[1], frame[2]);
      break;
    case USB_COMMAND_CHANNEL_PRESSURE:
      handleChannelPressure(frame[1], frame[2]);
      break;
    case USB_COMMAND_NOTE_OFF:
      handleNoteOff(frame[1], frame[2], frame[3]);
      break;
    case USB_COMMAND_NOTE_ON:
      if(frame[3] == 0)
	handleNoteOff(frame[1], frame[2], frame[3]);
      else
	handleNoteOn(frame[1], frame[2], frame[3]);
      break;
    case USB_COMMAND_POLY_KEY_PRESSURE:
      handlePolyKeyPressure(frame[1], frame[2], frame[3]);
      break;
    case USB_COMMAND_CONTROL_CHANGE:
      handleControlChange(frame[1], frame[2], frame[3]);
      break;
    case USB_COMMAND_PITCH_BEND_CHANGE:
      handlePitchBend(frame[1], frame[2] | (frame[3]<<7));
      break;
    default:
      switch(frame[0] & 0xf0){
      case OWL_COMMAND_DISCOVER:
	handleDiscover(frame[0]&0x0f, (frame[1] << 16) | (frame[2]<<8) | frame[3]);
	break;
      case OWL_COMMAND_ENUM:
	handleEnum(frame[0]&0x0f, frame[1], (frame[2] << 8) | frame[3]);
	break;
      case OWL_COMMAND_IDENT:
	handleIdent(frame[0]&0x0f, frame[1], frame[2], frame[3]);
	break;
      case OWL_COMMAND_PARAMETER:
	if((frame[0]&0x0f) != uid){
	  // it's not from us
	  handleParameterChange(frame[1], (frame[2]<<8) | frame[3]);
	  // propagate
	  sendMessage(frame[0], frame[1], frame[2], frame[3]);
	}
	break;
      case OWL_COMMAND_DATA:
	// OSC, firmware or file data
	// 0x30, type, sizeH, sizeL
	// uint16_t size = (frame[2]<<8) | frame[3]);
	break;
      case OWL_COMMAND_SYNC:
	// 0xc0 until 0xff at end of frame
	// use ASCII SYN instead?
	break;
      }
      // if((frame[0] & 0x0f) != uid && (frame[0] & 0x0f) != (uid+1)){
      // 	// we are not the originator
      // 	// forward message
      // }
      // ignore
    }
  }
};
