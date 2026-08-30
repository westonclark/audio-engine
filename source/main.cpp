#include "./coreaudio/device/device.h"
#include "./file/file.h"
#include "engine/engine.h"
#include <CoreAudio/CoreAudio.h>
#include <thread>

int main() {
  AudioEngine engine(44100, 128);
  engine.channels[0] = Channel{};
  engine.channels[0].audioFile =
      readAudioFile("./01 - Katy Perry - Never Really Over.wav");

  std::vector<AudioDevice> audioDevices = getAvailableDevices();
  for (AudioDevice device : audioDevices) {
    if (device.name == "MacBook Pro Speakers") {
      engine.outputDevice = device;
    }
  }

  engine.prepare();
  engine.play();
  std::this_thread::sleep_for(std::chrono::duration<double>(10));
  engine.stop();
  engine.teardown();

  return 0;
}
