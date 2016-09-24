#include "DigitalBusHandler.h"
#include "MidiStatus.h"
#include "serial.h"
#include "bus.h"
#include <string.h>

DigitalBusHandler::DigitalBusHandler() 
  : // uid(0), nuid(NO_UID), 
    token(NO_TOKEN), peers(0), 
    parameterOffset(0), status(IDLE) {
  UUID = (uint8_t*)bus_deviceid();
  token = generateToken();
}

void DigitalBusHandler::sendFrame(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4){
  uint8_t buf[4] = {d1, d2, d3, d4};
  sendFrame(buf);
}

void DigitalBusHandler::sendFrame(uint8_t* frame){
  // if(isMidiFrame(frame)) // todo: remove
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
  // case ENUMERATE:
  //   startEnum();
  //   break;
  // case IDENTIFY:
  //   startIdent();
  //   break;
  case CONNECTED:
    break;
  }
  return status == CONNECTED;
}

bool DigitalBusHandler::rxError(const char* reason){
  status = ERROR;
  bus_rx_error(reason);
  return false;
}

bool DigitalBusHandler::txError(const char* reason){
  status = ERROR;
  bus_tx_error(reason);
  return false;
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
    status = CONNECTED;
  }else{
    if(seq < 0x0f)
      // increment seq and pass it on
      sendDiscover(seq+1, other);
    if(peers == 0)
      startDiscover();
  }
}

void DigitalBusHandler::sendParameterChange(uint8_t pid, int16_t value){
  sendFrame(OWL_COMMAND_PARAMETER|peers, pid, value>>8, value);
}

void DigitalBusHandler::handleParameterChange(uint8_t pid, int16_t value){
  bus_rx_parameter(pid, value);
  // todo
  // setParameterValue(pid, value);  
}

void DigitalBusHandler::sendCommand(uint8_t cmd, int16_t data){
  sendFrame(OWL_COMMAND_COMMAND|peers, cmd, data>>8, data);
}

void DigitalBusHandler::handleCommand(uint8_t cmd, int16_t data){
  bus_rx_command(cmd, data);
}

void DigitalBusHandler::sendMessage(const char* msg){
  uint16_t len = strnlen(msg, sizeof(buffer));
  uint16_t cnt = len/3;
  while(cnt--){
    sendFrame(OWL_COMMAND_MESSAGE|peers, msg[0], msg[1], msg[2]);
    msg += 3;
  }
  switch(len%3){
  case 0:
    if(*(msg-1) != '\0')
      sendFrame(OWL_COMMAND_MESSAGE|peers, '\0', '\0', '\0');
    break;
  case 1:
    sendFrame(OWL_COMMAND_MESSAGE|peers, msg[0], '\0', '\0');
    break;
  case 2:
    sendFrame(OWL_COMMAND_MESSAGE|peers, msg[0], msg[1], '\0');
    break;
  }
}

/* Received 3 bytes of string message */
void DigitalBusHandler::handleMessage(const char* str){
  bus_rx_message(str);
}

void DigitalBusHandler::sendData(const uint8_t* data, uint32_t len){
  sendFrame(OWL_COMMAND_DATA|peers, len>>16, len>>8, len);
  uint16_t cnt = len/3;
  while(cnt--){
    sendFrame(OWL_COMMAND_DATA|peers, data[0], data[1], data[2]);
    data += 3;
  }
  switch(len%3){
  case 0:
    break;
  case 1:
    sendFrame(OWL_COMMAND_DATA|peers, data[0], '\0', '\0');
    break;
  case 2:
    sendFrame(OWL_COMMAND_DATA|peers, data[0], data[1], '\0');
    break;
  }
}

void DigitalBusHandler::handleData(const uint8_t* data, uint32_t len){
  bus_rx_data(data, len);
}

void DigitalBusHandler::sendReset(){
  sendFrame(OWL_COMMAND_RESET, OWL_COMMAND_RESET, 
	    OWL_COMMAND_RESET, OWL_COMMAND_RESET);
}

bool DigitalBusHandler::isMidiFrame(uint8_t* frame){
  return frame[0] <= USB_COMMAND_SINGLE_BYTE && frame[0] > USB_COMMAND_CABLE_EVENT;
  // return (frame[0] & 0xf0) == 0 && 
  //   (frame[0] & 0x0f) > USB_COMMAND_CABLE_EVENT &&
  //   (frame[1] & 0xf0)  >= NOTE_ON;
}
