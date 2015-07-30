#include <inttypes.h>

#define FIRMWARE_VERSION             "vector-09"

/* #define DEFINE_OWL_SYSTICK */
/* if DEFINE_OWL_SYSTICK is defined, undefine xPortSysTickHandler in FreeRTOSConfig.h */
/* #define SERIAL_PORT                  1 */

/* #define DEBUG_AUDIO */
#define DEBUG_DWT
#define DEBUG_STACK

#define DEFAULT_PROGRAM              1
#define BUTTON_PROGRAM_CHANGE

#define AUDIO_BIGEND
/* #define AUDIO_SATURATE_SAMPLES // SATURATE adds almost 500 cycles to 24-bit mode */
#define AUDIO_PROTOCOL               I2S_PROTOCOL_PHILIPS
#define AUDIO_BITDEPTH               24    /* bits per sample */
#define AUDIO_DATAFORMAT             24
#define AUDIO_CODEC_MASTER           true
#define AUDIO_CHANNELS               2
#define AUDIO_SAMPLINGRATE           48000
#define AUDIO_BLOCK_SIZE             128   /* size in samples of a single channel audio block */
#define AUDIO_MAX_BLOCK_SIZE         1024

#define CCMRAM                      ((uint32_t)0x10000000)
#define PATCHRAM                    ((uint32_t)0x2000c000)
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
#define NOF_PARAMETERS               5
#define MAX_BUFFERS_PER_PATCH        8
#define MAX_SYSEX_FIRMWARE_SIZE      ((16+16+64+128+128)*1024) // FLASH sectors 2-6
#define MAX_SYSEX_PROGRAM_SIZE       (128*1024) // 128k, one flash sector

#define MAX_FACTORY_PATCHES          36
#define MAX_USER_PATCHES             4
#define MAX_NUMBER_OF_PATCHES        (MAX_FACTORY_PATCHES+MAX_USER_PATCHES+1)

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                    100000

#define USB_IRQ_PRIORITY             3
#define USB_IRQ_SUBPRIORITY          0
#define SWITCH_A_PRIORITY            2
#define SWITCH_A_SUBPRIORITY         0
#define SWITCH_B_PRIORITY            3
#define SWITCH_B_SUBPRIORITY         0
#define SERIAL_PORT_PRIORITY         3
#define SERIAL_PORT_SUBPRIORITY      0
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

#define BYPASS_DEBOUNCE              200
#define PUSHBUTTON_DEBOUNCE          100

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

#define PROGRAM_TASK_STACK_SIZE          (4*1024/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_STACK_SIZE          (512/sizeof(portSTACK_TYPE))
#define FLASH_TASK_STACK_SIZE            (512/sizeof(portSTACK_TYPE))
#define PC_TASK_STACK_SIZE               (512/sizeof(portSTACK_TYPE))
#define ARM_CYCLES_PER_SAMPLE            3500 /* 168MHz / 48kHz */

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

#define CCM __attribute__ ((section (".ccmdata")))
/* #ifdef EXTERNAL_SRAM */
/* #define EXT __attribute__ ((section (".extdata"))) */
/* #endif */
