#include "bus.h"
#include "DigitalBusReader.h"

/*
 * difference between USB midi and straight midi
 * blocks of 4, 3 bytes
 */

static DigitalBusReader bushandler;

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
      frame[index++] = c;
      if(index == 4){
	index = 0;
	// MidiReaderStatus status = 
	// 	bushandler.readFrame(frame);
	// if(status == ERROR_STATUS)
	// 	bushandler.clear();
	bushandler.readBusFrame(frame);
      }
    }
  }
}
