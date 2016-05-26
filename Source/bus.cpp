#include "bus.h"
#include "serial.h"
// #include "MidiReader.hpp"
#include "MidiHandler.hpp"

static MidiHandler bushandler;

void setupBus(){
}

extern "C" {
void USART_IRQHandler(void){
  if(USART_GetITStatus(USART_PERIPH, USART_IT_RXNE) != RESET){
    // Reading the receive data register clears the RXNE flag implicitly
    char c = USART_ReceiveData(USART_PERIPH);
  }
  /* If overrun condition occurs, clear the ORE flag and recover communication */
  if(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_ORE) != RESET)
    USART_ReceiveData(USART_PERIPH);
}
}

// void UART4_IRQHandler(void){
//   static uint8_t index = 0;
//   static uint8_t buf[4];
//   // PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun error) and IDLE (Idle line detected) pending bits are cleared by USART_GetITStatus() followed by USART_ReceiveData().
//   if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET){
//     // Reading the receive data register clears the RXNE flag implicitly
//     char c = USART_ReceiveData(UART4);
//     // pass it on
//     // printByte() blocks but presumably 
//     // sending data will be as fast as receiving it
//     buf[index++] = c;
//     if(index == 4){
//       index = 0;
//       // skip first of four bytes
//       MidiReaderStatus status;
//       bushandler.read(buf[1]);
//       bushandler.read(buf[2]);
//       status = bushandler.read(buf[3]);
//       if(status == ERROR_STATUS)
// 	bushandler.clear();
//     }
//     // // printByte(c);
//     // if(index == 4){
//     //   index = 0;
//     //   // skip first of four bytes
//     //   MidiReaderStatus status;
//     //   bushandler.read(buf[1]);
//     //   bushandler.read(buf[2]);
//     //   status = bushandler.read(buf[3]);
//     //   if(status == ERROR_STATUS)
//     // 	bushandler.clear();
//     //   // // pass it on
//     //   // serial_write(buf, 4);
//     // }
//     // USART_ClearITPendingBit(UART4, USART_IT_RXNE);
//   }
//   // if(USART_GetITStatus(UART4, USART_IT_ERR) != RESET)
//   //   USART_ClearITPendingBit(UART4, USART_IT_ERR);
//   /* If overrun condition occurs, clear the ORE flag and recover communication */
//   if (USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET)
//     USART_ReceiveData(UART4);
// }
