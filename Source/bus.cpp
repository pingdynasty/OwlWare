#include "bus.h"
#include "serial.h"
#include "MidiHandler.hpp"

static MidiHandler bushandler;

void setupBus(){
}

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
      // skip first of four bytes
      MidiReaderStatus status;
      switch(frame[0]){
      case USB_COMMAND_MISC:
      case USB_COMMAND_CABLE_EVENT:
	// ignore
	break;
      case USB_COMMAND_SINGLE_BYTE:
      case USB_COMMAND_SYSEX_EOX1:
	// one byte message
	status = bushandler.read(frame[1]);
	break;
      case USB_COMMAND_2BYTE_SYSTEM_COMMON:
      case USB_COMMAND_SYSEX_EOX2:
      case USB_COMMAND_PROGRAM_CHANGE:
      case USB_COMMAND_CHANNEL_PRESSURE:
	bushandler.read(frame[1]);
	status = bushandler.read(frame[2]);
	break;
      default:
	// three byte message
	bushandler.read(frame[1]);
	bushandler.read(frame[2]);
	status = bushandler.read(frame[3]);
	break;
      }
      if(status == ERROR_STATUS)
	bushandler.clear();
    }
  }
}
}
