#ifndef __StompBox_h__
#define __StompBox_h__

#include <string>
class PatchProcessor;

enum PatchParameterId {
  PARAMETER_A,
  PARAMETER_B,
  PARAMETER_C,
  PARAMETER_D,
  PARAMETER_E,
  PARAMETER_F
};

class AudioBuffer {
public:
  virtual ~AudioBuffer();
  virtual float* getSamples(int channel) = 0;
  virtual int getChannels() = 0;
  virtual int getSize() = 0;
};

class Patch {
public:
  Patch();
  virtual ~Patch();
  void registerParameter(PatchParameterId pid, const std::string& name, const std::string& description = "");
  float getParameterValue(PatchParameterId pid);
  int getBlockSize();
  double getSampleRate();
  AudioBuffer* createMemoryBuffer(int channels, int samples);
public:
  virtual void processAudio(AudioBuffer& output) = 0;
private:
  PatchProcessor* processor;
};

#endif // __StompBox_h__
