TEMPLATEROOT = .

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  CPPFLAGS   = -g -Wall -Wcpp -Wunused-function -DDEBUG -DUSE_FULL_ASSERT
  ASFLAGS  = -g
endif

ifeq ($(CONFIG),Release)
  CPPFLAGS   = -O2
# CPPFLAGS += -flto
# LDFLAGS += -flto
endif

ifndef PLATFORM
  PLATFORM=Pedal
endif

ifeq ($(PLATFORM),Modular)
  CPPFLAGS   += -DOWLMODULAR
endif

LDFLAGS += -Wl,--gc-sections

CPPFLAGS  += --specs=nano.specs
CPPFLAGS  += -D__FPU_PRESENT=1 -D__FPU_USED=1
CPPFLAGS  += -DEXTERNAL_SRAM 
# CPPFLAGS  += -fpic -fpie
CPPFLAGS  += -fdata-sections
CPPFLAGS  += -ffunction-sections
# CPPFLAGS  += -fno-omit-frame-pointer
CPPFLAGS  += -fno-builtin
CPPFLAGS  += -nostdlib -nostartfiles -ffreestanding
CPPFLAGS  += -mtune=cortex-m4
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11 # $(CFLAGS) 
CFLAGS  += -std=gnu99

# LDLIBS   = -L$(BUILD) -lowl 
LDSCRIPT = Source/flash.ld

C_SRC  = codec.c i2s.c errorhandlers.c crc32.c # fsmc_sram.c 
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

CPP_SRC = main.cpp
CPP_SRC += Owl.cpp CodecController.cpp MidiController.cpp ApplicationSettings.cpp
CPP_SRC += PatchRegistry.cpp ProgramManager.cpp
CPP_SRC += FactoryPatches.cpp
CPP_SRC += ServiceCall.cpp

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o) $(FREERTOS_SRC:%.c=Build/%.o)
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/MemMang
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/include
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F

# object files
OBJS += $(PERIPH) 
OBJS += $(BUILD)/startup.o
OBJS += $(BUILD)/libnosys_gnu.o
OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
OBJS += $(DSPLIB)/CommonTables/arm_common_tables.o

OBJS += $(DSPLIB)/TransformFunctions/arm_cfft_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_cfft_radix8_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_bitreversal2.o
OBJS += $(DSPLIB)/TransformFunctions/arm_rfft_fast_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_rfft_fast_init_f32.o
OBJS += $(DSPLIB)/CommonTables/arm_const_structs.o

# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_init_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_f32.o

# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

# OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
# OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_init_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

vpath %.c $(TEMPLATEROOT)/ProgramSource
vpath %.cpp $(TEMPLATEROOT)/ProgramSource
CPPFLAGS += -I$(TEMPLATEROOT)/ProgramSource
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/OwlPatches

OBJS += $(BUILD)/basicmaths.o $(BUILD)/StompBox.o $(BUILD)/PatchProcessor.o # $(BUILD)/OwlProgram.o
OBJS += $(BUILD)/sramalloc.o

# include common make file
include $(TEMPLATEROOT)/f4.mk
