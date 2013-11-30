#ifndef __CODEC_CONTROLLER_H
#define __CODEC_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "device.h"
#include "ApplicationSettings.h"

class CodecController;
extern CodecController codec;

// extern void audioCallback(int16_t *src, int16_t *dst, int16_t sz);

class CodecController {
private:
  uint16_t wm8731_registers[WM8731_NUM_REGS];
  void writeRegister(uint8_t addr, uint16_t val);
  uint16_t readRegister(uint8_t addr);
  void setRegister(uint8_t addr, uint16_t value);
  void clearRegister(uint8_t addr, uint16_t value);
  void setCodecMaster(bool master);
  void setCodecProtocol(I2SProtocol protocol);
  void setCodecFormat(I2SFormat format);  
public:
  CodecController();

  void setup();
  void init(ApplicationSettings& settings);
  uint32_t getSamplingRate();
  I2SProtocol getProtocol();
  I2SFormat getFormat();
  bool isMaster();

  void setActive(bool active);

  void start();
  void stop();
  void pause();
  void resume();

  void softMute(bool mute);
  void setBypass(bool bypass);
  bool getBypass();

  /* Input gain: 5 bit value 0-0x1f, decimal 0-31 */
  /* One step equals 1.5dB */
  /* 0dB equals 0x017, decimal 23 */
  /* Range is -34.5dB to +12dB*/
  void setInputGainLeft(uint8_t gain);
  void setInputGainRight(uint8_t gain);
  uint8_t getInputGainLeft();
  uint8_t getInputGainRight();

  /* Output gain: 7 bit value 0x30-0x7f, decimal 48-127 */
  /* One step equals 1dB */
  /* 0dB equals 0x079, decimal 121 */
  /* Range is -73dB to +6dB */
  /* Anything below 0x30, decimal 48, is MUTE */
  void setOutputGainLeft(uint8_t gain);
  void setOutputGainRight(uint8_t gain);
  uint8_t getOutputGainLeft();
  uint8_t getOutputGainRight();

  void setInputMuteLeft(bool mute);
  void setInputMuteRight(bool mute);
  bool getInputMuteLeft();
  bool getInputMuteRight();

  void setOutputMuteLeft(bool mute);
  void setOutputMuteRight(bool mute);
  bool getOutputMuteLeft();
  bool getOutputMuteRight();

  void setSwapLeftRight(bool swap);
  bool getSwapLeftRight();

};

#endif /* __CODEC_CONTROLLER_H */
