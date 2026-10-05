#include "file.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

AudioStream::~AudioStream() { close(); }

void AudioStream::open(const std::string &path) {
  close();

  file = fopen(path.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Error opening file: " + path);
  }

  std::string riff = readString(file, 4);   // 4 bytes - Riff Title
  uint32_t fileSize = readU32(file) + 8;    // 4 bytes - File Size
  std::string format = readString(file, 4); // 4 bytes - File Format

  if (riff != "RIFF" || format != "WAVE") {
    close();
    throw std::runtime_error("Only .wav files are supported");
  };

  char chunkName[5] = {};
  uint32_t chunkSize;
  while (fread(&chunkName, 4, 1, file) &&
         fread(&chunkSize, 4, 1, file)) {
    off_t chunkEnd = ftello(file) + chunkSize + (chunkSize & 1);

    // Format Chunk
    if (std::strcmp(chunkName, "fmt ") == 0) {
      uint16_t pcmFlags = readU16(file);   // 2 bytes - PcmFlags
      channels = readU16(file);            // 2 bytes - Channel Count
      sampleRate = readU32(file);          // 4 bytes - SampleRate
      uint32_t byteRate = readU32(file);   // 4 bytes - ByteRate
      blockAlign = readU16(file);          // 2 bytes - BlockAlign
      bitsPerSample = readU16(file);       // 2 bytes - BitDepth

      isFloatData = pcmFlags == 3;

      if (chunkSize != 16) {
        uint16_t extensionSize =
            readU16(file); // 2 bytes - Extension Size

        // Wave Format Extensible
        if (pcmFlags == 65534) {
          uint16_t validBits = readU16(file);   // 2 bytes - Valid Bits
          uint32_t channelMask = readU32(file); // 4 bytes - Channel Mask
          uint32_t subFormat = readU32(file);   // 4 bytes - Sub Format

          if (subFormat == 3) {
            isFloatData = true;
          }
        }
      }
    }

    // Data Chunk
    if (std::strcmp(chunkName, "data") == 0) {
      if (channels != 1) {
        close();
        throw std::runtime_error("Only mono files are supported: " + path);
      }

      dataOffset = ftello(file);
      totalFrames = chunkSize / blockAlign;
      currentFrame = 0;

      // Normalization scale
      float maxValue = 1u << (bitsPerSample - 1);
      normalizationScale = 1.0f / (1.0 + maxValue);

      frameData.reserve(STREAM_CHUNK_FRAMES);
      return;
    }
    fseeko(file, chunkEnd, SEEK_SET);
  }

  close();
  throw std::runtime_error("Could not locate audio data from file: " + path);
}

void AudioStream::close() {
  if (file) {
    fclose(file);
    file = nullptr;
  }
}

void AudioStream::seek(uint32_t frame) {
  currentFrame = std::min(frame, totalFrames);
  fseeko(file,
         dataOffset + (off_t)currentFrame * blockAlign,
         SEEK_SET);
}

const std::vector<float> &AudioStream::readFrames(size_t frames) {

  size_t remainingFrames = totalFrames - currentFrame;
  frames = std::min({frames, remainingFrames, STREAM_CHUNK_FRAMES});
  frameData.resize(frames);

  size_t framesRead = 0;

  if (isFloatData) {
    framesRead =
        fread(frameData.data(), sizeof(float), frames, file);
  } else {
    uint8_t rawBytes[4096];
    size_t bytesPerSample = blockAlign;
    size_t samplesPerRead = sizeof(rawBytes) / bytesPerSample;
    int unusedBits = 32 - bitsPerSample;

    while (framesRead < frames) {
      size_t wanted = std::min(samplesPerRead, frames - framesRead);
      size_t got = fread(rawBytes, bytesPerSample, wanted, file);

      // Construct each sample
      for (size_t i = 0; i < got; i++) {
        uint32_t rawValue = 0;
        memcpy(&rawValue, rawBytes + (i * bytesPerSample), bytesPerSample);

        // Shift signed bit
        int32_t sample = (int32_t)(rawValue << unusedBits) >> unusedBits;

        // Normalize
        frameData[framesRead + i] = sample * normalizationScale;
      }

      framesRead += got;
      if (got < wanted) {
        break;
      }
    }
  }

  if (framesRead < frames) {
    totalFrames = currentFrame + framesRead;
  }

  currentFrame += framesRead;
  frameData.resize(framesRead);
  return frameData;
}

bool AudioStream::isOpen() const { return file != nullptr; }

bool AudioStream::isFinished() const { return currentFrame >= totalFrames; }

std::string readString(FILE *file, uint32_t length) {
  std::string name(length, '\0');
  fread(&name[0], length, 1, file);
  return name;
}

uint32_t readU32(FILE *file) {
  uint32_t value;
  fread(&value, 4, 1, file);
  return value;
}

uint16_t readU16(FILE *file) {
  uint16_t value;
  fread(&value, 2, 1, file);
  return value;
}
