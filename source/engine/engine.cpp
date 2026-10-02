#include "engine.h"
#include "../coreaudio/IOProc/IOProc.h"
#include <algorithm>
#include <chrono>
#include <cstdint>

AudioEngine::AudioEngine(int sampleRate, int bufferSize)
    : bufferSize(bufferSize), sampleRate(sampleRate), channels(8) {}

void AudioEngine::prepare() {
  AudioEngine &engine = *this;
  setDeviceSampleRate(engine.outputDevice.id, engine.sampleRate);
  engine.outputProcId =
      setDeviceCallback(engine.outputDevice.id, coreAudioIOProc, &engine);

  for (Channel &channel : engine.channels) {
    channel.fillBuffer();
  }
  engine.startDiskThread();
}

void AudioEngine::play() {
  AudioEngine &engine = *this;
  startDevice(engine.outputDevice.id, engine.outputProcId);
  engine.isRunning = true;
}

void AudioEngine::stop() {
  AudioEngine &engine = *this;
  stopDevice(engine.outputDevice.id, engine.outputProcId);
  engine.isRunning = false;
}

void AudioEngine::teardown() {
  AudioEngine &engine = *this;

  if (engine.outputProcId != nullptr) {
    if (engine.isRunning) {
      engine.stop();
    }
    removeDeviceCallback(engine.outputDevice.id, engine.outputProcId);
    engine.outputProcId = nullptr;
  }
  engine.stopDiskThread();
}

void AudioEngine::startDiskThread() {
  AudioEngine &engine = *this;
  if (engine.diskThreadRunning.load()) {
    return;
  }
  engine.diskThreadRunning.store(true);
  engine.diskThread = std::thread(&AudioEngine::diskThreadLoop, &engine);
}

void AudioEngine::stopDiskThread() {
  AudioEngine &engine = *this;
  engine.diskThreadRunning.store(false);
  if (engine.diskThread.joinable()) {
    engine.diskThread.join();
  }
}

void AudioEngine::diskThreadLoop() {
  AudioEngine &engine = *this;
  while (engine.diskThreadRunning.load()) {
    for (Channel &channel : engine.channels) {
      channel.fillBuffer();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void AudioEngine::process(const AudioBufferList *input, AudioBufferList *output,
                          uint32_t frameCount) {
  AudioEngine &engine = *this;

  for (uint32_t blockStart = 0; blockStart < frameCount;
       blockStart += MAX_BLOCK_FRAMES) {
    uint32_t blockFrames = std::min(frameCount - blockStart, MAX_BLOCK_FRAMES);

    for (Channel &channel : engine.channels) {
      channel.process(blockFrames);
    }

    for (uint32_t i = 0; i < (*output).mNumberBuffers; i++) {
      AudioBuffer &buffer = (*output).mBuffers[i];
      float *outData = static_cast<float *>(buffer.mData);
      uint32_t outputChannels = buffer.mNumberChannels;

      for (uint32_t frame = 0; frame < blockFrames; frame++) {
        float mixedValue = 0;
        for (Channel &channel : engine.channels) {
          mixedValue += channel.processBuffer[frame];
        }
        mixedValue = std::clamp(mixedValue, -1.f, 1.f);

        uint32_t outIndex = (blockStart + frame) * outputChannels;
        for (uint32_t c = 0; c < outputChannels; c++) {
          outData[outIndex + c] = mixedValue;
        }
      }
    }
  }
};

AudioEngine::~AudioEngine() {
  AudioEngine &engine = *this;
  engine.teardown();
}
