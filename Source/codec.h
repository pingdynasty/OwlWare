/*
 * codec.h - stm32f405_codec board codec interface routines
 *
 * Cut from stm32f4_discovery_audio_codec.c
 *
 */

#ifndef __codec__
#define __codec__

#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx.h"

/*-----------------------------------
Hardware Configuration defines parameters
-----------------------------------------*/                 
/* I2S peripheral configuration defines */
#define CODEC_I2S                      SPI2
#define CODEC_I2S_EXT                  I2S2ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI2
#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S_EXT_ADDRESS          0x4000340C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI2
#define CODEC_I2S_IRQ                  SPI2_IRQn
#define CODEC_I2S_EXT_IRQ              SPI2_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)
#define CODEC_I2S_WS_PIN               GPIO_Pin_12
#define CODEC_I2S_SCK_PIN              GPIO_Pin_13
#define CODEC_I2S_SDI_PIN              GPIO_Pin_14
#define CODEC_I2S_SDO_PIN              GPIO_Pin_15
#define CODEC_I2S_MCK_PIN              GPIO_Pin_6
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource12
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource13
#define CODEC_I2S_SDI_PINSRC           GPIO_PinSource14
#define CODEC_I2S_SDO_PINSRC           GPIO_PinSource15
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource6
#define CODEC_I2S_GPIO                 GPIOB
#define CODEC_I2S_MCK_GPIO             GPIOC
#define AUDIO_I2S_IRQHandler           SPI2_IRQHandler


#define AUDIO_DMA_PERIPH_DATA_SIZE     DMA_PeripheralDataSize_HalfWord
#define AUDIO_DMA_MEM_DATA_SIZE        DMA_MemoryDataSize_HalfWord

/* I2S DMA Stream definitions */
#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream4
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream4_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF4
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF4
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF4
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF4
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF4
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream3
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_3
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream3_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF3
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF3
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF3

/* I2C peripheral configuration defines (control interface of the audio codec) */
#define CODEC_I2C                      I2C2
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C2
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_10
#define CODEC_I2C_SDA_PIN              GPIO_Pin_11
#define CODEC_I2S_SCL_PINSRC           GPIO_PinSource10
#define CODEC_I2S_SDA_PINSRC           GPIO_PinSource11

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))


/* ------------------------------------------------------------ */
/* WM8731 Registers */

#define LEFT_LINE_IN_REGISTER                   0x00
#define RIGHT_LINE_IN_REGISTER                  0x01
#define LEFT_HEADPHONE_OUT_REGISTER             0x02
#define RIGHT_HEADPHONE_OUT_REGISTER            0x03
#define ANALOGUE_AUDIO_PATH_CONTROL_REGISTER    0x04
#define DIGITAL_AUDIO_PATH_CONTROL_REGISTER     0x05
#define POWER_DOWN_CONTROL_REGISTER             0x06
#define DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER 0x07
#define SAMPLING_CONTROL_REGISTER               0x08
#define ACTIVE_CONTROL_REGISTER                 0x09
#define RESET_CONTROL_REGISTER                  0x0f

/* Register 0x00 and 0x01: Left/Right Line In */
#define WM8731_INVOL_P6DB    0x01b
#define WM8731_INVOL_0DB     0x017
#define WM8731_INVOL_M6DB    0x013
#define WM8731_INMUTE       (1<<7)
#define WM8731_INBOTH       (1<<8)

/* Register 0x02 and 0x03: Left/Right Headphone Out */
#define WM8731_HPVOL_P6DB    0x07f
#define WM8731_HPVOL_0DB     0x079
#define WM8731_HPVOL_M6DB    0x073
#define WM8731_HPVOL_M73DB   0x030
#define WM8731_HPVOL_MUTE    0x000
#define WM8731_ZCEN         (1<<7)
#define WM8731_HPBOTH       (1<<8)

/* Register 0x04: Analogue Audio Path Control */
#define WM8731_MICBOOST      (1<<0)     /* Microphone Input Level Boost Enable */
#define WM8731_MUTEMIC       (1<<1)     /* Mic Input Mute to ADC */
#define WM8731_INSEL         (1<<2)     /* Microphone Input Enable, Line Input Disable */
#define WM8731_BYPASS        (1<<3)     /* Bypass Enable */
#define WM8731_DACSEL        (1<<4)     /* DAC Enable */
#define WM8731_SIDETONE      (1<<5)     /* Side Tone Enable */
#define WM8731_SIDEATT_M15DB (0x03<<6)  /* Side Tone Attenuation -15dB */
#define WM8731_SIDEATT_M12DB (0x02<<6)  /* Side Tone Attenuation -12dB */
#define WM8731_SIDEATT_M9DB  (0x01<<6)  /* Side Tone Attenuation -9dB */
#define WM8731_SIDEATT_M6DB  (0x00<<6)  /* Side Tone Attenuation -6dB */

/* Register 0x05: Digital Audio Path Control */
#define WM8731_ADCHPD        (1<<0)     /* ADC High Pass Filter Disable */
#define WM8731_DEEMP_48K     (0x03<<1)  /* De-emphasis Control 48kHz */
#define WM8731_DEEMP_44K1    (0x02<<1)  /* De-emphasis Control 44.1kHz */
#define WM8731_DEEMP_32K     (0x01<<1)  /* De-emphasis Control 32kHz */
#define WM8731_DEEMP_NONE     0x00      /* De-emphasis Control Disabled */
#define WM8731_DACMU         (1<<3)     /* DAC Soft Mute */
#define WM8731_HPOR          (1<<4)     /* Store dc offset when High Pass Filter disabled */

/* Register 0x06: Power Down Control */
#define WM8731_LINEINPD      (1<<0)
#define WM8731_MICPD         (1<<1)
#define WM8731_ADCPD         (1<<2)
#define WM8731_DACPD         (1<<3)
#define WM8731_OUTPD         (1<<4)
#define WM8731_OSCPD         (1<<5)
#define WM8731_CLKOUTPD      (1<<6)
#define WM8731_POWEROFF      (1<<7)

/* Register 0x07: Digital Audio Interface Format */
#define WM8731_FORMAT_DSP     0x03      /* Audio Data Format: DSP Mode, frame sync + 2 data packed words */
#define WM8731_FORMAT_I2S     0x02      /* Audio Data Format: I2S Mode, MSB-First left-1 justified */
#define WM8731_FORMAT_MSB_LJ  0x01      /* Audio Data Format: MSB-First, left justified */
#define WM8731_FORMAT_MSB_RJ  0x00      /* Audio Data Format: MSB-First, right justified */
#define WM8731_IWL_16BIT      0x00      /* Input Word Length 16 bits */
#define WM8731_IWL_20BIT     (0x01<<2)  /* Input Word Length 20 bits */
#define WM8731_IWL_24BIT     (0x02<<2)  /* Input Word Length 24 bits */
#define WM8731_IWL_32BIT     (0x03<<2)  /* Input Word Length 32 bits */
#define WM8731_LRP           (1<<4)     /* DACLRC phase control */
#define WM8731_LRSWAP        (1<<5)     /* DAC Left Right Clock Swap */
#define WM8731_MS            (1<<6)     /* Master Mode Enable / Slave Mode Disable */
#define WM8731_BCLKINV       (1<<7)     /* Bit Clock Invert */

/* Register 0x08: Sampling Control */
#define WM8731_MODE_NORMAL    0x00
#define WM8731_MODE_USB       0x01
#define WM8731_BOSR          (1<<1)
#define WM8731_SR_48_48      (0x00<<2)  /* Normal mode rates, MCLK = 12.288MHz, 256fs  */
#define WM8731_SR_48_08      (0x01<<2)
#define WM8731_SR_08_48      (0x02<<2)
#define WM8731_SR_08_08      (0x03<<2)
#define WM8731_SR_32_32      (0x06<<2)
#define WM8731_SR_96_96      (0x07<<2)
#define WM8731_SR_USB_48_48  (0x00<<2)  /* USB mode rates */
#define WM8731_SR_USB_44_44  (0x18<<2)
#define WM8731_SR_USB_48_08  (0x01<<2)
#define WM8731_SR_USB_44_08  (0x19<<2)
#define WM8731_SR_USB_08_48  (0x02<<2)
#define WM8731_SR_USB_08_44  (0x1a<<2)
#define WM8731_SR_USB_08_08  (0x03<<2)
#define WM8731_SR_USB_32_32  (0x06<<2)
#define WM8731_SR_USB_96_96  (0x07<<2)
#define WM8731_CLKIDIV2      (1<<6)     /* Core Clock is MCLK divided by 2 */
#define WM8731_CLKODIV2      (1<<7)     /* CLOCKOUT is Core Clock divided by 2 */

/* Register 0x09: Active Control */
#define WM8731_ACTIVE        (1<<0)     /* Activate Interface */
#define WM8731_NOT_ACTIVE    (0<<0)     /* Activate Interface */

/* Register 0x0f: Reset Register */
#define WM8731_RESET          0x00      /* Reset Register */

/* ------------------------------------------------------------ */

#ifdef __cplusplus
 extern "C" {
#endif

/* Low layer codec functions */
void     Codec_CtrlInterface_Init(void);
uint32_t Codec_AudioInterface_Init(uint32_t AudioFreq, bool master, uint16_t standard, uint16_t format);
uint32_t Codec_Reset(void);
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue);
void     Codec_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif
