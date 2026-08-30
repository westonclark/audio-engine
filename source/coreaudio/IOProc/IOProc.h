#pragma once

#include <CoreAudio/CoreAudio.h>

OSStatus coreAudioIOProc(AudioObjectID device, const AudioTimeStamp *now,
                         const AudioBufferList *input,
                         const AudioTimeStamp *inputTime,
                         AudioBufferList *output,
                         const AudioTimeStamp *outputTime, void *clientData);
