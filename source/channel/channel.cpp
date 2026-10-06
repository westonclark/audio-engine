#include "./channel.h"
#include <cmath>

Channel::Channel() : ringBuffer(RING_BUFFER_FRAMES) { setGain(gain.load()); }

void Channel::setGain(double newGain) {
  gainRatio.store(std::pow(10.0, newGain / 20.0));
};

void Channel::loadFile(const std::string &path) {
  stream.open(path);
  ringBuffer.reset();
}

void Channel::fillRingBuffer() {
  if (!stream.isOpen()) {
    return;
  }

  while (!stream.isFinished() &&
         ringBuffer.getFreeSpace() >= STREAM_CHUNK_FRAMES) {
    const std::vector<float> &frames = stream.readFrames(STREAM_CHUNK_FRAMES);
    ringBuffer.write(frames.data(), frames.size());
  }
}

void Channel::prepare(uint32_t maxFrames) {
  channelBuffer.assign(maxFrames, 0.f);
}

void Channel::process(uint32_t frameCount) {
  float *samples = channelBuffer.data();

  size_t framesRead = ringBuffer.read(samples, frameCount);
  std::fill(samples + framesRead, samples + frameCount, 0.f);

  float ratio = gainRatio.load();
  for (uint32_t i = 0; i < framesRead; i++) {
    samples[i] *= ratio;
  }
}
