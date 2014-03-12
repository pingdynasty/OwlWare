TEMPLATEROOT = .

 CFLAGS = -g -Wall -Wcpp -DUSE_FULL_ASSERT -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS   = -O2 -Wall -Wcpp -DUSE_FULL_ASSERT -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS  += -DEXTERNAL_SRAM
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 $(CFLAGS) 
ASFLAGS  = -g
LDLIBS   = -lm
LDSCRIPT = Source/flash.ld

C_SRC  = codec.c i2s.c errorhandlers.c main.c basicmaths.c fsmc_sram.c
C_SRC += system_hse.c
C_SRC += usbd_desc.c usb_bsp.c usbd_usr.c
C_SRC += usbd_audio_core.c 
C_SRC += armcontrol.c usbcontrol.c owlcontrol.c midicontrol.c eepromcontrol.c
C_SRC += clock.c operators.c serial.c gpio.c sysex.c

CPP_SRC  = StompBox.cpp Owl.cpp CodecController.cpp MidiController.cpp PatchController.cpp ApplicationSettings.cpp
CPP_SRC += PatchProcessor.cpp PatchRegistry.cpp

OBJS =  $(C_SRC:%.c=Build/%.o)  $(CPP_SRC:%.cpp=Build/%.o)

# object files
OBJS += $(PERIPH) 
OBJS += $(BUILD)/startup.o
OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += $(SYSCALLS)
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

# include common make file
include $(TEMPLATEROOT)/Makefile.f4
