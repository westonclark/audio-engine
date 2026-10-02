#pragma once

#include "../channel/channel.h"
#include "../coreaudio/device/device.h"
#include <CoreAudio/CoreAudio.h>
#include <atomic>
#include <thread>
#include <vector>

class AudioEngine {
public:
  AudioEngine(int sampleRate, int bufferSize);

  float sampleRate;
  int bufferSize;

  AudioDevice inputDevice;
  AudioDeviceIOProcID inputProcId = nullptr;

  AudioDevice outputDevice;
  AudioDeviceIOProcID outputProcId = nullptr;

  bool isRunning = false;

  std::vector<Channel> channels;

  std::thread diskThread;
  std::atomic<bool> diskThreadRunning = false;

  void prepare();
  void play();
  void stop();
  void teardown();
  void process(const AudioBufferList *input, AudioBufferList *output,
               uint32_t frameCount);

  void startDiskThread();
  void stopDiskThread();
  void diskThreadLoop();

  ~AudioEngine();
};
