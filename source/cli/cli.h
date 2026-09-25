#include "../engine/engine.h"
#include <iostream>
#include <string>

class Cli {
public:
  Cli(AudioEngine &engine);
  void start();

private:
  bool running = true;
  AudioEngine &engine;

  void parseCommand(std::string command);

};
