#include "sysex.h"

/* convert to/from sysex 7-bit data 
 * taken from http://blogs.bl0rg.net/netzstaub/2008/08/14/encoding-8-bit-data-in-midi-sysex/
 */
uint8_t data_to_sysex(uint8_t *data, uint8_t *sysex, uint8_t len) {
  uint8_t retlen = 0;
  uint8_t cnt;
  uint8_t cnt7 = 0;

  sysex[0] = 0;
  for(cnt = 0; cnt < len; cnt++) {
    uint8_t c = data[cnt] & 0x7F;
    uint8_t msb = data[cnt] >> 7;
    sysex[0] |= msb << cnt7;
    sysex[1 + cnt7] = c;
    if(cnt7++ == 6) {
      sysex += 8;
      retlen += 8;
      sysex[0] = 0;
      cnt7 = 0;
    }
  }
  return retlen + cnt7 + (cnt7 != 0 ? 1 : 0);
}

uint8_t sysex_to_data(uint8_t *sysex, uint8_t *data, uint8_t len) {
  uint8_t cnt;
  uint8_t cnt2 = 0;
  uint8_t bits = 0;
  for(cnt = 0; cnt < len; cnt++) {
    if((cnt % 8) == 0) {
      bits = sysex[cnt];
    }else{
      data[cnt2++] = sysex[cnt] | ((bits & 1) << 7);
      bits >>= 1;
    }
  }
  return cnt2;
}
