#pragma once

#include <CoreAudio/CoreAudio.h>
#include <cstdint>
#include <string>
#include <vector>

struct AudioDevice {
  AudioDeviceID id;
  std::string name;
  uint32_t inputChannelCount;
  uint32_t outputChannelCount;
  std::vector<float> supportedSampleRates;
};

std::vector<AudioDevice> getAvailableDevices();

std::string getDeviceName(AudioDeviceID deviceId);
uint32_t getDeviceChannelCount(AudioDeviceID deviceId, AudioObjectPropertyScope scope);

std::vector<float> getDeviceSampleRates(AudioDeviceID deviceId);
void setDeviceSampleRate(AudioDeviceID deviceId, float sampleRate);

AudioDeviceIOProcID setDeviceCallback(AudioDeviceID deviceId, AudioDeviceIOProc procId, void *clientData = nullptr);
void removeDeviceCallback(AudioDeviceID deviceId, AudioDeviceIOProcID procId);

void startDevice(AudioDeviceID deviceId, AudioDeviceIOProcID procId);
void stopDevice(AudioDeviceID deviceId, AudioDeviceIOProcID procId);
