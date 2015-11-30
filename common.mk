# name of executable
ELF=$(BUILD)/OwlWare.elf                    
BIN=$(BUILD)/OwlWare.bin

# Tool path
TOOLROOT=$(TEMPLATEROOT)/Tools/gcc-arm-none-eabi-4_9-2015q2/bin
STLINK=$(TEMPLATEROOT)/Tools/stlink/
DFUUTIL=$(TEMPLATEROOT)/Tools/dfu-util/dfu-util

# Tools
CC=$(TOOLROOT)/arm-none-eabi-gcc
CXX=$(TOOLROOT)/arm-none-eabi-g++
LD=$(TOOLROOT)/arm-none-eabi-gcc
AR=$(TOOLROOT)/arm-none-eabi-ar
AS=$(TOOLROOT)/arm-none-eabi-as
GDB=$(TOOLROOT)/arm-none-eabi-gdb
OBJCOPY=$(TOOLROOT)/arm-none-eabi-objcopy
OBJDUMP=$(TOOLROOT)/arm-none-eabi-objdump
STFLASH=$(STLINK)/st-flash
STUTIL=$(STLINK)/st-util
BOOTLOADER ?= $(TEMPLATEROOT)/Libraries/OwlBoot/OwlBoot.bin

# Set up search path
vpath %.cpp $(TEMPLATEROOT)/Source
vpath %.c $(TEMPLATEROOT)/Source
vpath %.s $(TEMPLATEROOT)/Source
vpath %.c $(TEMPLATEROOT)/Libraries/syscalls
# vpath %.c $(CORE)
vpath %.c $(PERIPH_FILE)/src
vpath %.c $(PERIPH_FILE)/inc
vpath %.c $(DEVICE)
vpath %.c $(USB_DEVICE_FILE)/Core/src
vpath %.c $(USB_OTG_FILE)/src/

all: bin

# Build executable 
$(ELF) : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile library targets
%.o: %.c
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

%.o: %.S
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

# compile and generate dependency info
$(BUILD)/%.o: %.c
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	@$(CC) -MM -MT"$@" $(CPPFLAGS) $(CFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.cpp
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@
	@$(CXX) -MM -MT"$@" $(CPPFLAGS) $(CXXFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.s
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.c
	@$(CC) -S $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.cpp
	@$(CXX) -S $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	@$(OBJCOPY) -O binary $< $@

clean:
	@rm -f $(OBJS) $(BUILD)/*.d $(ELF) $(CLEANOTHER) $(BIN) $(ELF:.elf=.s) $(OBJS:.o=.s) gdbscript

debug: $(ELF)
	@echo "target extended localhost:4242" > gdbscript
	@echo "load $(ELF)" >> gdbscript
	$(GDB) -x gdbscript $(ELF)
# 	bash -c "$(GDB) -x <(echo target extended localhost:4242) $(ELF)"

flash:
	$(STFLASH) write $(BIN) 0x8008000

stlink:
	@echo "target extended localhost:4242" > gdbscript
	$(GDB) -x gdbscript $(ELF)

etags:
	find $(PERIPH_FILE) -type f -iname "*.[ch]" | xargs etags --append
	find $(DEVICE) -type f -iname "*.[ch]" | xargs etags --append
	find $(CORE) -type f -iname "*.[ch]" | xargs etags --append
	find $(DISCOVERY_FILE) -type f -iname "*.[ch]" | xargs etags --append
	find . -type f -iname "*.[ch]" | xargs etags --append

bin: $(BIN)
	@echo Successfully built OWL $(PLATFORM) $(CONFIG) firmware in $(BIN)

map : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -Wl,-Map=$(ELF:.elf=.map) $(OBJS) $(LDLIBS)

as: $(ELF)
	@$(OBJDUMP) -S $(ELF) > $(ELF:.elf=.s)

dfu: $(BIN)
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8008000:leave -D $(BIN)
	@echo Uploaded $(BIN) to OWL firmware

bootloader: $(BOOTLOADER)
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D $(BOOTLOADER)
	@echo Uploaded $(BOOTLOADER) to OWL bootsector

# pull in dependencies
-include $(OBJS:.o=.d)
