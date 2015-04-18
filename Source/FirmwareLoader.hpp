#ifndef __FirmwareLoader_H__
#define __FirmwareLoader_H__

#include <math.h>
#include "crc32.h"
#include "sysex.h"
#include "clock.h"
#include "device.h"
// #include "ProgramManager.h"

class FirmwareLoader {
private:
  // enum SysexFirmwareStatus {
  //   NORMAL = 0,
  //   UPLOADING,
  //   ERROR = 0xff
  // };
  // SysexFirmwareStatus status = NORMAL;
  int packageIndex = 0;
  uint32_t timestamp;
  uint8_t* buffer = NULL;
  uint32_t size;
  uint32_t index;
  uint32_t crc;

public:
  void clear(){
    // free(buffer);
    buffer = NULL;
    index = 0;
    packageIndex = 0;
  }

  int error(int code){
    clear();
    return code;
  }

  uint8_t* getData(){
    return buffer;
  }

  uint32_t getSize(){
    return size;
  }

  /* decode a 32-bit unsigned integer from 5 bytes of sysex encoded data */
  uint32_t decodeInt(uint8_t *data){
    uint8_t buf[4];
    sysex_to_data(data, buf, 5);
    uint32_t result = buf[3] | (buf[2] << 8L) | (buf[1] << 16L) | (buf[0] << 24L);
    return result;
  }

  int32_t handleFirmwareUpload(uint8_t* data, uint16_t length){
    int offset = 3;
    int idx = decodeInt(data+offset);
    offset += 5;
    if(idx == 0){
      clear();
      // first package
      timestamp = getSysTicks();
      if(length < 3+5+5)
	return error(-1);
      // stop running program and free its memory
      exitProgram();
      // get firmware data size (decoded)
      size = decodeInt(data+offset);
      offset += 5; // it takes five 7-bit values to encode four bytes
      // allocate memory
      if(size > MAX_SYSEX_FIRMWARE_SIZE)
	return error(-2);
      buffer = (uint8_t*)EXTRAM;
      return 0;
    }
    if(++packageIndex != idx)
      return error(-7); // out of sequence package
    int len = floor((length-offset)*7/8.0f);
    // wait for program to exit before writing to buffer
    if(index+len < size){
      // mid package
      len = sysex_to_data(data+offset, buffer+index, length-offset);
      index += len;
      return 0;
    }
    // last package
    len = floor((length-offset-5)*7/8.0f);
    if(index+len != size)
      return error(-3); // wrong size
    len = sysex_to_data(data+offset, buffer+index, length-offset-5);
    index += len;
    if(index != size)
      return error(-4); // size mismatch
    // check crc
    crc = crc32(buffer, size, 0);
    // get checksum: last 4 bytes of buffer
    uint32_t checksum = decodeInt(data+length-5);
    if(crc != checksum)
      return error(-5);
    return index;
  }
};

#endif // __FirmwareLoader_H__
