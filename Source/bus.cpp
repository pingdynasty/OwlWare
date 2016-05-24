#include "bus.h"
#include "serial.h"
// #include "MidiReader.hpp"
#include "MidiHandler.hpp"

static MidiHandler bushandler;

void setupBus(){
}

void UART4_IRQHandler(void){
  static uint8_t index = 0;
  static uint8_t buf[4];
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
    char c = USART_ReceiveData(USART1);
    // pass it on
    // printByte() blocks but presumably 
    // sending data will be as fast as receiving it
    printByte(c);
    buf[index++] = c;
    if(index == 4){
      index = 0;
      // skip first of four bytes
      MidiReaderStatus status;
      bushandler.read(buf[1]);
      bushandler.read(buf[2]);
      status = bushandler.read(buf[3]);
      if(status == ERROR_STATUS)
	bushandler.clear();
      // // pass it on
      // serial_write(buf, 4);
    }
  }
}
