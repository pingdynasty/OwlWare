#include "serial.h"
#include "device.h"
#include "stm32f4xx.h"

USART_TypeDef* usart = 0;

void setupSerialPort1(uint32_t baudrate){
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
  USART_InitStruct.USART_BaudRate = baudrate;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART1, &USART_InitStruct);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SERIAL_PORT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SERIAL_PORT_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  USART_Cmd(USART1, ENABLE);
  usart = USART1;
}

void setupSerialPort2(uint32_t baudrate){
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
  USART_InitStruct.USART_BaudRate = baudrate;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART2, &USART_InitStruct);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SERIAL_PORT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SERIAL_PORT_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  USART_Cmd(USART2, ENABLE);
  usart = USART2;
}

void USART_puts(USART_TypeDef* USARTx, volatile const char *s){
  while(*s){
    // wait until data register is empty
    while( !(USARTx->SR & 0x00000040) );
    USART_SendData(USARTx, *s);
    *s++;
  }
}

void USART_putc(USART_TypeDef* USARTx, char c){
  while( !(USARTx->SR & 0x00000040) );
  USART_SendData(USARTx, c);
}

void printString(const char* s){
  if(usart)
    USART_puts(usart, s);
}

void printByte(char c){
  if(usart)
    USART_putc(usart, c);
}

void printIntegerInBase(unsigned long n, unsigned long base){ 
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
  unsigned long i = 0;

  if (n == 0) {
    printByte('0');
    return;
  } 

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    printByte(buf[i - 1] < 10 ?
	      '0' + buf[i - 1] :
	      'A' + buf[i - 1] - 10);
}

void printInteger(long n)
{
  if(n < 0){
    printByte('-');
    n = -n;
  }
  printIntegerInBase(n, 10);
}

void printHex(unsigned long n)
{
  printIntegerInBase(n, 16);
}

void printOctal(unsigned long n)
{
  printIntegerInBase(n, 8);
}

void printBinary(unsigned long n){
  printIntegerInBase(n, 2);
}

void printDouble(double val, uint8_t precision){
  if(val < 0.0){
    printByte('-');
    val = -val;
  }
  printInteger((long)val);
  if(precision > 0) {
    printByte('.');
    unsigned long frac;
    unsigned long mult = 1;
    uint8_t padding = precision -1;
    while(precision--)
      mult *=10;
    if(val >= 0)
      frac = (val - (int)val) * mult;
    else
      frac = ((int)val- val) * mult;
    unsigned long frac1 = frac;
    while(frac1 /= 10 )
      padding--;
    while(padding--)
      printByte('0');
    printInteger(frac);
  }
}
