#include "serial.h"
#include "device.h"
#include "stm32f4xx.h"

/* Initialize the serial port */
void serial_setup(uint32_t baudrate) {
  /* Enable clocks */
  USART_CLK_CMD(USART_CLK, ENABLE);
  USART_GPIO_CLK_CMD(USART_GPIO_CLK, ENABLE);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = USART_TX_PIN | USART_RX_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(USART_GPIO_PORT, &GPIO_InitStruct);

  GPIO_PinAFConfig(USART_GPIO_PORT, USART_TX_PINSOURCE, USART_GPIO_AF);
  GPIO_PinAFConfig(USART_GPIO_PORT, USART_RX_PINSOURCE, USART_GPIO_AF);

  /* /\* Configure USART Tx as alternate function push-pull *\/ */
  /* GPIO_InitTypeDef GPIO_InitStruct; */
  /* GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; */
  /* GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; */
  /* GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; */
  /* /\* GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; *\/ */
  /* GPIO_InitStruct.GPIO_Pin = USART_TX_PIN; */
  /* GPIO_Init(USART_TX_GPIO_PORT, &GPIO_InitStruct); */

  /* /\* Configure USART Rx as open drain *\/ */
  /* /\* GPIO_InitStruct.GPIO_Mode = GPIO_OType_OD; *\/ */
  /* GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; */
  /* /\* GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; *\/ */
  /* GPIO_InitStruct.GPIO_Pin = USART_RX_PIN; */
  /* GPIO_Init(USART_RX_GPIO_PORT, &GPIO_InitStruct); */

  /* USART configuration */
  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate = baudrate;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART_PERIPH, &USART_InitStruct);

  /* Enable the Transmit interrupt: this interrupt is generated when the 
     transmit data register is empty */  
  /* USART_ITConfig(USART_PERIPH, USART_IT_TXE, ENABLE); */
  /* Enable the Receive interrupt: this interrupt is generated when the 
     receive data register is not empty */
  USART_ITConfig(USART_PERIPH, USART_IT_RXNE, ENABLE);
  /* Enable the Error interrupt: Frame error, noise error, overrun error */
  USART_ITConfig(USART_PERIPH, USART_IT_ERR, ENABLE);

  /* Enable the USART Interrupt */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SERIAL_PORT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SERIAL_PORT_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable USART */
  USART_Cmd(USART_PERIPH, ENABLE);
}

#if 0
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
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); // PB6 TX
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1); // PB7 RX
  // alternate function pin remap: todo?
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
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // PA2 TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // PA3 RX
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

void setupSerialPort4(uint32_t baudrate){
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  /* GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; */
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4); // PA0 TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4); // PA1 RX
  USART_InitStruct.USART_BaudRate = baudrate;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(UART4, &USART_InitStruct);
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SERIAL_PORT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = SERIAL_PORT_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  USART_Cmd(UART4, ENABLE);
  usart = UART4;
}
#endif

void USART_puts(USART_TypeDef* USARTx, volatile const char *s){
  while(*s){
    // wait until data register is empty
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, *s);
    *s++;
  }
}

void USART_putc(USART_TypeDef* USARTx, char c){
  while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
  USART_SendData(USARTx, c);
}

void printString(const char* s){
#ifdef USART_PERIPH
  USART_puts(USART_PERIPH, s);
#endif
}

void printByte(char c){
#ifdef USART_PERIPH
  USART_putc(USART_PERIPH, c);
#endif
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

void serial_write(uint8_t* data, uint16_t size){
#ifdef USART_PERIPH
  for(int i=0; i<size; ++i){
    // wait until data register is empty
    while(USART_GetFlagStatus(USART_PERIPH, USART_FLAG_TXE) == RESET);
    USART_SendData(USART_PERIPH, data[i]);
  }
#endif
}
