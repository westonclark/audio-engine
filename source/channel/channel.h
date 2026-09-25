#include "../file/file.h"

class Channel {
public:
  AudioFile audioFile;
  size_t readPosition = 0;

  std::atomic<double> gain = 0;
  std::atomic<double> gainRatio;

  Channel();

  void process();
  void setGain(double newGain);
};
