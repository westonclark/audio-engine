#include "file.h"
#include <cstdint>

AudioFile readAudioFile(std::string path) {

  FILE *file = fopen(path.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Error opening file: " + path);
  }

  AudioFile audioFile{};
  bool isFloatData = false;

  // ==========================================================================
  // Riff Header
  // ==========================================================================
  std::string riffTitle = readString(file, 4);  // 4 bytes - Riff Title
  uint32_t fileSize = readU32(file) + 8;        // 4 bytes - File Size
  std::string fileFormat = readString(file, 4); // 4 bytes - File Format

  if (riffTitle != "RIFF" || fileFormat != "WAVE") {
    fclose(file);
    throw std::runtime_error("Only .wav files are supported");
  };

  // ==========================================================================
  // Chunks
  // ==========================================================================
  char chunkName[5] = {};
  uint32_t chunkSize;
  while (fread(&chunkName, 4, 1, file) && fread(&chunkSize, 4, 1, file)) {
    long chunkEnd = ftell(file) + chunkSize + (chunkSize & 1);
    // ========================================================================
    // Format Chunk
    // ========================================================================
    if (std::strcmp(chunkName, "fmt ") == 0) {
      uint16_t pcmFlags = readU16(file);   // 2 bytes - PcmFlags
      uint16_t channels = readU16(file);   // 2 bytes - Channel Count
      uint32_t sampleRate = readU32(file); // 4 bytes - SampleRate
      uint32_t byteRate = readU32(file);   // 4 bytes - ByteRate
      uint16_t blockAlign = readU16(file); // 2 bytes - BlockAlign
      uint16_t bitDepth = readU16(file);   // 2 bytes - BitDepth

      audioFile.channels = channels;
      audioFile.bitsPerSample = bitDepth;
      audioFile.sampleRate = sampleRate;

      if (pcmFlags == 3) {
        isFloatData = true;
      }

      // Parse extension if present
      if (chunkSize != 16) {
        uint16_t extensionSize = readU16(file); // 2 bytes - Extension Size
        if (pcmFlags == 65534) {                // WAVE_FORMAT_EXTENSIBLE
          uint16_t validBits = readU16(file); // 2 bytes - Valid Bits Per Sample
          uint32_t channelMask = readU32(file); // 4 bytes - Channel Mask
          uint32_t subFormat = readU32(file);   // 4 bytes - Sub Format

          audioFile.bitsPerSample = validBits;
          if (subFormat == 3) {
            isFloatData = true;
          }
        }
      }
    }

    // ========================================================================
    // Data Chunk
    // ========================================================================
    if (std::strcmp(chunkName, "data") == 0) {
      uint16_t bitsPerSample = audioFile.bitsPerSample;

      // Vector of floats to store final data
      int bytesPerSample = bitsPerSample / 8;
      int numberOfSamples = chunkSize / bytesPerSample;
      audioFile.samples = std::vector<float>(numberOfSamples);

      if (isFloatData) {
        fread(audioFile.samples.data(), sizeof(float), numberOfSamples, file);
      } else {

        // Read raw bytes into memory
        std::vector<uint8_t> rawBytes(chunkSize);
        fread(rawBytes.data(), 1, chunkSize, file);

        // Create normalization ratio
        float maxValue = 1u << (bitsPerSample - 1); // - 1 since value is signed
        float normalizationScale = 1.0f / (1.0 + maxValue);

        for (int i = 0; i < numberOfSamples; i++) {
          // Copy bytes into 4 byte int
          uint32_t rawValue = 0;
          memcpy(&rawValue, rawBytes.data() + (i * bytesPerSample),
                 bytesPerSample);

          // Shift signed bit all the way left and back right to fill
          int unusedBits = 32 - bitsPerSample;
          int32_t sample = (int32_t)(rawValue << unusedBits) >> unusedBits;

          // Normalize to -1 - 1 range
          float normalizedValue = sample * normalizationScale;
          audioFile.samples[i] = normalizedValue;
        }
      }

      fclose(file);
      return audioFile;
    }
    fseek(file, chunkEnd, SEEK_SET);
  }

  fclose(file);
  throw std::runtime_error("Could not locate audio data from file: " + path);
}

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
