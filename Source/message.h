#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdint.h>

#ifndef CHECKSUM_ERROR_STATUS
#define CHECKSUM_ERROR_STATUS      -10
#endif
#ifndef OUT_OF_MEMORY_ERROR_STATUS
#define OUT_OF_MEMORY_ERROR_STATUS -20
#endif
#ifndef CONFIGURATION_ERROR_STATUS
#define CONFIGURATION_ERROR_STATUS -30
#endif

#ifdef __cplusplus
 extern "C" {
#endif

   char* ftoa(float val, int base);
   char* itoa(int val, int base);
   void debugMessage(const char* msg);
   // void error(int8_t code, const char* reason);
   // void assert_failed(const char* msg, const char* location, int line);

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
