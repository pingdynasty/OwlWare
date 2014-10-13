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

enum PatchButtonId {
  BYPASS_BUTTON,
  PUSHBUTTON,
  GREEN_BUTTON,
  RED_BUTTON
};

class AudioBuffer {
public:
  virtual ~AudioBuffer();
  virtual float* getSamples(int channel) = 0;
  virtual int getChannels() = 0;
  virtual int getSize() = 0;
  virtual void clear() = 0;
};

class Patch {
public:
  Patch();
  virtual ~Patch();
  void registerParameter(PatchParameterId pid, const char* name, const char* description = "");
  float getParameterValue(PatchParameterId pid);
  bool isButtonPressed(PatchButtonId bid);
  void pressButton(PatchButtonId bid);
  int getBlockSize();
  double getSampleRate();
  AudioBuffer* createMemoryBuffer(int channels, int samples);
public:
  virtual void processAudio(AudioBuffer& output) = 0;
private:
  PatchProcessor* processor;
};

#endif // __StompBox_h__
