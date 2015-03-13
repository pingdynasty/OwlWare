TEMPLATEROOT = .

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
CFLAGS   = -g -Wall -Wcpp -Wunused-function
ASFLAGS  = -g
endif

ifeq ($(CONFIG),Release)
CFLAGS   = -O2
endif

CFLAGS  += --specs=nano.specs
CFLAGS  += -D__FPU_PRESENT=1 -D__FPU_USED=1 -DUSE_FULL_ASSERT
CFLAGS  += -DEXTERNAL_SRAM 
CFLAGS  += -fdata-sections -ffunction-sections -fno-omit-frame-pointer
CFLAGS  += -fno-builtin
# CFLAGS  += -nostdlib -nostartfiles -ffreestanding
# CFLAGS  += -mtune=cortex-m4
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 $(CFLAGS) 
CFLAGS  += -std=gnu99
# LDFLAGS = -flto -Wl,--gc-sections

# LDLIBS   = -lm
LDSCRIPT = Source/flash.ld

C_SRC  = codec.c i2s.c errorhandlers.c main.c basicmaths.c crc32.c # fsmc_sram.c 
# C_SRC += usb_dcd_int.c
C_SRC += system_hse.c
C_SRC += usbd_desc.c usb_bsp.c usbd_usr.c
C_SRC += usbd_audio_core.c 
C_SRC += armcontrol.c usbcontrol.c owlcontrol.c midicontrol.c eepromcontrol.c
C_SRC += clock.c operators.c gpio.c sysex.c # serial.c 
C_SRC += bkp_sram.c
# C_SRC += sramalloc.c

# FreeRTOS Source Files
FREERTOS_SRC = port.c
FREERTOS_SRC += list.c
FREERTOS_SRC += queue.c
FREERTOS_SRC += tasks.c
FREERTOS_SRC += event_groups.c
FREERTOS_SRC += timers.c
FREERTOS_SRC += heap_4.c

CPP_SRC  = Owl.cpp CodecController.cpp MidiController.cpp ApplicationSettings.cpp
CPP_SRC += PatchRegistry.cpp ProgramManager.cpp

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o) $(FREERTOS_SRC:%.c=Build/%.o)
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/MemMang
CFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/include
CFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F

# object files
OBJS += $(PERIPH) 
OBJS += $(BUILD)/startup.o
OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += libnosys_gnu.o
# OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
# OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_init_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

# include common make file
include $(TEMPLATEROOT)/Makefile.f4
