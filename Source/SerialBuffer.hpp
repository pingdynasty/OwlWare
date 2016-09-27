#ifndef _SerialBuffer_h_
#define _SerialBuffer_h_

#include <string.h> // for memcpy

template<uint16_t size>
class SerialBuffer {
private:
  volatile uint16_t writepos = 0;
  volatile uint16_t readpos = 0;
  uint8_t buffer[size];
public:
  void push(uint8_t* data, uint16_t len){
    uint8_t* dest = getWriteHead();
    uint16_t rem = size-writepos;
    if(len > rem){
      memcpy(dest, data, rem);
      memcpy(buffer, data+rem, len-rem);      
      writepos = len-rem;
    }else{
      memcpy(dest, data, len);
      writepos += len;
    }
  }
  void pull(uint8_t* data, uint16_t len){
    uint8_t* src = getReadHead();
    uint16_t rem = size-readpos;
    if(len > rem){
      memcpy(data, src, rem);
      memcpy(data+rem, buffer, len-rem);      
      readpos = len-rem;
    }else{
      memcpy(data, src, len);
      readpos += len;
    }
  }

  void push(uint8_t c){
    buffer[writepos++] = c;
    if(writepos >= size)
      writepos = 0;
  }
  uint8_t pull(){
    uint8_t c = buffer[readpos++];
    if(readpos >= size)
      readpos = 0;
    return c;
  }

  void skipUntilLast(char c){
    uint8_t* src = getReadHead();
    uint16_t rem = size-readpos;
    for(int i=0; i<rem; ++i){
      if(src[i] != c){
	readpos += i;
	return;
      }
    }
    rem = writepos;
    for(int i=0; i<rem; ++i){
      if(buffer[i] != c){
	readpos = i;
	return;
      }
    }
  }

  uint8_t* getWriteHead(){
    return buffer+writepos;
  }
  void incrementWriteHead(uint16_t len){
    // ASSERT((writepos >= readpos && writepos+len <= size) ||
    // 	   (writepos < readpos && writepos+len <= readpos), "uart rx overflow");
    writepos += len;
    if(writepos >= size)
      writepos -= size;
  }

  uint8_t* getReadHead(){
    return buffer+readpos;
  }
  void incrementReadHead(uint16_t len){
    // ASSERT((readpos >= writepos && readpos+len <= size) ||
    // 	   (readpos < writepos && readpos+len <= writepos), "uart rx underflow");
    readpos += len;
    if(readpos >= size)
      readpos -= size;
  }
  bool notEmpty(){
    return writepos != readpos;
  }
  uint16_t available(){
    return (writepos + size - readpos) % size;
  }
  uint16_t getContiguousWriteCapacity(){
    if(writepos < readpos)
      return readpos - writepos;
    else
      return size - writepos;
    // return size-writepos;
  }
  uint16_t getContiguousReadCapacity(){
    if(writepos < readpos)
      return size - readpos;
    else
      return writepos - readpos;
  }
  void reset(){
    readpos = writepos = 0;
  }
};

#endif /* _SerialBuffer_h_ */
