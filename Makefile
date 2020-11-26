TEMPLATEROOT = .

ifndef CONFIG
  CONFIG = Debug
endif

ifndef PLATFORM
  PLATFORM=Pedal
endif

ifeq ($(CONFIG),Debug)
  CPPFLAGS = -g -Wall -Wcpp -Wunused-function -DDEBUG -DUSE_FULL_ASSERT
  ASFLAGS  = -g
endif

ifeq ($(CONFIG),Release)
  CPPFLAGS = -O2
#  CPPFLAGS += -flto
#  LDFLAGS += -flto
endif

ifeq ($(PLATFORM),Modular)
  CPPFLAGS   += -DOWLMODULAR
endif

LDFLAGS += -Wl,--gc-sections
LDSCRIPT = Source/flash.ld

CPPFLAGS += --specs=nano.specs
CPPFLAGS += -DEXTERNAL_SRAM -DARM_CORTEX
# CPPFLAGS += -fpic -fpie
CPPFLAGS += -fdata-sections
CPPFLAGS += -ffunction-sections
#CPPFLAGS +=  -mno-unaligned-access
#CPPFLAGS  += -fno-omit-frame-pointer
CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding
CPPFLAGS += -mtune=cortex-m4
CXXFLAGS = -fno-rtti -fno-exceptions -std=gnu++11
CFLAGS  += -std=gnu99

C_SRC  = codec.c i2s.c errorhandlers.c crc32.c # fsmc_sram.c 
# C_SRC += usb_dcd_int.c
C_SRC += system_hse.c
C_SRC += usbd_desc.c usb_bsp.c usbd_usr.c
C_SRC += usbd_audio_core.c 
C_SRC += armcontrol.c usbcontrol.c owlcontrol.c midicontrol.c eepromcontrol.c
C_SRC += clock.c operators.c gpio.c sysex.c # serial.c 
C_SRC += bkp_sram.c
C_SRC += sramalloc.c
C_SRC += basicmaths.c

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
CPP_SRC += FactoryPatches.cpp ServiceCall.cpp
CPP_SRC += PatchProcessor.cpp StompBox.cpp FloatArray.cpp

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o) $(FREERTOS_SRC:%.c=Build/%.o)
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F
vpath %.c $(TEMPLATEROOT)/Libraries/FreeRTOS/portable/MemMang
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/include
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/FreeRTOS/portable/GCC/ARM_CM4F

vpath %.c $(TEMPLATEROOT)/ProgramSource
vpath %.cpp $(TEMPLATEROOT)/ProgramSource
CPPFLAGS += -I$(TEMPLATEROOT)/ProgramSource
CPPFLAGS += -I$(TEMPLATEROOT)/Libraries/OwlPatches

include $(TEMPLATEROOT)/libs.mk
include $(TEMPLATEROOT)/f4.mk

DFUCMD = $(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0
deploy: bootloader
	@echo Flashing OWL $(PLATFORM) firmware
	$(DFUCMD) -s 0x080E0000 -D binaries/patch0.bin
	$(DFUCMD) -s 0x080C0000 -D binaries/patch1.bin
	$(DFUCMD) -s 0x080A0000 -D binaries/patch2.bin
	$(DFUCMD) -s 0x08080000 -D binaries/patch3.bin
	$(DFUCMD) -s 0x8008000:leave -D binaries/OwlWare-v12-$(PLATFORM).bin


bootloader: $(BOOTLOADER)
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D $(BOOTLOADER)
	@echo Uploaded $(BOOTLOADER) to OWL bootsector

BOOTLOADER ?= $(TEMPLATEROOT)/Libraries/OwlBoot/OwlBoot.bin

.PHONY: clean bootloader deploy program-info program-lock program-unlock program-bootloader program-firmware program-patches program-all program-erase

program-info:
	openocd -f openocd.cfg -c "init" -c "flash info 0" -c "exit"

program-lock:
	openocd -f openocd.cfg -c "init" -c "halt" -c "flash protect 0 0 1 on" -c "flash info 0" -c "exit"

program-unlock:
	openocd -f openocd.cfg -c "init" -c "reset halt" -c "stm32f4x unlock 0" -c "reset halt" -c "exit"
	openocd -f openocd.cfg -c "init" -c "halt" -c "flash protect 0 0 11 off" -c "flash info 0" -c "exit"

program-bootloader:
	openocd -f openocd.cfg -c "program $(BOOTLOADER) verify exit 0x08000000"

program-firmware:
	openocd -f openocd.cfg -c "program binaries/OwlWare-v12-$(PLATFORM).bin verify exit 0x08008000"

program-patches:
	openocd -f openocd.cfg -c "program binaries/patch0.bin verify 0x080E0000" \
	                       -c "program binaries/patch1.bin verify 0x080C0000" \
	                       -c "program binaries/patch2.bin verify 0x080A0000" \
	                       -c "program binaries/patch3.bin verify 0x08080000" -c "exit"

program-all: program-bootloader program-firmware program-patches program-lock

program-erase: program-unlock
	openocd -f openocd.cfg -c "init" -c "reset halt" -c "stm32f4x mass_erase 0" -c "flash info 0" -c "exit"
