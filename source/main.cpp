#include "./coreaudio/device/device.h"
#include "cli/cli.h"
#include "engine/engine.h"
#include <CoreAudio/CoreAudio.h>

int main() {
  AudioEngine engine(48000, 128);

  std::vector<AudioDevice> audioDevices = getAvailableDevices();
  for (AudioDevice device : audioDevices) {
    if (device.name == "MacBook Pro Speakers") {
      engine.outputDevice = device;
    }
  }

  engine.channels[0].audioFile = readAudioFile("./media/01_Kick Out.wav");
  engine.channels[1].audioFile = readAudioFile("./media/02_Snare Top.wav");
  engine.channels[2].audioFile = readAudioFile("./media/03_Snare Down.wav");
  engine.channels[3].audioFile = readAudioFile("./media/04_Rack Tom.wav");
  engine.channels[4].audioFile = readAudioFile("./media/05_Floor Tom.wav");
  engine.channels[5].audioFile = readAudioFile("./media/06_Hi Hat.wav");
  engine.channels[6].audioFile = readAudioFile("./media/07_Overhead L.wav");
  engine.channels[7].audioFile = readAudioFile("./media/08_Overhead R.wav");

  engine.prepare();
  std::cout << "Audio Engine Started" << std::endl;

  Cli cli(engine);
  cli.start();

  engine.teardown();
  return 0;
}
