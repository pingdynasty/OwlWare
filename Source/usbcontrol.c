#include "usbcontrol.h"
#include "device.h"
#include "clock.h"

#include "usb_conf.h"
#include "usbd_cdc_core.h"
#include "usbd_audio_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;

/* De-initialise the USB peripheral, for example before jumping
 * to the system bootloader
 */
void usb_deinit(void){
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Turn off discover pin: tells the computer USB is disconnected */
/*   USB_DISC_GPIO_PORT->BSRRL = USB_DISC_PIN; */

  /* Turn off interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_IRQ_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_IRQ_SUBPRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* De-initialise the USB subsystem */
  USBD_DeInit(&USB_OTG_dev);
}


/* Initialise the USB serial device, including the
 * relevant I/O pins. This code uses the full speed
 * USB peripheral on the STM32F4. There is a separate
 * "high speed" peripheral (though the onboard transceiver
 * is still limited to full speed) that could be used
 * in a future board revision, e.g. for USB host
 * capability.
 */
void usb_init(void){
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  uint32_t ticks;

  /* Enable clocks */
  RCC_AHB1PeriphClockCmd(USB_DATA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(USB_VBUS_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(USB_DISC_GPIO_CLK, ENABLE);

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

  /* Configure DISC pin */
  GPIO_InitStructure.GPIO_Pin = USB_DISC_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(USB_DISC_GPIO_PORT, &GPIO_InitStructure);

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

  /* Toggle the USB DISC line to tell the computer to
   * re-enumerate the device if it has already been
   * enumerated (might be important, for example, if
   * the device has come from DFU mode where the computer
   * will have recognised it as a DFU device).
   */
  USB_DISC_GPIO_PORT->BSRRL = USB_DISC_PIN; /* DISC high --> pullup off */

  /* Wait 100ms */
  delay(100);

  USB_DISC_GPIO_PORT->BSRRH = USB_DISC_PIN; /* DISC low --> pullup on */
}

/* Handler for USB interrupts */
void OTG_FS_IRQHandler(void){
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}
