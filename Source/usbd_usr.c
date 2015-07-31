#include "usbd_usr.h"
#include "usbd_ioreq.h"

uint8_t usbd_usr_device_status = 0;

USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,
};

void USBD_USR_Init(void)
{
  usbd_usr_device_status = 0x01;
}

void USBD_USR_DeviceReset(uint8_t speed)
{
  usbd_usr_device_status = 0x02;
}

void USBD_USR_DeviceConfigured (void)
{
  usbd_usr_device_status = 0x03;
}

void USBD_USR_DeviceSuspended(void)
{
  usbd_usr_device_status = 0x00;
}

void USBD_USR_DeviceResumed(void)
{
  usbd_usr_device_status = 0x07;
}

void USBD_USR_DeviceConnected (void)
{
  usbd_usr_device_status = 0x0f;
}


void USBD_USR_DeviceDisconnected (void)
{
  usbd_usr_device_status = 0x00;
}
