#include "./file/file.h"
#include <CoreAudio/CoreAudio.h>

int main() { AudioFile audioFile = readAudioFile("./audio.wav");
return 0;}
