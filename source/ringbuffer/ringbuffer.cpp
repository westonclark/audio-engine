#include "ringbuffer.h"
#include <algorithm>
#include <cstring>

RingBuffer::RingBuffer(size_t capacity)
    : data(capacity), capacity(capacity), mask(capacity - 1) {}

size_t RingBuffer::getFreeSpace() const {
  size_t used = writeIndex.load(std::memory_order_acquire) -
                readIndex.load(std::memory_order_acquire);
  return capacity - used;
}

size_t RingBuffer::write(const float *source, size_t count) {
  size_t write = writeIndex.load(std::memory_order_relaxed);
  size_t read = readIndex.load(std::memory_order_acquire);

  count = std::min(count, capacity - (write - read));

  size_t start = write & mask;
  size_t firstPart = std::min(count, capacity - start);
  memcpy(data.data() + start, source, firstPart * sizeof(float));
  memcpy(data.data(), source + firstPart, (count - firstPart) * sizeof(float));

  writeIndex.store(write + count, std::memory_order_release);
  return count;
}

size_t RingBuffer::read(float *destination, size_t count) {
  size_t read = readIndex.load(std::memory_order_relaxed);
  size_t write = writeIndex.load(std::memory_order_acquire);

  count = std::min(count, write - read);

  size_t start = read & mask;
  size_t firstPart = std::min(count, capacity - start);
  memcpy(destination, data.data() + start, firstPart * sizeof(float));
  memcpy(destination + firstPart, data.data(),
         (count - firstPart) * sizeof(float));

  readIndex.store(read + count, std::memory_order_release);
  return count;
}

void RingBuffer::reset() {
  writeIndex.store(0);
  readIndex.store(0);
}
