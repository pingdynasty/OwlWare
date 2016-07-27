#include "DigitalBusHandler.h"
#include "MidiStatus.h"
#include "serial.h"
#include "bus.h"
#include <string.h>

DigitalBusHandler::DigitalBusHandler() 
  : uid(0), nuid(NO_UID), token(NO_TOKEN), peers(0), 
    parameterOffset(0), status(IDLE) {
  UUID = (uint8_t*)bus_deviceid();
  token = generateToken();
}

void DigitalBusHandler::sendFrame(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4){
  uint8_t buf[4] = {d1, d2, d3, d4};
  sendFrame(buf);
}

void DigitalBusHandler::sendFrame(uint8_t* frame){
  serial_write(frame, 4);
}

uint32_t DigitalBusHandler::generateToken(){
  uint32_t tok = (VERSION << 16) | UUID[11];
  tok ^= (UUID[10] << 16) | (UUID[9] << 8) | UUID[8];
  tok ^= (UUID[7] << 16) | (UUID[6] << 8) | UUID[5];
  tok ^= (UUID[4] << 16) | (UUID[3] << 8) | UUID[2];
  tok ^= (UUID[1] << 16) | (UUID[0] << 8) | PRODUCT;
  return tok;
}

bool DigitalBusHandler::connected(){
  switch(status){
  case IDLE:
  case ERROR:
  case DISCOVER:
    startDiscover();
    break;
  case ENUMERATE:
    startEnum();
    break;
  case IDENTIFY:
    startIdent();
    break;
  case CONNECTED:
    break;
  }
  return status == CONNECTED;
}

void DigitalBusHandler::rxError(const char* reason){
  status = ERROR;
  bus_rx_error(reason);
}

void DigitalBusHandler::txError(const char* reason){
  status = ERROR;
  bus_tx_error(reason);
}

void DigitalBusHandler::startDiscover(){
  status = DISCOVER;
  sendDiscover(0, token);
}

void DigitalBusHandler::sendDiscover(uint8_t seq, uint32_t token){
  sendFrame(OWL_COMMAND_DISCOVER|seq, token>>16, token>>8, token);
}

void DigitalBusHandler::handleDiscover(uint8_t seq, uint32_t other){
  // on receipt of other token, add +1 to seq and pass it on, then send own token.
  // once we get our own token back, the seq tells us how many peers there are.
  // lowest token then takes precedence.
  if(other == token){
    // that's our token.
    peers = seq;
    status = ENUMERATE;
  }else{
    if(seq < 0x0f)
      // increment seq and pass it on
      sendDiscover(seq+1, other);
    if(other < token)
      uid = NO_UID; // we will not be UID 0
    if(status != ENUMERATE)
      status = DISCOVER;
    if(peers == 0)
      startDiscover();
    // if(peers != 0 && seq == peers-1 && uid == 0)
    //   // this should be the last of the disco msgs and we are uid 0: start enum
    //   startEnum();
  }
}

void DigitalBusHandler::startEnum(){
  if(uid == 0){
    parameterOffset = 0;
    sendEnum(uid, VERSION, PRODUCT, parameterOffset+PARAMETERS);
  }
}

void DigitalBusHandler::sendEnum(uint8_t id, uint8_t version, uint8_t product, uint8_t params){
  sendFrame(OWL_COMMAND_ENUM|id, version, product, params);
}

void DigitalBusHandler::handleEnum(uint8_t id, uint8_t version, uint8_t product, uint8_t params){
  if(uid == NO_UID){
    // our UID has not been set yet
    // set it and pass on incremented value
    uid = id+1;
    parameterOffset = params;
    sendEnum(uid, VERSION, PRODUCT, parameterOffset+PARAMETERS);
    // note that only one ENUM should be received as they are not propagated.
    // downstream UID will be (uid+1 > peers) ? 0 : uid+1
  }
  if(nuid == NO_UID){
    nuid = uid+1;
    if(nuid > peers)
      nuid = 0;
  }
  status = IDENTIFY;
}

void DigitalBusHandler::startIdent(){
  sendIdent(uid, VERSION, PRODUCT, UUID);
  status = CONNECTED;
}

void DigitalBusHandler::sendIdent(uint8_t id, uint8_t version, uint8_t device, uint8_t* uuid){
  sendFrame(OWL_COMMAND_IDENT|uid, VERSION, PRODUCT, uuid[15]);
  sendFrame(OWL_COMMAND_IDENT|uid, uuid[14], uuid[13], uuid[12]);
  sendFrame(OWL_COMMAND_IDENT|uid, uuid[11], uuid[10], uuid[9]);
  sendFrame(OWL_COMMAND_IDENT|uid, uuid[8], uuid[7], uuid[6]);
  sendFrame(OWL_COMMAND_IDENT|uid, uuid[5], uuid[4], uuid[3]);
  sendFrame(OWL_COMMAND_IDENT|uid, uuid[2], uuid[1], uuid[0]);
}

/* void handleIdent(uint8_t id, uint8_t version, uint8_t device, uint8_t* uuid){ */
/*   if(id != uid && id != nuid) */
/*     sendIdent(id, version, device, uuid); // pass it on */
/* } */

void DigitalBusHandler::handleIdent(uint8_t id, uint8_t d1, uint8_t d2, uint8_t d3){
  // todo: need to wait for full set of 6 messages and buffer UUID?
  // no because uid is contained in every message
  // propagation done by DigitalBusReader
}

void DigitalBusHandler::sendParameterChange(uint8_t pid, int16_t value){
  sendFrame(OWL_COMMAND_PARAMETER|uid, pid, value>>8, value);
}

void DigitalBusHandler::handleParameterChange(uint8_t pid, int16_t value){
  bus_rx_parameter(pid, value);
  // todo
  // setParameterValue(pid, value);  
}

void DigitalBusHandler::sendButtonChange(uint8_t bid, int16_t value){
  sendFrame(OWL_COMMAND_BUTTON|uid, bid, value>>8, value);
}

void DigitalBusHandler::handleButtonChange(uint8_t bid, int16_t value){
  bus_rx_button(bid, value);
}

void DigitalBusHandler::sendCommand(uint8_t cmd, int16_t data){
  sendFrame(OWL_COMMAND_COMMAND|uid, cmd, data>>8, data);
}

void DigitalBusHandler::handleCommand(uint8_t cmd, int16_t data){
  bus_rx_command(cmd, data);
}

void DigitalBusHandler::sendMessage(const char* msg){
  uint16_t len = strnlen(msg, sizeof(buffer));
  uint16_t cnt = len/3;
  while(cnt--){
    sendFrame(OWL_COMMAND_MESSAGE|uid, msg[0], msg[1], msg[2]);
    msg += 3;
  }
  switch(len%3){
  case 0:
    if(*(msg-1) != '\0')
      sendFrame(OWL_COMMAND_MESSAGE|uid, '\0', '\0', '\0');
    break;
  case 1:
    sendFrame(OWL_COMMAND_MESSAGE|uid, msg[0], '\0', '\0');
    break;
  case 2:
    sendFrame(OWL_COMMAND_MESSAGE|uid, msg[0], msg[1], '\0');
    break;
  }
}

/* Received 3 bytes of string message */
void DigitalBusHandler::handleMessage(const char* str){
  bus_rx_message(str);
}

void DigitalBusHandler::sendData(const uint8_t* data, uint32_t len){
  sendFrame(OWL_COMMAND_DATA|uid, len>>16, len>>8, len);
  uint16_t cnt = len/3;
  while(cnt--){
    sendFrame(OWL_COMMAND_DATA|uid, data[0], data[1], data[2]);
    data += 3;
  }
  switch(len%3){
  case 0:
    break;
  case 1:
    sendFrame(OWL_COMMAND_DATA|uid, data[0], '\0', '\0');
    break;
  case 2:
    sendFrame(OWL_COMMAND_DATA|uid, data[0], data[1], '\0');
    break;
  }
}

void DigitalBusHandler::handleData(const uint8_t* data, uint32_t len){
  bus_rx_data(data, len);
}
