#include "bus.h"
#include "owlcontrol.h"
#include "Owl.h"
#include "DigitalBusReader.h"

/*
 * difference between USB midi and straight midi
 * blocks of 4, 3 bytes
 */

static DigitalBusReader bushandler;

void bus_setup(){
  debug << "setupBus";
}

extern "C" {
  static uint8_t bus_rx_index = 0;
  void USART_IRQHandler(void){
    static uint8_t frame[4];
    /* If overrun condition occurs, clear the ORE flag and recover communication */
    if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET)
      USART_ReceiveData(USART_PERIPH);
    else if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){    
      // Reading the receive data register clears the RXNE flag implicitly
      char c = USART_ReceiveData(USART_PERIPH);
      frame[bus_rx_index++] = c;
      if(bus_rx_index == 4){
	bus_rx_index = 0;
	// MidiReaderStatus status = 
	// 	bushandler.readFrame(frame);
	// if(status == ERROR_STATUS)
	// 	bushandler.clear();
	bushandler.readBusFrame(frame);
      }
    }
  }
}

/* outgoing: send message over digital bus */
void bus_tx_parameter(uint8_t pid, int16_t value){
  debug << "tx parameter [" << pid << "][" << value << "]" ;
  bushandler.sendParameterChange(pid, value);
}

/* incoming: callback when message received on digital bus */
void bus_rx_parameter(uint8_t pid, int16_t value){
  debug << "rx parameter [" << pid << "][" << value << "]" ;
  setParameter(pid, value);
}

void bus_tx_error(const char* reason){
  debug << "Digital bus send error: " << reason << ".";
}

void bus_rx_error(const char* reason){
  debug << "Digital bus receive error: " << reason << ".";
  bus_rx_index = 0;
}

void bus_tx_button(uint8_t bid, int16_t value){
  debug << "tx button [" << bid << "][" << value << "]" ;
  bushandler.sendButtonChange(bid, value);
}

void bus_rx_button(uint8_t bid, int16_t value){
  debug << "rx button [" << bid << "][" << value << "]" ;
  setButton(bid, value);
}
