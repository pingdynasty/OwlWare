#include "bus.h"
#include "owlcontrol.h"
#include "Owl.h"
#include "DigitalBusReader.h"
#include <string.h> // for memcpy

/*
 * difference between USB midi and straight midi
 * blocks of 4, 3 bytes
 */

void bus_setup(){
  debug << "bus_setup";
  // bus.sendReset();
}

uint8_t* bus_deviceid(){
  return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
  // return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
}

template<uint16_t size>
class SerialBuffer {
private:
  volatile uint16_t writepos = 0;
  volatile uint16_t readpos = 0;
  uint8_t buffer[size];
public:
  void push(uint8_t c){
    buffer[writepos++] = c;
    if(writepos >= size)
      writepos = 0;
  }
  uint8_t pop(){
    uint8_t c = buffer[readpos++];
    if(readpos >= size)
      readpos = 0;
    return c;
  }
  uint8_t* getWriteHead(){
    return buffer+writepos;
  }
  void incrementWriteHead(uint16_t len){
    // ASSERT((writepos >= readpos && writepos+len <= size) ||
    // 	   (writepos < readpos && writepos+len <= readpos), "uart rx overflow");
    writepos += len;
    if(writepos >= size)
      writepos = 0;
  }
  uint8_t* getReadHead(){
    return buffer+readpos;
  }
  void incrementReadHead(uint16_t len){
    // ASSERT((readpos >= writepos && readpos+len <= size) ||
    // 	   (readpos < writepos && readpos+len <= writepos), "uart rx underflow");
    readpos += len;
    if(readpos >= size)
      readpos = 0;
  }
  bool notEmpty(){
    return writepos != readpos;
  }
  uint16_t available(){
    return (writepos + size - readpos) % size;
  }
  void reset(){
    readpos = writepos = 0;
  }
};

static SerialBuffer<128> bus_tx_buf;

extern "C" {
  static uint8_t bus_rx_index = 0;

  void USART_IRQHandler(void){
    if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){    
      // Reading the receive data register clears the RXNE flag implicitly
      static uint8_t frame[4];
      char c = USART_ReceiveData(USART_PERIPH);
      frame[bus_rx_index++] = c;
      if(bus_rx_index == 4){
	bus_rx_index = 0;
	bus.readBusFrame(frame);
      }
    }
    if(USART_GetITStatus(USART_PERIPH, USART_IT_TXE) != RESET){
      if(bus_tx_buf.notEmpty()){
	USART_SendData(USART_PERIPH, bus_tx_buf.pop());
      }else{
	USART_ITConfig(USART_PERIPH, USART_IT_TXE, DISABLE);
      }
    }
    if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET){
      /* If overrun condition occurs, clear the ORE flag and recover communication */
      USART_ReceiveData(USART_PERIPH);
      bus_rx_index = 0;
    }
  }

  void serial_write(uint8_t* data, uint16_t size){
    uint8_t* frame = bus_tx_buf.getWriteHead();
    memcpy(frame, data, size);
    bus_tx_buf.incrementWriteHead(size);
    USART_ITConfig(USART_PERIPH, USART_IT_TXE, ENABLE);
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
  setParameterValue(pid, value);
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
  bus_rx_index = 0;
  bus.reset();
  bus.sendReset();
  error(USB_ERROR, reason);
}

void bus_tx_button(uint8_t bid, int16_t value){
  debug << "tx button [" << bid << "][" << value << "]" ;
  bus.sendButtonChange(bid, value);
}

void bus_rx_button(uint8_t bid, int16_t value){
  debug << "rx button [" << bid << "][" << value << "]" ;
  setButton(bid, value);
}
