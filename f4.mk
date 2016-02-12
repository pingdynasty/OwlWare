# Library path
LIBROOT=$(TEMPLATEROOT)/Libraries

# Build path
BUILD=$(TEMPLATEROOT)/Build

# Code Paths
# DEVICE=$(LIBROOT)/CMSIS/Device/ST/STM32F4xx
DEVICE=$(LIBROOT)/CMSIS/ST/STM32F4xx
CMSIS=$(LIBROOT)/CMSIS/Include
PERIPH_FILE=$(LIBROOT)/STM32F4xx_StdPeriph_Driver
DSPLIB=$(LIBROOT)/CMSIS/DSP_Lib/Source
USB_DEVICE_FILE=$(LIBROOT)/STM32_USB_Device_Library
USB_HOST_FILE=$(LIBROOT)/STM32_USB_HOST_Library
USB_OTG_FILE=$(LIBROOT)/STM32_USB_OTG_Driver

# Processor specific
# LDSCRIPT ?= $(DEMO_FILE)/TrueSTUDIO/STM32F4-Discovery_Demo/stm32_flash.ld
# STARTUP ?= $(BUILD)/startup_stm32f4xx.o
# SYSTEM ?= $(BUILD)/system_stm32f4xx.o
PERIPH = $(BUILD)/stm32f4xx_gpio.o $(BUILD)/stm32f4xx_adc.o $(BUILD)/stm32f4xx_rcc.o $(BUILD)/stm32f4xx_syscfg.o $(BUILD)/stm32f4xx_pwr.o 
PERIPH += $(BUILD)/stm32f4xx_exti.o $(BUILD)/stm32f4xx_dac.o $(BUILD)/stm32f4xx_tim.o $(BUILD)/stm32f4xx_dma.o
PERIPH += $(BUILD)/stm32f4xx_usart.o
PERIPH += $(BUILD)/stm32f4xx_spi.o $(BUILD)/stm32f4xx_i2c.o $(BUILD)/stm32f4xx_dbgmcu.o 
PERIPH += $(BUILD)/stm32f4xx_flash.o $(BUILD)/stm32f4xx_fsmc.o
PERIPH += $(BUILD)/misc.o # stm32f4xx_comp.o 
USB_OTG = $(BUILD)/usb_dcd.o $(BUILD)/usb_core.o $(BUILD)/usb_dcd_int.o
USB_DEVICE = $(BUILD)/usbd_core.o $(BUILD)/usbd_ioreq.o $(BUILD)/usbd_req.o
USB_HOST = $(BUILD)/usbh_core.o $(BUILD)/usbh_ioreq.o $(BUILD)/usbh_stdreq.o $(BUILD)/usbh_hcs.o $(BUILD)/usb_hcd.o

# Compilation Flags
ARCH_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS += -fsingle-precision-constant
DEF_FLAGS = -DUSE_STDPERIPH_DRIVER -DARM_MATH_CM4 -D__FPU_PRESENT -D__FPU_USED=1 -DSTM32F407xx -DSTM32F4XX
INC_FLAGS = -I$(TEMPLATEROOT)/Libraries -I$(CMSIS) -I$(PERIPH_FILE)/inc -I$(TEMPLATEROOT)/Source
INC_FLAGS += -I$(DEVICE)/Include -I$(CMSIS)
INC_FLAGS += -I$(USB_DEVICE_FILE)/Core/inc # -I$(USB_DEVICE_FILE)/Class/cdc/inc
INC_FLAGS += -I$(USB_HOST_FILE)/Core/inc
INC_FLAGS += -I$(USB_OTG_FILE)/inc
CFLAGS += $(ARCH_FLAGS) $(INC_FLAGS) $(DEF_FLAGS)
CXXFLAGS += $(ARCH_FLAGS) $(INC_FLAGS) $(DEF_FLAGS)
LDFLAGS += -T$(LDSCRIPT) $(ARCH_FLAGS)

include $(TEMPLATEROOT)/common.mk
