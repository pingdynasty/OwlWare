#include <inttypes.h>

/* #define OWLMODULAR */
#define FIRMWARE_VERSION             "sysex-01"
#define HARDWARE_VERSION             "OWL Rev05"

/* #define SERIAL_PORT                  1 */

/* #define DEBUG_AUDIO */
/* #define DEBUG_DWT */

#define GREEN_PATCH                  0
#define RED_PATCH                    1

#define AUDIO_BIGEND
/* #define AUDIO_SATURATE_SAMPLES // SATURATE adds almost 500 cycles to 24-bit mode */
#define AUDIO_PROTOCOL               I2S_PROTOCOL_PHILIPS
#define AUDIO_FORMAT                 I2S_FORMAT_16bit
#define AUDIO_CODEC_MASTER           true
#define AUDIO_CHANNELS               2
#define AUDIO_BITDEPTH               16    /* bits per sample */
#define AUDIO_SAMPLINGRATE           48000
#define AUDIO_BLOCK_SIZE             256   /* size in samples of a single channel audio block */
#define AUDIO_MAX_BLOCK_SIZE         1024  /* size in samples of a single channel audio block */

#define PATCHRAM                    ((uint32_t)0x20008000)
#define EXTRAM                      ((uint32_t)0x68000000)

#ifdef OWLMODULAR
/* +0db in and out */
#define AUDIO_INPUT_GAIN_LEFT        0x017
#define AUDIO_INPUT_GAIN_RIGHT       0x017
#define AUDIO_OUTPUT_GAIN_LEFT       0x079
#define AUDIO_OUTPUT_GAIN_RIGHT      0x079 
#else /* OWLMODULAR */
/* +6db in, -6dB out */
#define AUDIO_INPUT_GAIN_LEFT        0x01b
#define AUDIO_INPUT_GAIN_RIGHT       0x01b
#define AUDIO_OUTPUT_GAIN_LEFT       0x073
#define AUDIO_OUTPUT_GAIN_RIGHT      0x073
#endif /* OWLMODULAR */

#define MIDI_CHANNEL                 0
#define MIDI_MAX_MESSAGE_SIZE        256
#define NOF_ADC_VALUES               5
#define NOF_PARAMETERS               24
#define MAX_BUFFERS_PER_PATCH        8
#define MAX_NUMBER_OF_PATCHES        64
#define MAX_SYSEX_FIRMWARE_SIZE      (80*1024)

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                    100000

#define USB_IRQ_PRIORITY             3
#define USB_IRQ_SUBPRIORITY          0
#define SWITCH_A_PRIORITY            2
#define SWITCH_A_SUBPRIORITY         1
#define SWITCH_B_PRIORITY            3
#define SWITCH_B_SUBPRIORITY         1
#define SERIAL_PORT_PRIORITY         3
#define SERIAL_PORT_SUBPRIORITY      3
#define SYSTICK_PRIORITY             2
#define SYSTICK_SUBPRIORITY          0

/* pin configuration */
/* Switch A: bypass foot switch */
#define SWITCH_A_PORT                GPIOE
#define SWITCH_A_CLOCK               RCC_AHB1Periph_GPIOE
#define SWITCH_A_PORT_SOURCE         EXTI_PortSourceGPIOE
#define SWITCH_A_PIN                 GPIO_Pin_4
#define SWITCH_A_PIN_SOURCE          EXTI_PinSource4
#define SWITCH_A_PIN_LINE            EXTI_Line4           /* Line 4 connects to all Px4 pins, et c */
#define SWITCH_A_IRQ                 EXTI4_IRQn
#define SWITCH_A_HANDLER             EXTI4_IRQHandler

/* Expression input */
#define EXPRESSION_PEDAL
#ifdef OWLMODULAR
#define EXPRESSION_PEDAL_TIP_PORT    GPIOA
#define EXPRESSION_PEDAL_TIP_PIN     GPIO_Pin_3
#define EXPRESSION_PEDAL_TIP_CHANNEL ADC_Channel_3
#define EXPRESSION_PEDAL_RING_PORT   GPIOA
#define EXPRESSION_PEDAL_RING_PIN    GPIO_Pin_2
#define EXPRESSION_PEDAL_RING_CHANNEL ADC_Channel_2
#else /* OWLMODULAR */
#define EXPRESSION_PEDAL_TIP_PORT    GPIOA
#define EXPRESSION_PEDAL_TIP_PIN     GPIO_Pin_2
#define EXPRESSION_PEDAL_TIP_CHANNEL ADC_Channel_2
#define EXPRESSION_PEDAL_RING_PORT   GPIOA
#define EXPRESSION_PEDAL_RING_PIN    GPIO_Pin_3
#define EXPRESSION_PEDAL_RING_CHANNEL ADC_Channel_3
#endif /* OWLMODULAR */

/* Switch B: illuminated pushbutton */
#define SWITCH_B_PORT                GPIOE
#define SWITCH_B_CLOCK               RCC_AHB1Periph_GPIOE
#define SWITCH_B_PORT_SOURCE         EXTI_PortSourceGPIOE
#define SWITCH_B_PIN                 GPIO_Pin_2
#define SWITCH_B_PIN_SOURCE          EXTI_PinSource2
#define SWITCH_B_PIN_LINE            EXTI_Line2
#define SWITCH_B_IRQ                 EXTI2_IRQn
#define SWITCH_B_HANDLER             EXTI2_IRQHandler

#define LED_PORT                     GPIOE
#define LED_GREEN                    GPIO_Pin_5
#define LED_RED                      GPIO_Pin_3
#define LED_CLOCK                    RCC_AHB1Periph_GPIOE

#define USB_DP_PIN                   GPIO_Pin_12
#define USB_DM_PIN                   GPIO_Pin_11
#define USB_DATA_GPIO_PORT           GPIOA
#define USB_DATA_GPIO_CLK            RCC_AHB1Periph_GPIOA

#define USB_VBUS_PIN                 GPIO_Pin_9
#define USB_VBUS_GPIO_PORT           GPIOA
#define USB_VBUS_GPIO_CLK            RCC_AHB1Periph_GPIOA

#define WM8731_NUM_REGS 10

#ifdef  USE_FULL_ASSERT
#ifdef __cplusplus
 extern "C" {
#endif
   void assert_failed(uint8_t* file, uint32_t line);
#ifdef __cplusplus
}
#endif
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
#endif

/* #define CCM __attribute__ ((section (".ccmdata"))) */
/* #ifdef EXTERNAL_SRAM */
/* #define EXT __attribute__ ((section (".extdata"))) */
/* #endif */
