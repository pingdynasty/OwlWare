#ifndef __CRCC_H__
#define __CRCC_H__

#include <inttypes.h>

class CRCC {
  /* from mvduin on the STM32 Forum
     https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=%2Fpublic%2FSTe2ecommunities%2Fmcu%2FLists%2Fcortex_mx_stm32%2FSTM32F4%20CRC%20-%20Preload%20CRC-DR&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=1233
  */
private:
  /*00*/  volatile uint32_t  dr;     //r>
  /*04*/  uint8_t      idr;    //rw
  /*08*/  volatile uint32_t  cr;     //-x
 
  uint32_t rbit( uint32_t x ) {
    unsigned char y = x;
    y = ((y * 0x0802LU & 0x22110LU) | (y * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
    // uint32_t y;
    // asm( "  rbit    %0, %1" : "=r" (y) : "r" (x) );
    return y;
  }

public: 
  CRCC() : cr(1) {}

  void reset() {  cr = 1;  }
 
  void write( uint32_t data ) {  dr = rbit( data );  }
 
  uint32_t read() {  return ~rbit( dr );  } 
 
  uint32_t calc( uint8_t data, uint32_t crc )  // single byte
  {
    crc = ~crc ^ data;
 
    reset();
    write( ~( crc << 24 ) );
    return ( crc >> 8 ) ^ read();
  }
 
  uint32_t calc( size_t nwords, uint32_t wdata[], uint32_t crc = 0 )  // wordwise
  {
    if( nwords == 0 )
      return crc;
 
    reset();
    write( wdata[0] ^ crc );
    for( size_t i = 1; i != nwords; i++ )
      write( wdata[i] );
 
    return read();
  }
 
  uint32_t calc( size_t nbytes, uint8_t data[], uint32_t crc = 0 )  // general
  {
    if( nbytes >= 4 ) {
      // process 0-3 bytes to make pointer word-aligned
      size_t align = -(size_t)data & 3;
      nbytes -= align;
      while( align-- )
	crc = calc( *data++, crc );
 
      // process bulk of data
      size_t nwords = nbytes >> 2;
      crc = calc( nwords, (uint32_t *)data, crc );
      data += nwords << 2;
      nbytes &= 3;
    }
 
    // leftover bytes
    while( nbytes-- )
      crc = calc( *data++, crc );
 
    return crc;
  }
};

#endif // __CRCC_H__
