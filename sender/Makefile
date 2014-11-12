TARGET=FirmwareSender
BUILD=Build

# Tools
CC=gcc
CXX=g++
LD=g++

CFLAGS  += -g -I../sender -I../Source
CXXFLAGS = -std=c++11 $(CFLAGS) 
CFLAGS  += -std=gnu99
LDLIBS   = -lm -lpthread -ldl -lX11 -lasound

# Set up search path
vpath %.cpp ../../juce/modules/juce_core
vpath %.cpp ../../juce/modules/juce_audio_basics
vpath %.cpp ../../juce/modules/juce_audio_devices
vpath %.cpp ../../juce/modules/juce_events
vpath %.cpp ../Source
vpath %.c ../Source
vpath %.cpp ../sender
vpath %.cpp ../sender

C_SRC  = sysex.c crc32.c
CPP_SRC  = FirmwareSender.cpp 
CPP_SRC += juce_core.cpp juce_audio_basics.cpp juce_audio_devices.cpp juce_events.cpp

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o)

all: $(TARGET)

# Build executable 
$(TARGET) : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile and generate dependency info
$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -MM -MT"$@" $(CFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	$(CXX) -MM -MT"$@" $(CXXFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.s
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.cpp
	$(CXX) -S $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(BUILD)/*.d $(TARGET) $(OBJS:.o=.s) 

map : $(OBJS) $(LDSCRIPT) $(TANN_LIB)
	$(LD) $(LDFLAGS) -Wl,-Map=$(ELF:.elf=.map) $(OBJS) $(LDLIBS)

# pull in dependencies
-include $(OBJS:.o=.d)
