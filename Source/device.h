#ifndef __device_h__
#define __device_h__

#include <inttypes.h>

#define FIRMWARE_VERSION             "B v12e"

/* #define DEFINE_OWL_SYSTICK */
/* if DEFINE_OWL_SYSTICK is defined, undefine xPortSysTickHandler in FreeRTOSConfig.h */
/* #define SERIAL_PORT                  1 */
/* #define SERIAL_PORT                  2 */

/* untested: pins probably require remap */
/* #define USART_PERIPH                 USART1 */
/* #define USART_CLK                    RCC_APB1Periph_USART1 */
/* #define USART_GPIO_CLK               RCC_AHB1Periph_GPIOB */
/* #define USART_CLK_CMD(x, y)          RCC_APB2PeriphClockCmd(x, y) */
/* #define USART_GPIO_CLK_CMD(x, y)     RCC_AHB1PeriphClockCmd(x, y) */
/* #define USART_GPIO_PORT              GPIOB */
/* #define USART_GPIO_AF                GPIO_AF_USART1 */
/* #define USART_TX_PIN                 GPIO_Pin_6 */
/* #define USART_TX_PINSOURCE           GPIO_PinSource6 */
/* #define USART_RX_PIN                 GPIO_Pin_7 */
/* #define USART_RX_PINSOURCE           GPIO_PinSource7 */
/* #define USART_IRQHandler             USART1_IRQHandler */
/* #define USART_IRQn                   USART1_IRQn */

/* #define USART_BAUDRATE               115200 */
/* #define USART_PERIPH                 USART2 */
/* #define USART_CLK                    RCC_APB1Periph_USART2 */
/* #define USART_GPIO_CLK               RCC_AHB1Periph_GPIOA */
/* #define USART_CLK_CMD(x, y)          RCC_APB1PeriphClockCmd(x, y) */
/* #define USART_GPIO_CLK_CMD(x, y)     RCC_AHB1PeriphClockCmd(x, y) */
/* #define USART_GPIO_PORT              GPIOA */
/* #define USART_GPIO_AF                GPIO_AF_USART2 */
/* #define USART_TX_PIN                 GPIO_Pin_2 */
/* #define USART_TX_PINSOURCE           GPIO_PinSource2 */
/* #define USART_RX_PIN                 GPIO_Pin_3 */
/* #define USART_RX_PINSOURCE           GPIO_PinSource3 */
/* #define USART_IRQn                   USART2_IRQn */
/* #define USART_IRQHandler             USART2_IRQHandler */

#define USART_BAUDRATE               115200
#define USART_PERIPH                 UART4
#define USART_CLK                    RCC_APB1Periph_UART4
#define USART_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define USART_CLK_CMD(x, y)          RCC_APB1PeriphClockCmd(x, y)
#define USART_GPIO_CLK_CMD(x, y)     RCC_AHB1PeriphClockCmd(x, y)
#define USART_GPIO_PORT              GPIOA
#define USART_GPIO_AF                GPIO_AF_UART4
#define USART_TX_PIN                 GPIO_Pin_0
#define USART_TX_PINSOURCE           GPIO_PinSource0
#define USART_RX_PIN                 GPIO_Pin_1
#define USART_RX_PINSOURCE           GPIO_PinSource1
#define USART_IRQHandler             UART4_IRQHandler
#define USART_IRQn                   UART4_IRQn

#define EEPROM_PAGE_BEGIN            ((uint32_t)0x08060000)
#define EEPROM_PAGE_SIZE             (128*1024)
#define EEPROM_PAGE_END              ((uint32_t)0x08100000)
#define STORAGE_MAX_BLOCKS           64

/* #define DEBUG_AUDIO */
#define DEBUG_DWT
/* #define DEBUG_STACK */

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

#if defined OWLMODULAR || defined OWLRACK
/* +0db in and out */
#define AUDIO_INPUT_GAIN_LEFT        0x017
#define AUDIO_INPUT_GAIN_RIGHT       0x017
#define AUDIO_OUTPUT_GAIN_LEFT       0x079
#define AUDIO_OUTPUT_GAIN_RIGHT      0x079 
#define AUDIO_INPUT_OFFSET           0xffffefaa /* -0.06382 * 65535 */
#define AUDIO_INPUT_SCALAR           0xfffbb5c7 /* -4.290 * 65535 */
#define AUDIO_OUTPUT_OFFSET          0x00001eec /* 0.1208 * 65535 */
#define AUDIO_OUTPUT_SCALAR          0xfffb5bab /* -4.642 * 65535 */
#else /* OWLMODULAR */
/* +6db in, -6dB out */
#define AUDIO_INPUT_GAIN_LEFT        0x01b
#define AUDIO_INPUT_GAIN_RIGHT       0x01b
#define AUDIO_OUTPUT_GAIN_LEFT       0x073
#define AUDIO_OUTPUT_GAIN_RIGHT      0x073
#define AUDIO_INPUT_OFFSET           0x0     /* AC coupled */
#define AUDIO_INPUT_SCALAR           0x1fffe /* 2.0 * 65535 */
#define AUDIO_OUTPUT_OFFSET          0x0     /* AC coupled */
#define AUDIO_OUTPUT_SCALAR          0x1fffe /* 2.0 * 65535 */
#endif /* OWLMODULAR */

#define MIDI_OMNI_CHANNEL            (-1)
#define MIDI_OUTPUT_CHANNEL          0
#define MIDI_MAX_MESSAGE_SIZE        256
#define NOF_ADC_VALUES               5
#define NOF_PARAMETERS               24
#define NOF_BUTTONS                  5
#define MAX_BUFFERS_PER_PATCH        8
#define MAX_SYSEX_FIRMWARE_SIZE      ((16+16+64+128+128)*1024) // FLASH sectors 2-6
#define MAX_SYSEX_PROGRAM_SIZE       (128*1024) // 128k, one flash sector

#define MAX_FACTORY_PATCHES          36
#define MAX_USER_PATCHES             4
#define MAX_NUMBER_OF_PATCHES        40
#define MAX_NUMBER_OF_RESOURCES      12

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                    100000

#define USB_IRQ_PRIORITY             3
#define USB_IRQ_SUBPRIORITY          0
#define SWITCH_A_PRIORITY            2
#define SWITCH_A_SUBPRIORITY         0
#define SWITCH_B_PRIORITY            3
#define SWITCH_B_SUBPRIORITY         0
#define USART_PRIORITY               3
#define USART_SUBPRIORITY            0
#define SYSTICK_PRIORITY             2
#define SYSTICK_SUBPRIORITY          0

/* pin configuration */
#ifdef OWLMODULAR
/* Switch A: push gate on OWL Modular */
#define SWITCH_A_PORT                GPIOB
#define SWITCH_A_CLOCK               RCC_AHB1Periph_GPIOB
#define SWITCH_A_PORT_SOURCE         EXTI_PortSourceGPIOB
#define SWITCH_A_PIN                 GPIO_Pin_6
#define SWITCH_A_PIN_SOURCE          EXTI_PinSource6
#define SWITCH_A_PIN_LINE            EXTI_Line6           /* Line 6 connects to all Px6 pins, et c */
#define SWITCH_A_IRQ                 EXTI9_5_IRQn
#define SWITCH_A_HANDLER             EXTI9_5_IRQHandler
#else /* not OWLMODULAR */
/* Switch A: bypass foot switch on OWL Pedal */
#define SWITCH_A_PORT                GPIOE
#define SWITCH_A_CLOCK               RCC_AHB1Periph_GPIOE
#define SWITCH_A_PORT_SOURCE         EXTI_PortSourceGPIOE
#define SWITCH_A_PIN                 GPIO_Pin_4
#define SWITCH_A_PIN_SOURCE          EXTI_PinSource4
#define SWITCH_A_PIN_LINE            EXTI_Line4           /* Line 4 connects to all Px4 pins, et c */
#define SWITCH_A_IRQ                 EXTI4_IRQn
#define SWITCH_A_HANDLER             EXTI4_IRQHandler
#endif /* OWLMODULAR */

/* Expression input */
#if !(defined OWLMODULAR || defined OWLRACK)
#define EXPRESSION_PEDAL
#endif

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

#ifdef OWLMODULAR
#define PUSH_GATE_OUT_PIN            GPIO_Pin_7
#define PUSH_GATE_OUT_PORT           GPIOB
#define PUSH_GATE_OUT_CLK            RCC_AHB1Periph_GPIOB
#endif /* OWLMODULAR */

#define BYPASS_DEBOUNCE              200
#define PUSHBUTTON_DEBOUNCE          40

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

// todo:remove
#define PROGRAM_TASK_STACK_SIZE          (4*1024/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_STACK_SIZE          (512/sizeof(portSTACK_TYPE))
#define FLASH_TASK_STACK_SIZE            (512/sizeof(portSTACK_TYPE))
#define PC_TASK_STACK_SIZE               (512/sizeof(portSTACK_TYPE))
#define ARM_CYCLES_PER_SAMPLE            3500 /* 168MHz / 48kHz */
// todo:remove

#define IDLE_TASK_STACK_SIZE         (128/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_STACK_SIZE      (512/sizeof(portSTACK_TYPE))
#define UTILITY_TASK_STACK_SIZE      (512/sizeof(portSTACK_TYPE))
#define ARM_CYCLES_PER_SAMPLE        3500 /* 168MHz / 48kHz */

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

#endif // __device_h__
