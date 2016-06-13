#include "bus.h"
#include "serial.h"
#include "MidiHandler.hpp"

/*
 * difference between USB midi and straight midi
 * blocks of 4, 3 bytes
 */

static MidiHandler bushandler;

void setupBus(){
}

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

class DigitalBusDiscovery {
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
public:
  DigitalBusDiscovery() : uid(0xff), downstream(0xff), token(0xffffffff), peers(0), parameterOffset(0) {}
  void sendMessage(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4){
    // send a 4-byte message
  }
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
    token = generateToken();
    sendDiscover(token);
  }
  void sendDiscover(uint32_t token){
    sendMessage(OWL_COMMAND_DISCOVER, (token>>16)&0x0f, (token>>8)&0x0f, token&0x0f);
  }
  void handleDiscover(uint32_t other){
    if(token == other){
      // we are UID 0
      startEnum();
    }else if(token < other){
      // we claim UID 0
      // sendDiscover(token);
      // sit and wait for our disc to come back
    }else{
      // we cede UID 0
      sendDiscover(other);
    }
  }
  void startEnum(){
    uid = 0;
    parameterOffset = 0;
    sendEnum(uid, VERSION, parameterOffset+PARAMETERS);
  }
  void sendEnum(uint8_t id, uint8_t version, uint16_t params){
    sendMessage(OWL_COMMAND_ENUM|id, version, params>>8, params & 0x0f);
  }
  void handleEnum(uint8_t id, uint8_t version, uint16_t params){
    if(uid == 0xff){
      // our UID has not been set yet
      // set it and pass on incremented value
      uid = id+1;
      parameterOffset = params;
      sendEnum(uid, VERSION, parameterOffset+PARAMETERS);
    }else if(uid == id){
      // we are talking to ourselves: ignore
    }else if(uid == 0){
      // id must be the number of peers on the bus
      peers = id;
      startIdent();
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
  void handleIdent(uint8_t id, uint8_t version, uint8_t device, uint8_t* uuid){
    if(id != uid && id != uid+1)
      sendIdent(id, version, device, uuid);
  }
};

extern "C" {
void USART_IRQHandler(void){
  static uint8_t index = 0;
  static uint8_t frame[4];
  /* If overrun condition occurs, clear the ORE flag and recover communication */
  if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET)
    USART_ReceiveData(USART_PERIPH);
  else if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){    
    // Reading the receive data register clears the RXNE flag implicitly
    char c = USART_ReceiveData(USART_PERIPH);
    // pass it on
    printByte(c);
    // printByte() blocks but presumably 
    // sending data will be as fast as receiving it
    frame[index++] = c;
    if(index == 4){
      index = 0;
      MidiReaderStatus status = 
	bushandler.readFrame(frame);
      if(status == ERROR_STATUS)
	bushandler.clear();
    }
  }
}
}
