#include "bus.h"
#include "owlcontrol.h"
#include "Owl.h"
#include <string.h> // for memcpy
#include "SerialBuffer.hpp"

static SerialBuffer<256> bus_tx_buf;

#include "DigitalBusStreamReader.h"
DigitalBusStreamReader bus;
// #include "MidiStreamReader.h"
// MidiStreamReader bus;

void bus_setup(){
  // debug << "bus_setup";
  // bus.sendReset();
}

void bus_process(){
  bus.process();
}

void bus_set_midi_channel(uint8_t ch){
  bus.setInputChannel(ch);
}

uint8_t* bus_deviceid(){
  return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
  // return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
}

extern "C" {
  // static uint8_t bus_rx_index = 0;

  void USART_IRQHandler(void){
    if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){    
      // Reading the receive data register clears the RXNE flag implicitly
      char c = USART_ReceiveData(USART_PERIPH);
      bus.read(c);
    }
    //   static uint8_t frame[4];
    //   char c = USART_ReceiveData(USART_PERIPH);
    //   frame[bus_rx_index++] = c;
    //   if(bus_rx_index == 4){
    // 	bus_rx_index = 0;
    // 	bus.readBusFrame(frame);
    //   }
    // }
    if(USART_GetITStatus(USART_PERIPH, USART_IT_TXE) != RESET){
      if(bus_tx_buf.notEmpty()){
	USART_SendData(USART_PERIPH, bus_tx_buf.pull());
      }else{
	USART_ITConfig(USART_PERIPH, USART_IT_TXE, DISABLE);
      }
    }
    if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET){
      /* If overrun condition occurs, clear the ORE flag and recover communication */
      USART_ReceiveData(USART_PERIPH);
      // bus_rx_index = 0;
    }
  }

  void serial_write(uint8_t* data, uint16_t size){
    bus_tx_buf.push(data, size);
    USART_ITConfig(USART_PERIPH, USART_IT_TXE, ENABLE);
  }
}

/* outgoing: send message over digital bus */
void bus_tx_midi(uint8_t* frame){
  serial_write(frame, 4); // assuming 4 byte buffer
  // bus.sendFrame(frame);
}

void bus_tx_parameter(uint8_t pid, int16_t value){
  // debug << "tx parameter [" << pid << "][" << value << "]" ;
  bus.sendParameterChange(pid, value);
}

/* incoming: callback when message received on digital bus */
void bus_rx_parameter(uint8_t pid, int16_t value){
  // debug << "rx parameter [" << pid << "][" << value << "]" ;
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
  bus.reset();
  bus.sendReset();
  error(USB_ERROR, reason);
}

void bus_tx_button(uint8_t bid, int16_t value){
  // debug << "tx button [" << bid << "][" << value << "]" ;
  bus.sendButtonChange(bid, value);
}

void bus_rx_button(uint8_t bid, int16_t value){
  // debug << "rx button [" << bid << "][" << value << "]" ;
  setButton(bid, value);
}
