#include "./coreaudio/device/device.h"
#include "./file/file.h"
#include "engine/engine.h"
#include <CoreAudio/CoreAudio.h>
#include <thread>

int main() {
  AudioEngine engine(48000, 128);

  engine.channels[0].audioFile = readAudioFile("./media/01_Kick Out.wav");
  engine.channels[1].audioFile = readAudioFile("./media/02_Snare Top.wav");
  engine.channels[2].audioFile = readAudioFile("./media/03_Snare Down.wav");
  engine.channels[3].audioFile = readAudioFile("./media/04_Rack Tom.wav");
  engine.channels[4].audioFile = readAudioFile("./media/05_Floor Tom.wav");
  engine.channels[5].audioFile = readAudioFile("./media/06_Hi Hat.wav");
  engine.channels[6].audioFile = readAudioFile("./media/07_Overhead L.wav");
  engine.channels[7].audioFile = readAudioFile("./media/08_Overhead R.wav");

  std::vector<AudioDevice> audioDevices = getAvailableDevices();
  for (AudioDevice device : audioDevices) {
    if (device.name == "MacBook Pro Speakers") {
      engine.outputDevice = device;
    }
  }

  engine.prepare();
  engine.play();
  std::this_thread::sleep_for(std::chrono::duration<double>(30));
  engine.stop();
  engine.teardown();

  return 0;
}
