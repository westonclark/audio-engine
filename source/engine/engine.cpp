#include "engine.h"
#include "../coreaudio/IOProc/IOProc.h"
#include <cstdint>

AudioEngine::AudioEngine(int sampleRate, int bufferSize)
    : bufferSize(bufferSize), sampleRate(sampleRate), channels(8) {}

void AudioEngine::prepare() {
  AudioEngine &engine = *this;
  setDeviceSampleRate(engine.outputDevice.id, engine.sampleRate);
  engine.outputProcId =
      setDeviceCallback(engine.outputDevice.id, coreAudioIOProc, &engine);
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

  if (engine.outputProcId == nullptr) {
    return;
  }
  if (engine.isRunning) {
    engine.stop();
  }
  removeDeviceCallback(engine.outputDevice.id, engine.outputProcId);
  engine.outputProcId = nullptr;
}

void AudioEngine::process(const AudioBufferList *input, AudioBufferList *output,
                          uint32_t frameCount) {
  AudioEngine &engine = *this;

  for (uint32_t i = 0; i < (*output).mNumberBuffers; i++) {
    AudioBuffer &buffer = (*output).mBuffers[i];
    float *outData = static_cast<float *>(buffer.mData);
    uint32_t sampleCount = frameCount * buffer.mNumberChannels;

    for (uint32_t j = 0; j < sampleCount; j++) {
      for (Channel &channel : engine.channels) {
        if (channel.readPosition < channel.audioFile.samples.size()) {
          int channelGain = channel.gain.load();
          float gainedValue = channel.audioFile.samples[channel.readPosition] *
                              std::pow(10.0, channelGain / 20.0);
          outData[j] = std::clamp(outData[j] += gainedValue, -1.f, 1.f);

          // if channel.audiofile.channels is 1, only increment on the even j's
          if (channel.audioFile.channels == 1) {
            if (j & 1) {
              channel.readPosition++;
            }
          } else {
            channel.readPosition++;
          }
        };
      }
    }
  }
};

AudioEngine::~AudioEngine() {
  AudioEngine &engine = *this;
  engine.teardown();
}
