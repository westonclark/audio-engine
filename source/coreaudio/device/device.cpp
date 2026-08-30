#include "device.h"
#include <cstdint>

std::vector<AudioDevice> getAvailableDevices() {
  AudioObjectPropertyAddress audioHardwareObjectAddress = {
      .mSelector = kAudioHardwarePropertyDevices,
      .mScope = kAudioObjectPropertyScopeGlobal,
      .mElement = kAudioObjectPropertyElementMain,
  };

  uint32_t deviceListSize;
  OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                   &audioHardwareObjectAddress,
                                                   0, nullptr, &deviceListSize);
  if (status != 0) {
    throw std::runtime_error("AudioObjectGetPropertyDataSize failed: " +
                             std::to_string(status));
  }

  std::vector<AudioDeviceID> deviceIds(deviceListSize / sizeof(AudioDeviceID));
  status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                      &audioHardwareObjectAddress, 0, nullptr,
                                      &deviceListSize, deviceIds.data());
  if (status != 0) {
    throw std::runtime_error("AudioObjectGetPropertyData failed: " +
                             std::to_string(status));
  }

  std::vector<AudioDevice> devices;
  for (AudioDeviceID id : deviceIds) {
    devices.push_back({
        .id = id,
        .name = getDeviceName(id),
        .inputChannelCount =
            getDeviceChannelCount(id, kAudioObjectPropertyScopeInput),
        .outputChannelCount =
            getDeviceChannelCount(id, kAudioObjectPropertyScopeOutput),
        .supportedSampleRates = getDeviceSampleRates(id),
    });
  }

  return devices;
}

std::string getDeviceName(AudioDeviceID deviceId) {
  AudioObjectPropertyAddress namePropertyAddress = {
      .mSelector = kAudioObjectPropertyName,
      .mScope = kAudioObjectPropertyScopeGlobal,
      .mElement = kAudioObjectPropertyElementWildcard};

  CFStringRef cfName = nullptr;
  uint32_t size = sizeof(CFStringRef);
  OSStatus status = AudioObjectGetPropertyData(deviceId, &namePropertyAddress,
                                               0, nullptr, &size, &cfName);
  if (status != 0) {
    throw std::runtime_error("getDeviceName failed for device " +
                             std::to_string(deviceId) + ": " +
                             std::to_string(status));
  }

  char buffer[256];
  CFStringGetCString(cfName, buffer, sizeof(buffer), kCFStringEncodingUTF8);
  CFRelease(cfName);

  return std::string(buffer);
}

uint32_t getDeviceChannelCount(AudioDeviceID deviceId,
                               AudioObjectPropertyScope scope) {
  AudioObjectPropertyAddress streamConfigPropertyAddress = {
      .mSelector = kAudioDevicePropertyStreamConfiguration,
      .mScope = scope,
      .mElement = kAudioObjectPropertyElementWildcard};

  uint32_t streamConfigSize;
  OSStatus status = AudioObjectGetPropertyDataSize(
      deviceId, &streamConfigPropertyAddress, 0, nullptr, &streamConfigSize);
  if (status != 0) {
    throw std::runtime_error(
        "getDeviceChannelCount DataSize failed for device " +
        std::to_string(deviceId) + ": " + std::to_string(status));
  }

  uint8_t tempBuffer[streamConfigSize];
  AudioBufferList *audioBufferList = (AudioBufferList *)tempBuffer;

  status =
      AudioObjectGetPropertyData(deviceId, &streamConfigPropertyAddress, 0,
                                 nullptr, &streamConfigSize, audioBufferList);
  if (status != 0) {
    throw std::runtime_error(
        "getDeviceChannelCount failed for device " + std::to_string(deviceId) +
        " scope " + std::to_string(scope) + ": " + std::to_string(status));
  }

  uint32_t channelCount = 0;
  for (uint32_t i = 0; i < audioBufferList->mNumberBuffers; i++) {
    channelCount += audioBufferList->mBuffers[i].mNumberChannels;
  }

  return channelCount;
}

std::vector<float> getDeviceSampleRates(AudioDeviceID deviceId) {
  AudioObjectPropertyAddress sampleRatesPropertyAddress = {
      .mSelector = kAudioDevicePropertyAvailableNominalSampleRates,
      .mScope = kAudioObjectPropertyScopeGlobal,
      .mElement = kAudioObjectPropertyElementMain};

  uint32_t dataSize;
  OSStatus status = AudioObjectGetPropertyDataSize(
      deviceId, &sampleRatesPropertyAddress, 0, nullptr, &dataSize);
  if (status != 0) {
    throw std::runtime_error(
        "getDeviceSampleRates DataSize failed for device " +
        std::to_string(deviceId) + ": " + std::to_string(status));
  }

  if (dataSize == 0) {
    return {};
  }

  std::vector<AudioValueRange> ranges(dataSize / sizeof(AudioValueRange));
  status = AudioObjectGetPropertyData(deviceId, &sampleRatesPropertyAddress, 0,
                                      nullptr, &dataSize, ranges.data());
  if (status != 0) {
    throw std::runtime_error("getDeviceSampleRates failed for device " +
                             std::to_string(deviceId) + ": " +
                             std::to_string(status));
  }

  std::vector<float> sampleRates;
  for (const auto &range : ranges) {
    sampleRates.push_back(range.mMinimum);
  }

  return sampleRates;
}

void setDeviceSampleRate(AudioDeviceID deviceId, float sampleRate) {
  AudioObjectPropertyAddress sampleRatePropertyAddress = {
      .mSelector = kAudioDevicePropertyNominalSampleRate,
      .mScope = kAudioObjectPropertyScopeGlobal,
      .mElement = kAudioObjectPropertyElementMain};

  Float64 rate = sampleRate;
  OSStatus status = AudioObjectSetPropertyData(
      deviceId, &sampleRatePropertyAddress, 0, nullptr, sizeof(rate), &rate);
  if (status != 0) {
    throw std::runtime_error("setDeviceSampleRate failed for device " +
                             std::to_string(deviceId) + ": " +
                             std::to_string(status));
  }
}

AudioDeviceIOProcID setDeviceCallback(AudioDeviceID deviceId,
                                      AudioDeviceIOProc procId,
                                      void *clientData) {
  AudioDeviceIOProcID procIdOut;
  OSStatus status =
      AudioDeviceCreateIOProcID(deviceId, procId, clientData, &procIdOut);
  if (status != 0) {
    throw std::runtime_error("AudioDeviceCreateIOProcID failed: " +
                             std::to_string(status));
  }
  return procIdOut;
}

void removeDeviceCallback(AudioDeviceID deviceId, AudioDeviceIOProcID procId) {
  OSStatus status = AudioDeviceDestroyIOProcID(deviceId, procId);
  if (status != 0) {
    throw std::runtime_error("AudioDeviceDestroyIOProcID failed: " +
                             std::to_string(status));
  }
}

void startDevice(AudioDeviceID deviceId, AudioDeviceIOProcID procId) {
  OSStatus status = AudioDeviceStart(deviceId, procId);
  if (status != 0) {
    throw std::runtime_error("AudioDeviceStart failed: " +
                             std::to_string(status));
  }
}

void stopDevice(AudioDeviceID deviceId, AudioDeviceIOProcID procId) {
  OSStatus status = AudioDeviceStop(deviceId, procId);
  if (status != 0) {
    throw std::runtime_error("AudioDeviceStop failed: " +
                             std::to_string(status));
  }
}
