#include "CodecController.h"
#include "ApplicationSettings.h"

#include <string.h>
#include "codec.h"
#include "i2s.h"
#include "gpio.h"
#include "device.h"
#include "Owl.h"

#if AUDIO_BITDEPTH == 16
 /* size in half-words of the stereo audio buffers */
#define AUDIO_BUFFER_SIZE    (2*AUDIO_MAX_BLOCK_SIZE*AUDIO_CHANNELS)
#else
#define AUDIO_BUFFER_SIZE    (2*AUDIO_MAX_BLOCK_SIZE*AUDIO_CHANNELS*2)
#endif

/* DMA buffers for I2S */
int16_t tx_buffer[AUDIO_BUFFER_SIZE]; 
int16_t rx_buffer[AUDIO_BUFFER_SIZE];

const uint16_t wm8731_init_data[] = {
	WM8731_INVOL_P6DB,                   			  // Reg 0x00: Left Line In
	WM8731_INVOL_P6DB,			                  // Reg 0x01: Right Line In
	WM8731_HPVOL_M6DB,			                  // Reg 0x02: Left Headphone out
	WM8731_HPVOL_M6DB,	                                  // Reg 0x03: Right Headphone out
	WM8731_MUTEMIC|WM8731_DACSEL,                             // Reg 0x04: Analog Audio Path Control
#ifdef OWLMODULAR
	WM8731_ADCHPD|WM8731_DEEMP_NONE,                          // Reg 0x05: Digital Audio Path Control
#else
	WM8731_DEEMP_NONE,                                        // Reg 0x05: Digital Audio Path Control
#endif
	WM8731_MICPD|WM8731_OSCPD|WM8731_OUTPD|WM8731_CLKOUTPD,   // Reg 0x06: Power Down Control
	WM8731_FORMAT_I2S|WM8731_IWL_16BIT,                       // Reg 0x07: Digital Audio Interface Format
	WM8731_MODE_NORMAL|WM8731_SR_48_48,                       // Reg 0x08: Sampling Control
	WM8731_NOT_ACTIVE                    			  // Reg 0x09: Active Control
/*      0x017,                  // Reg 00: Left Line In (0dB, mute off) */
/*      0x017,                  // Reg 01: Right Line In (0dB, mute off) */
/*      0x079,                  // Reg 02: Left Headphone out (0dB) */
/*      0x079,                  // Reg 03: Right Headphone out (0dB) */
/* 	0x012,			// Reg 04: Analog Audio Path Control (DAC sel, Mute Mic) */
/* 	0x000,			// Reg 05: Digital Audio Path Control */
/* 	0x062,			// Reg 06: Power Down Control (Clkout, Osc, Mic Off) */
/* 	0x002,			// Reg 07: Digital Audio Interface Format (i2s, 16-bit, slave) */
/* 	0x042,                  // Reg 07: i2s, 16bit, master */
/* 	0x000,			// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC) */
/* 	0x001			// Reg 09: Active Control */
};

CodecController::CodecController(){
  memset(wm8731_registers, 0, sizeof(wm8731_registers));
}

void CodecController::setup(){
  /* Configure the Codec related IOs */
  Codec_GPIO_Init();   

  /* Initialize the Control interface of the Audio Codec */
  Codec_CtrlInterface_Init();     

  /* Reset the Codec Registers */
  if(Codec_Reset() != 0)
    assert_param(false);

  /* Load default values */
  for(int i=0;i<WM8731_NUM_REGS-1;i++)
    writeRegister(i, wm8731_init_data[i]);

//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // DEBUG
//   configureDigitalOutput(GPIOA, GPIO_Pin_6); // DEBUG
//   configureDigitalOutput(GPIOA, GPIO_Pin_7); // DEBUG
//   clearPin(GPIOA, GPIO_Pin_6); // DEBUG
//   clearPin(GPIOA, GPIO_Pin_7); // DEBUG
}

void CodecController::init(ApplicationSettings& settings){
//   setPin(GPIOA, GPIO_Pin_6); // DEBUG
  setActive(false);

  /* configure codec */
  setCodecMaster(settings.audio_codec_master);
  setCodecProtocol(settings.audio_codec_protocol);
  setCodecFormat(settings.audio_codec_format);

  /* Configure the I2S peripheral */
  if(Codec_AudioInterface_Init(settings.audio_samplingrate, settings.audio_codec_master, 
			       settings.audio_codec_protocol, settings.audio_codec_format) != 0)
    assert_param(false);

  setInputGainLeft(settings.inputGainLeft);
  setInputGainRight(settings.inputGainRight);
  setOutputGainLeft(settings.outputGainLeft);
  setOutputGainRight(settings.outputGainRight);

  I2S_Block_Init(tx_buffer, rx_buffer, settings.audio_blocksize);

  setActive(true);

//   clearPin(GPIOA, GPIO_Pin_6); // DEBUG
}

uint32_t CodecController::getSamplingRate(){
  return settings.audio_samplingrate;
}

I2SProtocol CodecController::getProtocol(){
  return settings.audio_codec_protocol;
}

I2SFormat CodecController::getFormat(){
  return settings.audio_codec_format;
}

bool CodecController::isMaster(){
  return settings.audio_codec_master;
}


void CodecController::setActive(bool active){
  /* It is recommended that between changing any content of Digital Audio Interface or Sampling Control Register that the active bit is reset then set. */
  if(active){
    /* OUTPD should be set AFTER active control, but that seems to hang the codec */
    clearRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OUTPD);
    setRegister(ACTIVE_CONTROL_REGISTER, WM8731_ACTIVE);
  }else{
    setRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OUTPD);
    clearRegister(ACTIVE_CONTROL_REGISTER, WM8731_ACTIVE);
  }
}

void CodecController::setCodecMaster(bool master){
  if(master){
    clearRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OSCPD);
    setRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_MS);
  }else{
    setRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OSCPD);
    clearRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_MS);
  }
}

void CodecController::setCodecProtocol(I2SProtocol protocol){
  if(protocol == I2S_PROTOCOL_PHILIPS){
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1fc)
		  | WM8731_FORMAT_I2S);
  }else{
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1fc)
		  | WM8731_FORMAT_MSB_LJ);
  }
}

void CodecController::setCodecFormat(I2SFormat format){
  switch(format){
  case I2S_FORMAT_16bit:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1f3)
		  | WM8731_IWL_16BIT);
    break;
  case I2S_FORMAT_24bit:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1f3)
		  | WM8731_IWL_24BIT);
    break;
  case I2S_FORMAT_32bit:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1f3)
		  | WM8731_IWL_32BIT);
    break;
  }
}

void CodecController::writeRegister(uint8_t addr, uint16_t value){
  if(addr < WM8731_NUM_REGS)
    wm8731_registers[addr] = value;
  Codec_WriteRegister(addr, value);
}

uint16_t CodecController::readRegister(uint8_t addr){
  return wm8731_registers[addr];
}

void CodecController::setRegister(uint8_t addr, uint16_t value){
  writeRegister(addr, wm8731_registers[addr] | value);
}

void CodecController::clearRegister(uint8_t addr, uint16_t value){
  writeRegister(addr, wm8731_registers[addr] & ~value);
}

void CodecController::start(){
//   setPin(GPIOA, GPIO_Pin_7); // DEBUG
  setActive(true);
  if(isMaster()){
    /* See STM32F405 Errata, I2S device limitations */
    /* The I2S peripheral must be enabled when the external master sets the WS line at: */
    if(getProtocol() == I2S_PROTOCOL_PHILIPS){
      while(getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for low
      /* High level when the I2S protocol is selected. */
      while(!getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for high
    }else{
      while(!getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for high
      /* Low level when the LSB or MSB-justified mode is selected. */
      while(getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for low
    }
  }
  I2S_Enable();
  I2S_Run();
}

void CodecController::stop(){
  I2S_Disable();
//   clearPin(GPIOA, GPIO_Pin_7); // DEBUG
  setActive(false);
}

void CodecController::pause(){
  I2S_Pause();
}

void CodecController::resume(){
  I2S_Resume();
}

void CodecController::softMute(bool mute){
  if(mute)
    setRegister(DIGITAL_AUDIO_PATH_CONTROL_REGISTER, WM8731_DACMU);
  else
    clearRegister(DIGITAL_AUDIO_PATH_CONTROL_REGISTER, WM8731_DACMU);
}

void CodecController::setBypass(bool bypass){
  if(bypass){
    softMute(true);
    writeRegister(ANALOGUE_AUDIO_PATH_CONTROL_REGISTER, 
		  (wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & ~WM8731_DACSEL) | WM8731_BYPASS);
  }else{
    writeRegister(ANALOGUE_AUDIO_PATH_CONTROL_REGISTER, 
		  (wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & ~WM8731_BYPASS) | WM8731_DACSEL);
    softMute(false);
  }
}

bool CodecController::getBypass(){
  return wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & WM8731_BYPASS;
}

void CodecController::setInputGainLeft(uint8_t gain){
  gain = (wm8731_registers[LEFT_LINE_IN_REGISTER] & 0xe0) | (gain & 0x1f);
  writeRegister(LEFT_LINE_IN_REGISTER, gain);
}

void CodecController::setInputGainRight(uint8_t gain){
  gain = (wm8731_registers[RIGHT_LINE_IN_REGISTER] & 0xe0) | (gain & 0x1f);
  writeRegister(RIGHT_LINE_IN_REGISTER, gain);
}

void CodecController::setOutputGainLeft(uint8_t gain){
  gain = (wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & 0x80) | (gain & 0x7f);
  writeRegister(LEFT_HEADPHONE_OUT_REGISTER, gain);
}

void CodecController::setOutputGainRight(uint8_t gain){
  gain = (wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & 0x80) | (gain & 0x7f);
  writeRegister(RIGHT_HEADPHONE_OUT_REGISTER, gain);
}

uint8_t CodecController::getInputGainLeft(){
  return wm8731_registers[LEFT_LINE_IN_REGISTER] & 0x1f;
}

uint8_t CodecController::getInputGainRight(){
  return wm8731_registers[RIGHT_LINE_IN_REGISTER] & 0x1f;
}

uint8_t CodecController::getOutputGainLeft(){
  return wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & 0x7f;
}

uint8_t CodecController::getOutputGainRight(){
  return wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & 0x7f;
}

void CodecController::setInputMuteLeft(bool mute){
  if(mute)
    setRegister(LEFT_LINE_IN_REGISTER, WM8731_INMUTE);
  else
    clearRegister(LEFT_LINE_IN_REGISTER, WM8731_INMUTE);
}

void CodecController::setInputMuteRight(bool mute){
  if(mute)
    setRegister(RIGHT_LINE_IN_REGISTER, WM8731_INMUTE);
  else
    clearRegister(RIGHT_LINE_IN_REGISTER, WM8731_INMUTE);
}

void CodecController::setOutputMuteLeft(bool mute){
  if(mute)
    setRegister(LEFT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
  else
    clearRegister(LEFT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
}

void CodecController::setOutputMuteRight(bool mute){
  if(mute)
    setRegister(RIGHT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
  else
    clearRegister(RIGHT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
}

bool CodecController::getInputMuteLeft(){
  return wm8731_registers[LEFT_LINE_IN_REGISTER] & WM8731_INMUTE;
}

bool CodecController::getInputMuteRight(){
  return wm8731_registers[RIGHT_LINE_IN_REGISTER] & WM8731_INMUTE;
}

bool CodecController::getOutputMuteLeft(){
  return wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & WM8731_HPVOL_MUTE;
}

bool CodecController::getOutputMuteRight(){
  return wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & WM8731_HPVOL_MUTE;
}

void CodecController::setSwapLeftRight(bool swap){
  if(swap)
    setRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_LRSWAP);
  else
    clearRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_LRSWAP);
}

bool CodecController::getSwapLeftRight(){
  return wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & WM8731_LRSWAP;
}
