#include "./channel.h"
#include <cmath>

Channel::Channel()
    : ringBuffer(RING_BUFFER_FRAMES), processBuffer(MAX_BLOCK_FRAMES) {
  setGain(gain.load());
}

void Channel::loadFile(const std::string &path) {
  Channel &channel = *this;
  channel.stream.open(path);
  channel.ringBuffer.reset();
}

void Channel::fillBuffer() {
  Channel &channel = *this;
  if (!channel.stream.isOpen()) {
    return;
  }

  while (!channel.stream.isFinished() &&
         channel.ringBuffer.freeSpace() >= STREAM_CHUNK_FRAMES) {
    const std::vector<float> &frames = channel.stream.readFrames(STREAM_CHUNK_FRAMES);
    channel.ringBuffer.write(frames.data(), frames.size());
  }
}

void Channel::process(uint32_t frameCount) {
  Channel &channel = *this;
  float *samples = channel.processBuffer.data();

  size_t framesRead = channel.ringBuffer.read(samples, frameCount);
  std::fill(samples + framesRead, samples + frameCount, 0.f);

  float gainRatio = channel.gainRatio.load();
  for (uint32_t i = 0; i < framesRead; i++) {
    samples[i] *= gainRatio;
  }
}

void Channel::setGain(double newGain) {
  Channel &channel = *this;
  channel.gainRatio.store(std::pow(10.0, newGain / 20.0));
};
