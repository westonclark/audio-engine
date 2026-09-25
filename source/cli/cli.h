#include "../engine/engine.h"
#include <optional>
#include <string>

class Cli {
public:
  Cli(AudioEngine &engine);
  void start();

private:
  bool running = true;
  AudioEngine &engine;

  bool handleCommand(std::string &command);
  std::optional<std::string> readInput();
};
