TEMPLATEROOT = .

# SRCS=$(wildcard Source/*.c)
# OBJS=$(SRCS:src/%.c=Build/%.o)
# OBJS=$(SRCS:src/%.c=Build/%.o)

# Build/%.o: Source/%.c
# 	$(CC) -c $(CFLAGS) $< -o $@
# 	$(CC) -MM $(CFLAGS) $< > $*.d


CFLAGS = -g -Wall -Wcpp -DUSE_FULL_ASSERT -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS += -DEXTERNAL_SRAM
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 $(CFLAGS) 
ASFLAGS  = -g
LDLIBS   = -lm -lstdc++
LDSCRIPT = Source/flash.ld

# object files
OBJS = $(PERIPH) stm32f4xx_spi.o stm32f4xx_i2c.o stm32f4xx_dbgmcu.o
OBJS += startup.o
OBJS += system_hse.o
OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += $(SYSCALLS)
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o

OBJS += stm32f4xx_flash.o # flash memory

OBJS += fsmc_sram.o stm32f4xx_fsmc.o # external SRAM
OBJS += codec.o i2s.o errorhandlers.o main.o basicmaths.o
OBJS += clock.o operators.o serial.o gpio.o
OBJS += armcontrol.o usbcontrol.o owlcontrol.o
OBJS += midicontrol.o 
OBJS += PatchRegistry.o # patchcontrol.o
OBJS += StompBox.o Owl.o CodecController.o MidiController.o ApplicationSettings.o PatchProcessor.o
OBJS += usbd_desc.o usb_bsp.o usbd_usr.o
OBJS += usbd_audio_core.o usbd_audio_out_if.o

# include common make file
include $(TEMPLATEROOT)/Makefile.f4
