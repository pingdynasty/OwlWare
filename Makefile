TEMPLATEROOT = .

# SRCS=$(wildcard Source/*.c)
# OBJS=$(SRCS:src/%.c=Build/%.o)
# OBJS=$(SRCS:src/%.c=Build/%.o)

# Build/%.o: Source/%.c
# 	$(CC) -c $(CFLAGS) $< -o $@
# 	$(CC) -MM $(CFLAGS) $< > $*.d


CFLAGS = -g -Wall -Wcpp -DUSE_FULL_ASSERT -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS += -DEXTERNAL_SRAM
CFLAGS += -I$(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_Device_Library/Core/inc
CFLAGS += -I$(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_OTG_Driver/inc
CFLAGS += -I$(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_Device_Library/Class/cdc/inc
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 $(CFLAGS) 
ASFLAGS  = -g
LDLIBS   = -lm -lstdc++
LDSCRIPT = Source/flash.ld

# object files
OBJS = $(DISCOVERY) $(PERIPH) stm32f4xx_spi.o stm32f4xx_i2c.o stm32f4xx_dbgmcu.o
OBJS += startup.o
OBJS += system_hse.o
# OBJS =  $(STARTUP) $(DISCOVERY) $(PERIPH) $(SYSCALLS) main.o stm32f4xx_it.o
# OBJS = stm32f4xx_adc.o stm32f4xx_dma.o stm32f4xx_gpio.o stm32f4xx_i2c.o stm32f4xx_rcc.o stm32f4xx_spi.o system_stm32f4xx.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o

OBJS += stm32f4xx_flash.o # flash memory

OBJS += fsmc_sram.o stm32f4xx_fsmc.o # external SRAM
OBJS += $(SYSCALLS)
OBJS += codec.o i2s.o errorhandlers.o main.o basicmaths.o
OBJS += clock.o operators.o serial.o gpio.o
OBJS += armcontrol.o usbcontrol.o owlcontrol.o
OBJS += midicontrol.o 
OBJS += PatchRegistry.o # patchcontrol.o
OBJS += StompBox.o Owl.o CodecController.o MidiController.o ApplicationSettings.o PatchProcessor.o
OBJS += usbd_desc.o usb_bsp.o usbd_usr.o
OBJS += usbd_audio_core.o usbd_audio_out_if.o

OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_core.o
OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_ioreq.o
OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_req.o
OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_OTG_Driver/src/usb_dcd.o
OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_OTG_Driver/src/usb_core.o

OBJS += $(TEMPLATEROOT)/Libraries/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32_USB_OTG_Driver/src/usb_dcd_int.o

# include common make file
include $(TEMPLATEROOT)/Makefile.f4
