/**
  ******************************************************************************
  * @file    usbd_conf_template.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   usb device configuration template file
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

//#define USE_USB_OTG_HS  
#define USE_USB_OTG_FS

#define USBD_CFG_MAX_NUM                1
#define USBD_ITF_MAX_NUM                1
#define USBD_SELF_POWERED

#define MIDI_TOTAL_IF_NUM              0x02
#define MIDI_OUT_EP                    0x01
#define MIDI_IN_EP                     0x82

// created in STM32_USB_Device_Library/Core/src/usbd_req.c
// used in mbox_usb.c as temporary string buffer
#define USB_MAX_STR_DESC_SIZ            100
extern uint8_t USBD_StrDesc[USB_MAX_STR_DESC_SIZ];
#define APP_RX_DATA_SIZE                2048 /* Total size of IN buffer */


#endif //__USBD_CONF__H__

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

