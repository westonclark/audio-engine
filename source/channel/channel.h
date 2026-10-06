#pragma once

#include "../file/file.h"
#include "../ringbuffer/ringbuffer.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

const size_t RING_BUFFER_FRAMES = 262144; // ~5.5 seconds at 48kHz

class Channel {
public:
  Channel();

  AudioStream stream;

  RingBuffer ringBuffer;            // disk thread writes, audio thread reads
  std::vector<float> channelBuffer; // audio thread only, sized in prepare

  std::atomic<double> gain = 0;
  std::atomic<double> gainRatio;

  void setGain(double newGain);

  void loadFile(const std::string &path);
  void fillRingBuffer();

  void prepare(uint32_t maxFrames);
  void process(uint32_t frameCount);
};
