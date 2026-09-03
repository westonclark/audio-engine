#include "../file/file.h"

class Channel {
public:
  AudioFile audioFile;
  size_t readPosition = 0;

  std::atomic<int> gain = 0;

  void process() {};
};
