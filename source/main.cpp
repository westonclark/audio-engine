#include "./coreaudio/device/device.h"
#include "cli/cli.h"
#include "engine/engine.h"
#include <CoreAudio/CoreAudio.h>
#include <iostream>

int main() {
  AudioEngine engine(48000, 128);
  Cli cli(engine);

  std::vector<AudioDevice> audioDevices = getAvailableDevices();
  for (AudioDevice device : audioDevices) {
    if (device.name == "MacBook Pro Speakers") {
      engine.outputDevice = device;
    }
  }

  engine.channels[0].loadFile("./media/01_Kick Out.wav");
  engine.channels[1].loadFile("./media/02_Snare Top.wav");
  engine.channels[2].loadFile("./media/03_Snare Down.wav");
  engine.channels[3].loadFile("./media/04_Rack Tom.wav");
  engine.channels[4].loadFile("./media/05_Floor Tom.wav");
  engine.channels[5].loadFile("./media/06_Hi Hat.wav");
  engine.channels[6].loadFile("./media/07_Overhead L.wav");
  engine.channels[7].loadFile("./media/08_Overhead R.wav");

  engine.prepare();
  std::cout << "Audio Engine Started" << std::endl;
  cli.start();

  engine.teardown();
  return 0;
}
