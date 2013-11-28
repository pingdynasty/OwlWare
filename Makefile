TEMPLATEROOT = .

# CSRCS = $(wildcard Source/*.c)
# CPPSRC = $(wildcard Source/*.cpp)
# COBJS = $(CSRCS:Source/%.c=Build/%.o)
# CPPOBJS = $(CPPSRCS:Source/%.cpp=Build/%.o)

# OBJS = $(COBJS) $(CPPOBJS)
# OBJS=$(SRCS:src/%.c=Build/%.o)


CFLAGS = -g -Wall -Wcpp -DUSE_FULL_ASSERT -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS += -DEXTERNAL_SRAM
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 $(CFLAGS) 
ASFLAGS  = -g
LDLIBS   = -lm -lstdc++
LDSCRIPT = Source/flash.ld

C_SRC  = codec.c i2s.c errorhandlers.c main.c basicmaths.c fsmc_sram.c
C_SRC += system_hse.c
C_SRC += usbd_desc.c usb_bsp.c usbd_usr.c
C_SRC += usbd_audio_core.c usbd_audio_out_if.c 
C_SRC += armcontrol.c usbcontrol.c owlcontrol.c midicontrol.c
C_SRC += clock.c operators.c serial.c gpio.c

# C_SRC += $(DSPLIB)/FastMathFunctions/arm_sin_f32.c $(DSPLIB)/FastMathFunctions/arm_cos_f32.c

CPP_SRC  = StompBox.cpp Owl.cpp CodecController.cpp MidiController.cpp ApplicationSettings.cpp 
CPP_SRC += PatchProcessor.cpp PatchRegistry.cpp

OBJS =  $(C_SRC:%.c=Build/%.o)  $(CPP_SRC:%.cpp=Build/%.o)

# object files
OBJS += $(PERIPH) 
OBJS += $(BUILD)/startup.o
# OBJS += system_hse.o
OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += $(SYSCALLS)
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += stm32f4xx_fsmc.o # external SRAM

# OBJS += $(BUILD)/stm32f4xx_flash.o # flash memory

# OBJS += codec.o i2s.o errorhandlers.o main.o basicmaths.o
# OBJS += clock.o operators.o serial.o gpio.o
# OBJS += armcontrol.o usbcontrol.o owlcontrol.o
# OBJS += midicontrol.o 
# OBJS += PatchRegistry.o # patchcontrol.o
# OBJS += StompBox.o Owl.o CodecController.o MidiController.o ApplicationSettings.o PatchProcessor.o
# OBJS += usbd_desc.o usb_bsp.o usbd_usr.o
# OBJS += usbd_audio_core.o usbd_audio_out_if.o

# include common make file
include $(TEMPLATEROOT)/Makefile.f4
