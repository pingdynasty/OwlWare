#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#include "usb_conf.h"

#define USBD_CFG_MAX_NUM                1
#define USBD_ITF_MAX_NUM                1
#define USBD_SELF_POWERED
#define USB_MAX_STR_DESC_SIZ            255

#define AUDIO_TOTAL_IF_NUM              0x02
#define AUDIO_OUT_EP                    0x01
#define AUDIO_IN_EP                     0x82

#define APP_RX_DATA_SIZE                1024 /* Total size of IN buffer */

#endif //__USBD_CONF__H__

