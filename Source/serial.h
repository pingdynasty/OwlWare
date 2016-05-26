#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void setupSerialPort1(uint32_t baudrate);
   void setupSerialPort2(uint32_t baudrate);
   void setupSerialPort4(uint32_t baudrate);
   void printString(const char* s);
   void printByte(char c);
   void printInteger(long n);
   void printHex(unsigned long n);
   void printOctal(unsigned long n);
   void printBinary(unsigned long n);
   void printDouble(double d, uint8_t precision);

   void serial_read(uint8_t* data, uint16_t size);
   void serial_write(uint8_t* data, uint16_t size);
   void serial_setup(uint32_t baudRate);
#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_H */
