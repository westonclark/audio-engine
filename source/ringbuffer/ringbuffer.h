#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

class RingBuffer {
public:
  RingBuffer(size_t capacity);

  size_t available() const;
  size_t freeSpace() const;

  size_t write(const float *source, size_t count);
  size_t read(float *destination, size_t count);

  void reset();

private:
  std::vector<float> data;
  size_t capacity;
  size_t mask;

  std::atomic<size_t> writeIndex = 0;
  std::atomic<size_t> readIndex = 0;
};
