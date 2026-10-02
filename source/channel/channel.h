#pragma once

#include "../file/file.h"
#include "../ringbuffer/ringbuffer.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

const size_t RING_BUFFER_FRAMES = 262144;

const uint32_t MAX_BLOCK_FRAMES = 4096;

class Channel {
public:
  Channel();

  AudioStream stream;         // disk thread only (after loadFile)
  RingBuffer ringBuffer;      // disk thread writes, audio thread reads
  std::vector<float> processBuffer; // audio thread only

  std::atomic<double> gain = 0;
  std::atomic<double> gainRatio;

  void loadFile(const std::string &path);
  void fillBuffer();
  void process(uint32_t frameCount);
  void setGain(double newGain);
};
