/*
  g++ -std=c++11 -Isender sender/FirmwareSender.cpp Source/sysex.c ../OwlNest/JuceLibraryCode/modules/juce_core/juce_core.cpp ../OwlNest/JuceLibraryCode/modules/juce_audio_basics/juce_audio_basics.cpp ../OwlNest/JuceLibraryCode/modules/juce_audio_devices/juce_audio_devices.cpp ../OwlNest/JuceLibraryCode/modules/juce_events/juce_events.cpp -lpthread -ldl -lX11 -lasound
*/
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include "../../OwlNest/JuceLibraryCode/JuceHeader.h"
#include "../Source/OpenWareMidiControl.h"
#include "../Source/CRCC.hpp"
#include "../Source/sysex.h"
#include "../Source/MidiStatus.h"

#define BINARY_BLOCKSIZE 210 // max size of binary data per sysex message (224b binary = 256b sysex)
#define DEFAULT_BLOCK_DELAY 20 // wait in milliseconds between sysex messages

class CommandLineException : public std::exception {
private:
  juce::String cause;
public:
  CommandLineException(juce::String c) : cause(c) {}
  juce::String getCause() const {
    return cause;
  }
  const char* what() const noexcept {
    return getCause().toUTF8();
  }
};

class FirmwareUploadApplication {
private:
  bool running = false;
  bool verbose = true;
  juce::ScopedPointer<MidiOutput> midiout;
  juce::ScopedPointer<File> fileout;
  juce::ScopedPointer<File> input;
  juce::ScopedPointer<OutputStream> out;
  int blockDelay = DEFAULT_BLOCK_DELAY;
public:
  void listDevices(const StringArray& names){
    for(int i=0; i<names.size(); ++i)
      std::cout << i << ": " << names[i] << std::endl;
  }

  MidiOutput* openMidiOutput(const String& name){
    MidiOutput* output = NULL;    
    StringArray outputs = MidiOutput::getDevices();
    for(int i=0; i<outputs.size(); ++i){
      if(outputs[i].trim().matchesWildcard(name, true)){
	if(verbose)
	  std::cout << "opening MIDI output " << outputs[i] << std::endl;
	output = MidiOutput::openDevice(i);
	break;
      }
    }
    if(output != NULL)
      output->startBackgroundThread();
    return output;
  }

  void send(MemoryBlock& block){
    send((unsigned char*)block.getData(), block.getSize());
  }

  void send(unsigned char* data, int size){
    if(verbose)
      std::cout << "sending " << std::dec << size << " bytes" << std::endl;
    if(out != NULL){
      out->writeByte(SYSEX);
      out->write(data, size);
      out->writeByte(SYSEX_EOX);
      out->flush();
    }
    if(midiout != NULL)
      midiout->sendMessageNow(juce::MidiMessage::createSysExMessage(data, size));
  }

  void usage(){
    std::cerr << getApplicationName() << std::endl 
	      << "usage:" << std::endl
	      << "-h or --help\tprint this usage information and exit" << std::endl
	      << "-l or --list\tlist available MIDI ports and exit" << std::endl
	      << "-in FILE\tinput FILE" << std::endl
	      << "-out DEVICE\tsend output to MIDI interface DEVICE" << std::endl
	      << "-save FILE\twrite output to FILE" << std::endl
	      << "-d NUM\tdelay for NUM milliseconds between blocks" << std::endl
	      << "-q or --quiet\treduce status output" << std::endl
	      << "-v or --verbose\tincrease status output" << std::endl
      ;
  }

  void configure(int argc, char* argv[]) {
    for(int i=1; i<argc; ++i){
      juce::String arg = juce::String(argv[i]);
      if(arg.compare("-h") == 0 || arg.compare("--help") == 0 ){
	usage();
	throw CommandLineException(juce::String::empty);
      }else if(arg.compare("-q") == 0 || arg.compare("--quiet") == 0 ){
	verbose = false;
      }else if(arg.compare("-v") == 0 || arg.compare("--verbose") == 0 ){
	verbose = true;
      }else if(arg.compare("-l") == 0 || arg.compare("--list") == 0){
	std::cout << "MIDI input devices:" << std::endl;
	listDevices(MidiInput::getDevices());
	std::cout << "MIDI output devices:" << std::endl;
	listDevices(MidiOutput::getDevices());
	throw CommandLineException(juce::String::empty);
      }else if(arg.compare("-d") == 0 && ++i < argc){
	blockDelay = juce::String(argv[i]).getIntValue();
      }else if(arg.compare("-in") == 0 && ++i < argc){
	juce::String name = juce::String(argv[i]);
	input = new juce::File(name);
      }else if(arg.compare("-out") == 0 && ++i < argc){
	juce::String name = juce::String(argv[i]);
	midiout = openMidiOutput(name);
      }else if(arg.compare("-save") == 0 && ++i < argc){
	juce::String name = juce::String(argv[i]);
	fileout = new juce::File(name);
	fileout->deleteFile();
	fileout->create();
      }else{
	usage();
	throw CommandLineException(juce::String::empty);
      }
    }
    if(input == NULL || (midiout == NULL && fileout == NULL)){
      usage();
      throw CommandLineException(juce::String::empty);
    }
  }

  void run(){
    running = true;
    if(verbose){
      std::cout << "Sending file " << input->getFileName() << std::endl; 
      if(midiout != NULL)
	std::cout << "\tto MIDI output" << std::endl; 
      if(fileout != NULL)
	std::cout << "\tto SysEx file " << fileout->getFullPathName() << std::endl;       
    }
    char header[] =  { MIDI_SYSEX_MANUFACTURER, MIDI_SYSEX_DEVICE, SYSEX_FIRMWARE_UPLOAD };
    int blocksize = BINARY_BLOCKSIZE;

    InputStream* in = input->createInputStream();
    if(fileout != NULL)
      out = fileout->createOutputStream();

    int packageIndex = 0;
    MemoryBlock block;
    block.append(header, sizeof(header));
    encodeInt(block, packageIndex++);
    // MemoryOutputStream stream;
    // stream.write(header, sizeof(header));

    unsigned char buffer[blocksize];
    unsigned char sysex[(int)ceil(blocksize*8/7)];
    int size = input->getSize(); // amount of data, excluding checksum
    encodeInt(block, size);
    // buffer[3] = (uint8_t)size & 0xff;
    // buffer[2] = (uint8_t)(size >> 8) & 0xff;
    // buffer[1] = (uint8_t)(size >> 16) & 0xff;
    // buffer[0] = (uint8_t)(size >> 24) & 0xff;
    // int len = data_to_sysex(buffer, sysex, 4);
    // if(len != 5)
    //   throw CommandLineException("Error in sysex conversion"); 
    // block.append(sysex, len);
    // stream.write(sysex, len);

    CRCC crc;
    uint32_t checksum = 0;
    // size = input->getSize(); // amount of data, excluding checksum
    for(int i=0; i < size && running;){
      int len = in->read(buffer, blocksize);
      checksum = crc.calc(len, buffer, checksum);
      i += len;
      if(verbose)
	std::cout << "preparing " << std::dec << len;
      len = data_to_sysex(buffer, sysex, len);
      if(verbose)
	std::cout << "/" << len << " bytes binary/sysex (total " << 
	  i << " of " << size << " bytes)" << std::endl;
      block.append(sysex, len);
      // stream.write(sysex, len);
      if(i == size){
	// last block
	if(verbose)
	  std::cout << "checksum 0x" << std::hex << checksum << std::endl;
	encodeInt(block, checksum);
	// buffer[3] = (uint8_t)checksum & 0xff;
	// buffer[2] = (uint8_t)(checksum >> 8) & 0xff;
	// buffer[1] = (uint8_t)(checksum >> 16) & 0xff;
	// buffer[0] = (uint8_t)(checksum >> 24) & 0xff;
	// len = data_to_sysex(buffer, sysex, 4);
	// if(len != 5)
	//   throw CommandLineException("Error in sysex conversion"); 
	// block.append(sysex, len);
	send(block);
	// stream.write(sysex, len);
	// send((unsigned char*)stream.getData(), stream.getDataSize());
      }else{
	send(block);
	block = MemoryBlock();
	block.append(header, sizeof(header));
	encodeInt(block, packageIndex++);
	// send((unsigned char*)stream.getData(), stream.getDataSize());
	// stream.reset();
	// stream.write(header, sizeof(header));
      }
      juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter()+blockDelay);
    }
    stop();
  }

  void encodeInt(MemoryBlock& block, uint32_t data){
    uint8_t in[4];
    uint8_t out[5];
    in[3] = (uint8_t)data & 0xff;
    in[2] = (uint8_t)(data >> 8) & 0xff;
    in[1] = (uint8_t)(data >> 16) & 0xff;
    in[0] = (uint8_t)(data >> 24) & 0xff;
    int len = data_to_sysex(in, out, 4);
    if(len != 5)
      throw CommandLineException("Error in sysex conversion"); 
    block.append(out, len);
  }

  void stop(){
    if(midiout != NULL)
      midiout->stopBackgroundThread();
    if(out != NULL)
      out->flush();
  }

  void shutdown(){
    running = false;
  }

  juce::String getApplicationName(){
    return "FirmwareSender";
  }
};

FirmwareUploadApplication app;

void sigfun(int sig){
  std::cout << "shutting down" << std::endl;
  app.shutdown();
  (void)signal(SIGINT, SIG_DFL);
}

int main(int argc, char* argv[]) {
  (void)signal(SIGINT, sigfun);
  int status = 0;
  try{
    app.configure(argc, argv);
    app.run();
  }catch(const std::exception& exc){
    std::cerr << exc.what() << std::endl;
    status = -1;
  }
  return status;
}
