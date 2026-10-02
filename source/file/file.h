#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

const size_t STREAM_CHUNK_FRAMES = 96000;

class AudioStream {
public:
  AudioStream() = default;
  AudioStream(const AudioStream &) = delete;
  AudioStream &operator=(const AudioStream &) = delete;
  ~AudioStream();

  uint32_t sampleRate = 0;
  uint16_t channels = 0;
  uint16_t bitsPerSample = 0;
  uint32_t totalFrames = 0;
  uint32_t currentFrame = 0;

  void open(const std::string &path);
  void close();
  void seek(uint32_t frame);
  const std::vector<float> &readFrames(size_t frames);

  bool isOpen() const;
  bool isFinished() const;

private:
  FILE *file = nullptr;
  off_t dataOffset = 0;
  uint16_t blockAlign = 0;
  bool isFloatData = false;
  float normalizationScale = 1.f;
  std::vector<float> frameData;
};

std::string readString(FILE *file, uint32_t length);
uint32_t readU32(FILE *file);
uint16_t readU16(FILE *file);
