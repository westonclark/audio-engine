#include "IOProc.h"
#include "../../engine/engine.h"

OSStatus coreAudioIOProc(AudioObjectID device, const AudioTimeStamp *now,
                         const AudioBufferList *input,
                         const AudioTimeStamp *inputTime,
                         AudioBufferList *output,
                         const AudioTimeStamp *outputTime, void *clientData) {
  AudioEngine *engine = static_cast<AudioEngine *>(clientData);
  uint32_t frameCount = 0;

  if ((*output).mNumberBuffers > 0) {
    const AudioBuffer &buffer = (*output).mBuffers[0];
    frameCount = buffer.mDataByteSize / sizeof(float) / buffer.mNumberChannels;
  }

  (*engine).process(input, output, frameCount);
  return 0;
}
