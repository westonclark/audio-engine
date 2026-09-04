#include "./channel.h"

void Channel::setGain(double newGain) {
  Channel &channel = *this;
  channel.gainRatio = std::pow(10.0, newGain / 20.0);
};
