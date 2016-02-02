#include "usbcontrol.h"
#include "device.h"
#include "clock.h"
#include "gpio.h"

#include "usb_conf.h"
#include "usbd_midi_core.h"
#include "usbh_midi_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_dcd_int.h"
#include <usbh_core.h>
#include <usbh_conf.h>

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;

#ifndef MIOS32_DONT_USE_USB_HOST
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END;
extern const USBH_Class_cb_TypeDef MIOS32_MIDI_USBH_Callbacks; // implemented in mios32_usb_midi.c
#endif

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

#ifndef MIOS32_DONT_USE_USB_HOST  
  /* Init Host Library */
  USBH_Init(&USB_OTG_dev, 
            USB_OTG_FS_CORE_ID,
            &USB_Host,
            (USBH_Class_cb_TypeDef *)&MIOS32_MIDI_USBH_Callbacks, 
            (USBH_Usr_cb_TypeDef *)&USBH_USR_Callbacks);
#endif

  // change connection state to disconnected
  USBD_USR_DeviceDisconnected();


  /* Initialise USB OTG device */
  USBD_Init(&USB_OTG_dev,
	    USB_OTG_FS_CORE_ID,
	    &USR_desc,
	    &MIDI_device_callbacks, //&USBD_CDC_cb,
	    &USR_cb);
    /* // init USB device and driver */
    /* USBD_Init(&USB_OTG_dev, */
    /* 	      USB_OTG_FS_CORE_ID, */
    /* 	      (USBD_DEVICE *)&USR_desc, */
    /* 	      (USBD_Class_cb_TypeDef *)&MIOS32_USB_CLASS_cb, */
    /* 	      (USBD_Usr_cb_TypeDef *)&USBD_USR_Callbacks); */

    // disconnect device
    DCD_DevDisconnect(&USB_OTG_dev);

    // wait 50 mS
    delay(50);

    // connect device
    DCD_DevConnect(&USB_OTG_dev);

#ifndef MIOS32_DONT_USE_USB_HOST
  // switch to host or device mode depending on the ID pin (Bootloader allows to overrule this pin)
//  if( MIOS32_USB_ForceDeviceMode() || MIOS32_SYS_STM_PINGET(GPIOA, GPIO_Pin_10) ) {
  if(getPin(USB_ID_GPIO_PORT, USB_ID_PIN) ) { // todo: check pin assignment
    USB_OTG_SetCurrentMode(&USB_OTG_dev, DEVICE_MODE);
  } else {
    USB_OTG_DriveVbus(&USB_OTG_dev, 1);
    USB_OTG_SetCurrentMode(&USB_OTG_dev, HOST_MODE);
  }
#endif
}

/* Handler for USB interrupts */
void OTG_FS_IRQHandler(void){
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
#ifndef MIOS32_DONT_USE_USB_HOST
  /*
  On-chip 5 V VBUS generation is not supported. For this reason, a charge pump 
  or, if 5 V are available on the application board, a basic power switch, must 
  be added externally to drive the 5 V VBUS line. The external charge pump can 
  be driven by any GPIO output. When the application decides to power on VBUS 
  using the chosen GPIO, it must also set the port power bit in the host port 
  control and status register (PPWR bit in OTG_FS_HPRT).
  
  Bit 12 PPWR: Port power
  The application uses this field to control power to this port, and the core 
  clears this bit on an overcurrent condition.
  */
  /* if (0 == state) */
  /* {  */
  /*   /\* DISABLE is needed on output of the Power Switch *\/ */
  /*   GPIO_SetBits(GPIOC, GPIO_Pin_0); */
  /* } */
  /* else */
  /* { */
  /*   /\*ENABLE the Power Switch by driving the Enable LOW *\/ */
  /*   GPIO_ResetBits(GPIOC, GPIO_Pin_0); */
  /* } */
#endif
}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  None
  * @retval None
  */
void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
#ifndef MIOS32_DONT_USE_USB_HOST
  /* GPIO_InitTypeDef GPIO_InitStructure;  */
  
  /* RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);   */
  
  /* GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; */
  /* GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; */
  /* GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; */
  /* GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; */
  /* GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; */
  /* GPIO_Init(GPIOC, &GPIO_InitStructure); */

  /* /\* By Default, DISABLE is needed on output of the Power Switch *\/ */
  /* GPIO_SetBits(GPIOC, GPIO_Pin_0); */
  
  /* delay(200);   /\* Delay is need for stabilising the Vbus Low in Reset Condition, when Vbus=1 and Reset-button is pressed by user *\/ */
#endif
}
