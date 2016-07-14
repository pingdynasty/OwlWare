#include "bus.h"
#include "owlcontrol.h"
#include "Owl.h"
#include "DigitalBusReader.h"

/*
 * difference between USB midi and straight midi
 * blocks of 4, 3 bytes
 */

static DigitalBusReader bus;

void bus_setup(){
  debug << "bus_setup";
}

uint8_t* bus_deviceid(){
  return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
  // return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
}

extern "C" {
  static uint8_t bus_rx_index = 0;
  void USART_IRQHandler(void){
    static uint8_t frame[4];
    if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET){
      /* If overrun condition occurs, clear the ORE flag and recover communication */
      USART_ReceiveData(USART_PERIPH);
      bus_rx_index = 0;
    }else if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){    
      // Reading the receive data register clears the RXNE flag implicitly
      char c = USART_ReceiveData(USART_PERIPH);
      frame[bus_rx_index++] = c;
      if(bus_rx_index == 4){
	bus_rx_index = 0;
	// MidiReaderStatus status = 
	// 	bus.readFrame(frame);
	// if(status == ERROR_STATUS)
	// 	bus.clear();
	bus.readBusFrame(frame);
      }
    }
  }
}

/* outgoing: send message over digital bus */
void bus_tx_parameter(uint8_t pid, int16_t value){
  debug << "tx parameter [" << pid << "][" << value << "]" ;
  bus.sendParameterChange(pid, value);
}

/* incoming: callback when message received on digital bus */
void bus_rx_parameter(uint8_t pid, int16_t value){
  debug << "rx parameter [" << pid << "][" << value << "]" ;
  setParameter(pid, value);
}

void bus_rx_command(uint8_t cmd, int16_t data){
}

void bus_rx_message(const char* msg){
  debugMessage(msg);
}

void bus_rx_data(const uint8_t* ptr, uint16_t size){
}

void bus_tx_error(const char* reason){
  error(USB_ERROR, reason);
}

void bus_rx_error(const char* reason){
  error(USB_ERROR, reason);
  bus_rx_index = 0;
}

void bus_tx_button(uint8_t bid, int16_t value){
  debug << "tx button [" << bid << "][" << value << "]" ;
  bus.sendButtonChange(bid, value);
}

void bus_rx_button(uint8_t bid, int16_t value){
  debug << "rx button [" << bid << "][" << value << "]" ;
  setButton(bid, value);
}
