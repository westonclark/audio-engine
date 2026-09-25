#include "./cli.h"

Cli::Cli(AudioEngine &engine) : engine(engine) {};

void Cli::start() {
  while (auto command = readInput()) {
    if (!handleCommand(*command))
      break;
  }
}

std::optional<std::string> Cli::readInput() {
  std::cout << "Enter Command: ";

  std::string line;
  if (!std::getline(std::cin, line)) {
    return std::nullopt;
  }
  return line;
}

bool Cli::handleCommand(std::string &command) {
  if (command == "quit") {
    return false;
  } else if (command == "start") {
    engine.play();
  } else if (command == "stop") {
    engine.stop();
  } else {
    std::cout << "Invalid Command " << std::endl;
  }
  return true;
};
