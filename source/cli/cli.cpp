#include "./cli.h"

Cli::Cli(AudioEngine &engine) : engine(engine) {};

void Cli::start() {
  std::string command;

  while (running) {
    std::cout << "Enter Command: ";
    if (!std::getline(std::cin, command)) {
      break;
    }
    parseCommand(command);
  }
}

void Cli::parseCommand(std::string command) {
  if (command == "start") {
    engine.play();
  }

  else if (command == "stop") {
    engine.stop();
  }

  else if (command == "quit") {
    running = false;
  }

  else {
    std::cout << "Invalid Command " << std::endl;
  }
};
