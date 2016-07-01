#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdint.h>

#define ASSERT(cond, msg) do{if(!(cond))error(PROGRAM_ERROR, msg);}while(0)
#define NO_ERROR         0x00
#define HARDFAULT_ERROR  0x10
#define BUS_ERROR        0x20
#define MEM_ERROR        0x30
#define NMI_ERROR        0x40
#define USAGE_ERROR      0x50
#define PROGRAM_ERROR    0x60
#define FLASH_ERROR      0x70

#define CONFIGURATION_ERROR_STATUS -30
#define OUT_OF_MEMORY_ERROR_STATUS -20
#define CHECKSUM_ERROR_STATUS      -10

#ifdef __cplusplus
 extern "C" {
#endif

   char* ftoa(float val, int base);
   char* itoa(int val, int base);
   void debugMessage(const char* msg);
   void setErrorStatus(int8_t err);
   void error(int8_t err, const char* msg);
   // void assert_failed(const char* msg, const char* location, int line);
   const char* getErrorMessage();
   const char* getDebugMessage();
   int8_t getErrorStatus();

#ifdef __cplusplus
}

char* itoa(int val, int base, int pad);

class Debug {
private:
  uint8_t pos = 0;
public:
  void print(char arg);
  void print(float arg);
  void print(int arg);
  void print(const char* arg);
};
extern Debug debug;

template<class T>
inline Debug &operator <<(Debug &obj, T arg)
{ obj.print(arg); return obj; }

void debugMessage(const char* msg, int);
void debugMessage(const char* msg, int, int, int);
void debugMessage(const char* msg, float);
void debugMessage(const char* msg, float, float);
void debugMessage(const char* msg, float, float, float);
/* void assert_failed(uint8_t* location, uint32_t line); */

#endif

#endif /* __MESSAG£_H */
