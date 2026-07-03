#include "./utils/utils.h"
#include <cstdint>
#include <iostream>

struct AudioFile {
  std::vector<float> samples;
  uint32_t sampleRate;
  uint16_t channels;
  uint16_t bitsPerSample;
};

AudioFile readAudioFile(std::string path);
