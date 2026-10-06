#include "engine.h"
#include "../coreaudio/IOProc/IOProc.h"
#include <algorithm>
#include <chrono>
#include <cstdint>

AudioEngine::AudioEngine(int sampleRate, int bufferSize)
    : bufferSize(bufferSize), sampleRate(sampleRate), channels(8) {}

void AudioEngine::prepare() {
  setDeviceSampleRate(outputDevice.id, sampleRate);

  setDeviceBufferSize(outputDevice.id, bufferSize);

  for (Channel &channel : channels) {
    channel.prepare(bufferSize);
  }

  outputProcId = setDeviceCallback(outputDevice.id, coreAudioIOProc, this);

  for (Channel &channel : channels) {
    channel.fillRingBuffer();
  }
  startDiskThread();
}

void AudioEngine::play() {
  startDevice(outputDevice.id, outputProcId);
  isRunning = true;
}

void AudioEngine::stop() {
  stopDevice(outputDevice.id, outputProcId);
  isRunning = false;
}

void AudioEngine::teardown() {

  if (outputProcId != nullptr) {
    if (isRunning) {
      stop();
    }
    removeDeviceCallback(outputDevice.id, outputProcId);
    outputProcId = nullptr;
  }
  stopDiskThread();
}

void AudioEngine::startDiskThread() {
  if (diskThreadRunning.load()) {
    return;
  }
  diskThreadRunning.store(true);
  diskThread = std::thread(&AudioEngine::diskThreadLoop, this);
}

void AudioEngine::stopDiskThread() {
  diskThreadRunning.store(false);
  if (diskThread.joinable()) {
    diskThread.join();
  }
}

void AudioEngine::diskThreadLoop() {
  while (diskThreadRunning.load()) {
    for (Channel &channel : channels) {
      channel.fillRingBuffer();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void AudioEngine::process(const AudioBufferList *input, AudioBufferList *output,
                          uint32_t frameCount) {

  for (Channel &channel : channels) {
    channel.process(frameCount);
  }

  for (uint32_t i = 0; i < (*output).mNumberBuffers; i++) {
    AudioBuffer &buffer = (*output).mBuffers[i];
    float *outData = static_cast<float *>(buffer.mData);
    uint32_t outputChannels = buffer.mNumberChannels;

    for (uint32_t frame = 0; frame < frameCount; frame++) {
      float mixedValue = 0;
      for (Channel &channel : channels) {
        mixedValue += channel.channelBuffer[frame];
      }
      mixedValue = std::clamp(mixedValue, -1.f, 1.f);

      // Mono channels are sent equally to every output channel
      uint32_t outIndex = frame * outputChannels;
      for (uint32_t channel = 0; channel < outputChannels; channel++) {
        outData[outIndex + channel] = mixedValue;
      }
    }
  }
};

AudioEngine::~AudioEngine() { teardown(); }
