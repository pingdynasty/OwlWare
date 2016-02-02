#ifndef MIOS32_DONT_USE_USB_HOST
#include <usbh_core.h>
#include <usb_core.h>
#include <usbd_req.h>
#include <usb_regs.h>
#include <usbh_conf.h>
#include <usbh_ioreq.h>
#include <usbh_stdreq.h>
#include <usbh_hcs.h>

extern USBH_HOST USB_Host;

#if 0
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern uint32_t USB_rx_buffer[MIOS32_USB_MIDI_DATA_OUT_SIZE/4];
static uint32_t USB_tx_buffer[MIOS32_USB_MIDI_DATA_IN_SIZE/4];

// check USB_rx_buffer size
#if MIOS32_USB_MIDI_DATA_OUT_SIZE != USBH_MSC_MPS_SIZE
# error "MIOS32_USB_MIDI_DATA_OUT_SIZE and USBH_MSC_MPS_SIZE must be equal!"
#endif
#if MIOS32_USB_MIDI_DATA_IN_SIZE != USBH_MSC_MPS_SIZE
# error "MIOS32_USB_MIDI_DATA_IN_SIZE and USBH_MSC_MPS_SIZE must be equal!"
#endif

static u8  USBH_hc_num_in;
static u8  USBH_hc_num_out;
static u8  USBH_BulkOutEp;
static u8  USBH_BulkInEp;
static u8  USBH_BulkInEpSize;
static u8  USBH_tx_count;
static u16 USBH_BulkOutEpSize;

typedef enum {
  USBH_MIDI_IDLE,
  USBH_MIDI_RX,
  USBH_MIDI_TX,
} USBH_MIDI_transfer_state_t;

static USBH_MIDI_transfer_state_t USBH_MIDI_transfer_state;


/////////////////////////////////////////////////////////////////////////////
// Local prototypes
/////////////////////////////////////////////////////////////////////////////

static void MIOS32_USB_MIDI_TxBufferHandler(void);
static void MIOS32_USB_MIDI_RxBufferHandler(void);


/////////////////////////////////////////////////////////////////////////////
// Local Variables
/////////////////////////////////////////////////////////////////////////////

// Rx buffer
static u32 rx_buffer[MIOS32_USB_MIDI_RX_BUFFER_SIZE];
static volatile u16 rx_buffer_tail;
static volatile u16 rx_buffer_head;
static volatile u16 rx_buffer_size;
static volatile u8 rx_buffer_new_data;

// Tx buffer
static u32 tx_buffer[MIOS32_USB_MIDI_TX_BUFFER_SIZE];
static volatile u16 tx_buffer_tail;
static volatile u16 tx_buffer_head;
static volatile u16 tx_buffer_size;
static volatile u8 tx_buffer_busy;

// transfer possible?
static u8 transfer_possible = 0;


/////////////////////////////////////////////////////////////////////////////
//! Initializes USB MIDI layer
//! \param[in] mode currently only mode 0 supported
//! \return < 0 if initialisation failed
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_Init(u32 mode)
{
  // currently only mode 0 supported
  if( mode != 0 )
    return -1; // unsupported mode

  return 0; // no error
}


/////////////////////////////////////////////////////////////////////////////
//! This function is called by the USB driver on cable connection/disconnection
//! \param[in] connected status (1 if connected)
//! \return < 0 on errors
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_ChangeConnectionState(u8 connected)
{
  // in all cases: re-initialize USB MIDI driver
  // clear buffer counters and busy/wait signals again (e.g., so that no invalid data will be sent out)
  rx_buffer_tail = rx_buffer_head = rx_buffer_size = 0;
  rx_buffer_new_data = 0; // no data received yet
  tx_buffer_tail = tx_buffer_head = tx_buffer_size = 0;

  if( connected ) {
    transfer_possible = 1;
    tx_buffer_busy = 0; // buffer not busy anymore

#ifndef MIOS32_DONT_USE_USB_HOST
    USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
#endif
  } else {
    // cable disconnected: disable transfers
    transfer_possible = 0;
    tx_buffer_busy = 1; // buffer busy
  }

  return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
//! This function returns the connection status of the USB MIDI interface
//! \param[in] cable number
//! \return 1: interface available
//! \return 0: interface not available
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_CheckAvailable(u8 cable)
{
#ifdef MIOS32_SYS_ADDR_BSL_INFO_BEGIN
  if( MIOS32_USB_ForceSingleUSB() && cable >= 1 )
    return 0;
#endif

  if( cable >= MIOS32_USB_MIDI_NUM_PORTS )
    return 0;

  return transfer_possible ? 1 : 0;
}


/////////////////////////////////////////////////////////////////////////////
//! This function puts a new MIDI package into the Tx buffer
//! \param[in] package MIDI package
//! \return 0: no error
//! \return -1: USB not connected
//! \return -2: buffer is full
//!             caller should retry until buffer is free again
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_PackageSend_NonBlocking(mios32_midi_package_t package)
{
  // device available?
  if( !transfer_possible )
    return -1;

  // buffer full?
  if( tx_buffer_size >= (MIOS32_USB_MIDI_TX_BUFFER_SIZE-1) ) {
    // call USB handler, so that we are able to get the buffer free again on next execution
    // (this call simplifies polling loops!)
    MIOS32_USB_MIDI_TxBufferHandler();

    // device still available?
    // (ensures that polling loop terminates if cable has been disconnected)
    if( !transfer_possible )
      return -1;

    // notify that buffer was full (request retry)
    return -2;
  }

  // put package into buffer - this operation should be atomic!
  MIOS32_IRQ_Disable();
  tx_buffer[tx_buffer_head++] = package.ALL;
  if( tx_buffer_head >= MIOS32_USB_MIDI_TX_BUFFER_SIZE )
    tx_buffer_head = 0;
  ++tx_buffer_size;
  MIOS32_IRQ_Enable();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//! This function puts a new MIDI package into the Tx buffer
//! (blocking function)
//! \param[in] package MIDI package
//! \return 0: no error
//! \return -1: USB not connected
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_PackageSend(mios32_midi_package_t package)
{
  static u16 timeout_ctr = 0;
  // this function could hang up if USB is available, but MIDI port won't be
  // serviced by the host (e.g. windows: no program uses the MIDI IN port)
  // Therefore we time out the polling after 10000 tries
  // Once the timeout value is reached, each new MIDI_PackageSend call will
  // try to access the USB port only a single time anymore. Once the try
  // was successfull (MIDI port will be used by host), timeout value is
  // reset again

  s32 error;

  while( (error=MIOS32_USB_MIDI_PackageSend_NonBlocking(package)) == -2 ) {
    if( timeout_ctr >= 10000 )
      break;
    ++timeout_ctr;
  }

  if( error >= 0 ) // no error: reset timeout counter
    timeout_ctr = 0;

  return error;
}


/////////////////////////////////////////////////////////////////////////////
//! This function checks for a new package
//! \param[out] package pointer to MIDI package (received package will be put into the given variable)
//! \return -1 if no package in buffer
//! \return >= 0: number of packages which are still in the buffer
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_PackageReceive(mios32_midi_package_t *package)
{
  // package received?
  if( !rx_buffer_size )
    return -1;

  // get package - this operation should be atomic!
  MIOS32_IRQ_Disable();
  package->ALL = rx_buffer[rx_buffer_tail];
  if( ++rx_buffer_tail >= MIOS32_USB_MIDI_RX_BUFFER_SIZE )
    rx_buffer_tail = 0;
  --rx_buffer_size;
  MIOS32_IRQ_Enable();

  return rx_buffer_size;
}



/////////////////////////////////////////////////////////////////////////////
//! This function should be called periodically each mS to handle timeout
//! and expire counters.
//!
//! For USB MIDI it also checks for incoming/outgoing USB packages!
//!
//! Not for use in an application - this function is called from
//! MIOS32_MIDI_Periodic_mS(), which is called by a task in the programming
//! model!
//! 
//! \return < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_USB_MIDI_Periodic_mS(void)
{
  if( USB_OTG_IsHostMode(&USB_OTG_dev) ) {
#ifndef MIOS32_DONT_USE_USB_HOST
    // process the USB host events
    USBH_Process(&USB_OTG_dev, &USB_Host);
#endif
  } else {
    // check for received packages
    MIOS32_USB_MIDI_RxBufferHandler();

    // check for packages which should be transmitted
    MIOS32_USB_MIDI_TxBufferHandler();
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//! USB Device Mode
//!
//! This handler sends the new packages through the IN pipe if the buffer 
//! is not empty
/////////////////////////////////////////////////////////////////////////////
static void MIOS32_USB_MIDI_TxBufferHandler(void)
{
  // before using the handle: ensure that device (and class) already configured
  if( USB_OTG_dev.dev.class_cb == NULL )
    return;

  // send buffered packages if
  //   - last transfer finished
  //   - new packages are in the buffer
  //   - the device is configured

  // atomic operation to avoid conflict with other interrupts
  MIOS32_IRQ_Disable();

  if( !tx_buffer_busy && tx_buffer_size && transfer_possible ) {
    s16 count = (tx_buffer_size > (MIOS32_USB_MIDI_DATA_IN_SIZE/4)) ? (MIOS32_USB_MIDI_DATA_IN_SIZE/4) : tx_buffer_size;

    // notify that new package is sent
    tx_buffer_busy = 1;

    // send to IN pipe
    tx_buffer_size -= count;

    u32 *buf_addr = (u32 *)USB_tx_buffer;
    int i;
    for(i=0; i<count; ++i) {
      *(buf_addr++) = tx_buffer[tx_buffer_tail];
      if( ++tx_buffer_tail >= MIOS32_USB_MIDI_TX_BUFFER_SIZE )
	tx_buffer_tail = 0;
    }

    DCD_EP_Tx(&USB_OTG_dev, MIOS32_USB_MIDI_DATA_IN_EP, (uint8_t*)&USB_tx_buffer, count*4);
  }

  MIOS32_IRQ_Enable();
}


/////////////////////////////////////////////////////////////////////////////
//! USB Device Mode
//!
//! This handler receives new packages if the Tx buffer is not full
/////////////////////////////////////////////////////////////////////////////
static void MIOS32_USB_MIDI_RxBufferHandler(void)
{
  s16 count;

  // before using the handle: ensure that device (and class) already configured
  if( USB_OTG_dev.dev.class_cb == NULL ) {
    return;
  }

  // atomic operation to avoid conflict with other interrupts
  MIOS32_IRQ_Disable();

  // check if we can receive new data and get packages to be received from OUT pipe
  u32 ep_num = MIOS32_USB_MIDI_DATA_OUT_EP & 0x7f;
  USB_OTG_EP *ep = &USB_OTG_dev.dev.out_ep[ep_num];
  if( rx_buffer_new_data && (count=ep->xfer_count>>2) ) {
    // check if buffer is free
    if( count < (MIOS32_USB_MIDI_RX_BUFFER_SIZE-rx_buffer_size) ) {
      u32 *buf_addr = (u32 *)USB_rx_buffer;

      // copy received packages into receive buffer
      // this operation should be atomic
      do {
	mios32_midi_package_t package;
	package.ALL = *buf_addr++;

	if( MIOS32_MIDI_SendPackageToRxCallback(USB0 + package.cable, package) == 0 ) {
	  rx_buffer[rx_buffer_head] = package.ALL;

	  if( ++rx_buffer_head >= MIOS32_USB_MIDI_RX_BUFFER_SIZE )
	    rx_buffer_head = 0;
	  ++rx_buffer_size;
	}
      } while( --count > 0 );

      // notify, that data has been put into buffer
      rx_buffer_new_data = 0;

      // configuration for next transfer
      DCD_EP_PrepareRx(&USB_OTG_dev,
		       MIOS32_USB_MIDI_DATA_OUT_EP,
		       (uint8_t*)(USB_rx_buffer),
		       MIOS32_USB_MIDI_DATA_OUT_SIZE);
    }
  }

  MIOS32_IRQ_Enable();
}


/////////////////////////////////////////////////////////////////////////////
//! Called by STM32 USB Device driver to check for IN streams
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
//! \note also: bEP, bEPStatus only relevant for LPC17xx port
/////////////////////////////////////////////////////////////////////////////
void MIOS32_USB_MIDI_EP1_IN_Callback(u8 bEP, u8 bEPStatus)
{
  // package has been sent
  tx_buffer_busy = 0;

  // check for next package
  MIOS32_USB_MIDI_TxBufferHandler();
}

/////////////////////////////////////////////////////////////////////////////
//! Called by STM32 USB Device driver to check for OUT streams
//! \note Applications shouldn't call this function directly, instead please use \ref MIOS32_MIDI layer functions
//! \note also: bEP, bEPStatus only relevant for LPC17xx port
/////////////////////////////////////////////////////////////////////////////
void MIOS32_USB_MIDI_EP2_OUT_Callback(u8 bEP, u8 bEPStatus)
{
  // put package into buffer
  rx_buffer_new_data = 1;
  MIOS32_USB_MIDI_RxBufferHandler();
}

#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// USB Host Audio Class Callbacks
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief  USBH_MIDI_InterfaceInit
 *         Interface initialization for MSC class.
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval  USBH_Status :Response for USB MIDI driver intialization
 */
static USBH_Status USBH_InterfaceInit(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
  USBH_HOST *pphost = phost;

  MIOS32_USB_MIDI_ChangeConnectionState(0);

  int i;
  for(i=0; i<pphost->device_prop.Cfg_Desc.bNumInterfaces && i < USBH_MAX_NUM_INTERFACES; ++i) {
    //MIOS32_MIDI_DebugPortSet(UART0);
    //MIOS32_MIDI_SendDebugMessage("InterfaceInit %d %d %d", i, pphost->device_prop.Itf_Desc[i].bInterfaceClass, pphost->device_prop.Itf_Desc[i].bInterfaceSubClass);

    if( (pphost->device_prop.Itf_Desc[i].bInterfaceClass == 1) &&
	(pphost->device_prop.Itf_Desc[i].bInterfaceSubClass == 3) ) {

      if( pphost->device_prop.Ep_Desc[i][0].bEndpointAddress & 0x80 ) {
	USBH_BulkInEp = (pphost->device_prop.Ep_Desc[i][0].bEndpointAddress);
	USBH_BulkInEpSize  = pphost->device_prop.Ep_Desc[i][0].wMaxPacketSize;
      } else {
	USBH_BulkOutEp = (pphost->device_prop.Ep_Desc[i][0].bEndpointAddress);
	USBH_BulkOutEpSize  = pphost->device_prop.Ep_Desc[i] [0].wMaxPacketSize;
      }

      if( pphost->device_prop.Ep_Desc[i][1].bEndpointAddress & 0x80 ) {
	USBH_BulkInEp = (pphost->device_prop.Ep_Desc[i][1].bEndpointAddress);
	USBH_BulkInEpSize  = pphost->device_prop.Ep_Desc[i][1].wMaxPacketSize;
      } else {
	USBH_BulkOutEp = (pphost->device_prop.Ep_Desc[i][1].bEndpointAddress);
	USBH_BulkOutEpSize  = pphost->device_prop.Ep_Desc[i][1].wMaxPacketSize;
      }

      USBH_hc_num_out = USBH_Alloc_Channel(pdev, USBH_BulkOutEp);
      USBH_hc_num_in = USBH_Alloc_Channel(pdev, USBH_BulkInEp);

      /* Open the new channels */
      USBH_Open_Channel(pdev,
			USBH_hc_num_out,
			pphost->device_prop.address,
			pphost->device_prop.speed,
			EP_TYPE_BULK,
			USBH_BulkOutEpSize);

      USBH_Open_Channel(pdev,
			USBH_hc_num_in,
			pphost->device_prop.address,
			pphost->device_prop.speed,
			EP_TYPE_BULK,
			USBH_BulkInEpSize);

      MIOS32_USB_MIDI_ChangeConnectionState(1);
      break;
    }
  }

  if( MIOS32_USB_MIDI_CheckAvailable(0) ) {
    pphost->usr_cb->DeviceNotSupported();
  }
	
  return USBH_OK;

}


/**
 * @brief  USBH_InterfaceDeInit
 *         De-Initialize interface by freeing host channels allocated to interface
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval None
 */
static void USBH_InterfaceDeInit(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
  if( USBH_hc_num_out ) {
    USB_OTG_HC_Halt(pdev, USBH_hc_num_out);
    USBH_Free_Channel  (pdev, USBH_hc_num_out);
    USBH_hc_num_out = 0;     /* Reset the Channel as Free */
  }

  if( USBH_hc_num_in ) {
    USB_OTG_HC_Halt(pdev, USBH_hc_num_in);
    USBH_Free_Channel  (pdev, USBH_hc_num_in);
    USBH_hc_num_in = 0;     /* Reset the Channel as Free */
  }
}

/**
 * @brief  USBH_ClassRequest
 *         This function will only initialize the MSC state machine
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval  USBH_Status :Response for USB Set Protocol request
 */
static USBH_Status USBH_ClassRequest(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
  USBH_Status status = USBH_OK;
  return status;
}

/**
 * @brief  USBH_Handle
 *         MSC state machine handler
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval USBH_Status
 */
static USBH_Status USBH_Handle(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
  if( transfer_possible ) {
    USBH_HOST *pphost = phost;

    if( HCD_IsDeviceConnected(pdev) ) {

      u8 force_rx_req = 0;

      if( USBH_MIDI_transfer_state == USBH_MIDI_TX ) {
	URB_STATE URB_State = HCD_GetURB_State(pdev, USBH_hc_num_out);

        if( URB_State == URB_IDLE ) {
	  // wait...
	} else if( URB_State == URB_DONE ) {
	  USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	} else if( URB_State == URB_STALL ) {
	  // Issue Clear Feature on OUT endpoint
	  if( USBH_ClrFeature(pdev, pphost, USBH_BulkOutEp, USBH_hc_num_out) == USBH_OK ) {
	    USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	  }
	} else if( URB_State == URB_NOTREADY ) {
	  // send again
	  USBH_BulkSendData(&USB_OTG_dev, (u8 *)USB_tx_buffer, USBH_tx_count, USBH_hc_num_out);
	} else if( URB_State == URB_ERROR ) {
	  USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	}
      } else if( USBH_MIDI_transfer_state == USBH_MIDI_RX ) {
	URB_STATE URB_State = HCD_GetURB_State(pdev, USBH_hc_num_in);
        if( URB_State == URB_IDLE || URB_State == URB_DONE ) {
	  // data received from receive
	  //u32 count = HCD_GetXferCnt(pdev, USBH_hc_num_in) / 4;
	  // Note: HCD_GetXferCnt returns a counter which isn't zeroed immediately on a USBH_BulkReceiveData() call
	  u32 count = USB_OTG_dev.host.hc[USBH_hc_num_in].xfer_count / 4;

	  // push data into FIFO
	  if( !count ) {
	    USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	  } else if( count < (MIOS32_USB_MIDI_RX_BUFFER_SIZE-rx_buffer_size) ) {
	    u32 *buf_addr = (u32 *)USB_rx_buffer;

	    // copy received packages into receive buffer
	    // this operation should be atomic
	    MIOS32_IRQ_Disable();
	    do {
	      mios32_midi_package_t package;
	      package.ALL = *buf_addr++;

	      if( MIOS32_MIDI_SendPackageToRxCallback(USB0 + package.cable, package) == 0 ) {
		rx_buffer[rx_buffer_head] = package.ALL;

		if( ++rx_buffer_head >= MIOS32_USB_MIDI_RX_BUFFER_SIZE )
		  rx_buffer_head = 0;
		++rx_buffer_size;
	      }
	    } while( --count > 0 );
	    MIOS32_IRQ_Enable();

	    USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	    force_rx_req = 1;
	  }
	} else if( URB_State == URB_STALL ) {
	  // Issue Clear Feature on IN endpoint
	  if( USBH_ClrFeature(pdev, pphost, USBH_BulkInEp, USBH_hc_num_in) == USBH_OK ) {
	    USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	  }
	} else if( URB_State == URB_ERROR || URB_State == URB_NOTREADY ) {
	  USBH_MIDI_transfer_state = USBH_MIDI_IDLE;
	}
      }


      if( USBH_MIDI_transfer_state == USBH_MIDI_IDLE ) {
	if( !force_rx_req && tx_buffer_size && transfer_possible ) {
	  // atomic operation to avoid conflict with other interrupts
	  MIOS32_IRQ_Disable();

	  s16 count = (tx_buffer_size > (USBH_BulkOutEpSize/4)) ? (USBH_BulkOutEpSize/4) : tx_buffer_size;

	  // send to IN pipe
	  tx_buffer_size -= count;

	  u32 *buf_addr = (u32 *)USB_tx_buffer;
	  int i;
	  for(i=0; i<count; ++i) {
	    *(buf_addr++) = tx_buffer[tx_buffer_tail];
	    if( ++tx_buffer_tail >= MIOS32_USB_MIDI_TX_BUFFER_SIZE )
	      tx_buffer_tail = 0;
	  }
	  
	  USBH_tx_count = count * 4;
	  USBH_BulkSendData(&USB_OTG_dev, (u8 *)USB_tx_buffer, USBH_tx_count, USBH_hc_num_out);

	  USBH_MIDI_transfer_state = USBH_MIDI_TX;

	  MIOS32_IRQ_Enable();
	} else {
	  // request data from device
	  USBH_BulkReceiveData(&USB_OTG_dev, (u8 *)USB_rx_buffer, USBH_BulkInEpSize, USBH_hc_num_in);
	  USBH_MIDI_transfer_state = USBH_MIDI_RX;
	}
      }
    }
  }

  return USBH_OK;
}


const USBH_Class_cb_TypeDef MIOS32_MIDI_USBH_Callbacks = {
  USBH_InterfaceInit,
  USBH_InterfaceDeInit,
  USBH_ClassRequest,
  USBH_Handle
};

#endif /* MIOS32_DONT_USE_USB_HOST */
