#include <stdint.h>
#include "usbd_midi_core.h"
#include "midicontrol.h"

#ifndef MIOS32_DONT_USE_USB_HOST

#include <usbd_core.h>
#include <usbd_def.h>
#include <usbd_desc.h>
#include <usbd_req.h>
#include <usbd_conf.h>
#include <usbh_core.h>
#include <usbh_conf.h>
#include <usb_otg.h>
#include <usb_dcd_int.h>
#include <usb_hcd_int.h>
#include <usb_regs.h>

#include <string.h>

/**
 * @brief  USBH_USR_Init
 *         Displays the message on LCD for host lib initialization
 * @param  None
 * @retval None
 */
static void USBH_USR_Init(void)
{
}

/**
 * @brief  USBH_USR_DeviceAttached
 *         Displays the message on LCD on device attached
 * @param  None
 * @retval None
 */
static void USBH_USR_DeviceAttached(void)
{  
}

/**
 * @brief  USBH_USR_UnrecoveredError
 * @param  None
 * @retval None
 */
static void USBH_USR_UnrecoveredError (void)
{
}

/**
 * @brief  USBH_DisconnectEvent
 *         Device disconnect event
 * @param  None
 * @retval None
 */
static void USBH_USR_DeviceDisconnected (void)
{
#ifdef MIOS32_MIDI_USBH_DEBUG
  MIOS32_USB_MIDI_ChangeConnectionState(0);
#endif
}

/**
 * @brief  USBH_USR_ResetUSBDevice
 *         Reset USB Device
 * @param  None
 * @retval None
 */
static void USBH_USR_ResetDevice(void)
{
}


/**
 * @brief  USBH_USR_DeviceSpeedDetected
 *         Displays the message on LCD for device speed
 * @param  Devicespeed : Device Speed
 * @retval None
 */
static void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
}

/**
 * @brief  USBH_USR_Device_DescAvailable
 *         Displays the message on LCD for device descriptor
 * @param  DeviceDesc : device descriptor
 * @retval None
 */
static void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
}

/**
 * @brief  USBH_USR_DeviceAddressAssigned
 *         USB device is successfully assigned the Address
 * @param  None
 * @retval None
 */
static void USBH_USR_DeviceAddressAssigned(void)
{
}


/**
 * @brief  USBH_USR_Conf_Desc
 *         Displays the message on LCD for configuration descriptor
 * @param  ConfDesc : Configuration descriptor
 * @retval None
 */
static void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
						 USBH_InterfaceDesc_TypeDef *itfDesc,
						 USBH_EpDesc_TypeDef *epDesc)
{
}

/**
 * @brief  USBH_USR_Manufacturer_String
 *         Displays the message on LCD for Manufacturer String
 * @param  ManufacturerString : Manufacturer String of Device
 * @retval None
 */
static void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
#ifdef MIOS32_MIDI_USBH_DEBUG
  // Debug Output via UART0
  mios32_midi_port_t prev_port = MIOS32_MIDI_DebugPortGet();
  MIOS32_MIDI_DebugPortSet(UART0);
  MIOS32_MIDI_SendDebugMessage("[USBH_USR] Manufacturer: %s", ManufacturerString);
  MIOS32_MIDI_DebugPortSet(prev_port);
#endif
}

/**
 * @brief  USBH_USR_Product_String
 *         Displays the message on LCD for Product String
 * @param  ProductString : Product String of Device
 * @retval None
 */
static void USBH_USR_Product_String(void *ProductString)
{
#ifdef MIOS32_MIDI_USBH_DEBUG
  // Debug Output via UART0
  mios32_midi_port_t prev_port = MIOS32_MIDI_DebugPortGet();
  MIOS32_MIDI_DebugPortSet(UART0);
  MIOS32_MIDI_SendDebugMessage("[USBH_USR] Product: %s", ProductString);
  MIOS32_MIDI_DebugPortSet(prev_port);
#endif
}

/**
 * @brief  USBH_USR_SerialNum_String
 *         Displays the message on LCD for SerialNum_String
 * @param  SerialNumString : SerialNum_String of device
 * @retval None
 */
static void USBH_USR_SerialNum_String(void *SerialNumString)
{
#ifdef MIOS32_MIDI_USBH_DEBUG
  // Debug Output via UART0
  mios32_midi_port_t prev_port = MIOS32_MIDI_DebugPortGet();
  MIOS32_MIDI_DebugPortSet(UART0);
  MIOS32_MIDI_SendDebugMessage("[USBH_USR] Serial Number: %s", SerialNumString);
  MIOS32_MIDI_DebugPortSet(prev_port);
#endif
} 

/**
 * @brief  EnumerationDone
 *         User response request is displayed to ask for
 *         application jump to class
 * @param  None
 * @retval None
 */
static void USBH_USR_EnumerationDone(void)
{
} 

/**
 * @brief  USBH_USR_DeviceNotSupported
 *         Device is not supported
 * @param  None
 * @retval None
 */
static void USBH_USR_DeviceNotSupported(void)
{
}  


/**
 * @brief  USBH_USR_UserInput
 *         User Action for application state entry
 * @param  None
 * @retval USBH_USR_Status : User response for key button
 */
static USBH_USR_Status USBH_USR_UserInput(void)
{
  return USBH_USR_RESP_OK;
}

/**
 * @brief  USBH_USR_OverCurrentDetected
 *         Device Overcurrent detection event
 * @param  None
 * @retval None
 */
static void USBH_USR_OverCurrentDetected (void)
{
}

/**
* @brief  USBH_USR_MSC_Application 
*         Demo application for mass storage
* @param  None
* @retval Staus
*/
static int USBH_USR_Application(void)
{
  return (0);
}

/**
 * @brief  USBH_USR_DeInit
 *         Deinit User state and associated variables
 * @param  None
 * @retval None
 */
static void USBH_USR_DeInit(void)
{
}

 const USBH_Usr_cb_TypeDef USBH_USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};

#endif /* MIOS32_DONT_USE_USB */
