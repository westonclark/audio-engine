#include "./channel.h"
#include <cmath>

Channel::Channel()
    : ringBuffer(RING_BUFFER_FRAMES), processBuffer(MAX_BLOCK_FRAMES) {
  setGain(gain.load());
}

void Channel::loadFile(const std::string &path) {
  stream.open(path);
  ringBuffer.reset();
}

void Channel::fillBuffer() {
  if (!stream.isOpen()) {
    return;
  }

  while (!stream.isFinished() &&
         ringBuffer.freeSpace() >= STREAM_CHUNK_FRAMES) {
    const std::vector<float> &frames = stream.readFrames(STREAM_CHUNK_FRAMES);
    ringBuffer.write(frames.data(), frames.size());
  }
}

void Channel::process(uint32_t frameCount) {
  float *samples = processBuffer.data();

  size_t framesRead = ringBuffer.read(samples, frameCount);
  std::fill(samples + framesRead, samples + frameCount, 0.f);

  float ratio = gainRatio.load();
  for (uint32_t i = 0; i < framesRead; i++) {
    samples[i] *= ratio;
  }
}

void Channel::setGain(double newGain) {
  gainRatio.store(std::pow(10.0, newGain / 20.0));
};
