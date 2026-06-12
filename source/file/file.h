#include <cstdint>
#include <iostream>

struct AudioFile {
  std::vector<float> samples;
  uint32_t sampleRate;
  uint16_t channels;
  uint16_t bitsPerSample;
};

AudioFile readAudioFile(std::string path);
std::string readString(FILE *file, uint32_t length);
uint32_t readU32(FILE *file);
uint16_t readU16(FILE *file);
