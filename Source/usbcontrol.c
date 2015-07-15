#include "usbcontrol.h"
#include "device.h"
#include "clock.h"

#include "usb_conf.h"
#include "usbd_cdc_core.h"
#include "usbd_audio_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_dcd_int.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;

/* De-initialise the USB peripheral.
 * Turns off interrupt and de-initialises USB subsystem.
 */
void usb_deinit(void){
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Turn off interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_IRQ_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_IRQ_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* De-initialise the USB subsystem */
  USBD_DeInit(&USB_OTG_dev);
}

/* Initialise the USB serial device, 
 * including the relevant I/O pins.
 */
void usb_init(void){
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable clocks */
  RCC_AHB1PeriphClockCmd(USB_DATA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(USB_VBUS_GPIO_CLK, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;

  /* Configure USB data pins */
  GPIO_InitStructure.GPIO_Pin = USB_DP_PIN | USB_DM_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(USB_DATA_GPIO_PORT, &GPIO_InitStructure);

  GPIO_PinAFConfig(USB_DATA_GPIO_PORT, GPIO_PinSource11, GPIO_AF_OTG1_FS) ;
  GPIO_PinAFConfig(USB_DATA_GPIO_PORT, GPIO_PinSource12, GPIO_AF_OTG1_FS) ;

  /* Configure VBUS Pin */
  GPIO_InitStructure.GPIO_Pin = USB_VBUS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(USB_VBUS_GPIO_PORT, &GPIO_InitStructure);

  /* Configure and enable USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_IRQ_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_IRQ_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Initialise USB OTG device */
  USBD_Init(&USB_OTG_dev,
	    USB_OTG_FS_CORE_ID,
	    &USR_desc,
	    &AUDIO_cb, //&USBD_CDC_cb,
	    &USR_cb);
}

/* Handler for USB interrupts */
void OTG_FS_IRQHandler(void){
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}
