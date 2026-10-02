#include "ringbuffer.h"
#include <algorithm>
#include <cstring>

RingBuffer::RingBuffer(size_t capacity)
    : data(capacity), capacity(capacity), mask(capacity - 1) {}

size_t RingBuffer::available() const {
  return writeIndex.load(std::memory_order_acquire) -
         readIndex.load(std::memory_order_acquire);
}

size_t RingBuffer::freeSpace() const { return capacity - available(); }

size_t RingBuffer::write(const float *source, size_t count) {
  RingBuffer &ring = *this;
  size_t write = ring.writeIndex.load(std::memory_order_relaxed);
  size_t read = ring.readIndex.load(std::memory_order_acquire);

  count = std::min(count, ring.capacity - (write - read));

  size_t start = write & ring.mask;
  size_t firstPart = std::min(count, ring.capacity - start);
  memcpy(ring.data.data() + start, source, firstPart * sizeof(float));
  memcpy(ring.data.data(), source + firstPart, (count - firstPart) * sizeof(float));

  ring.writeIndex.store(write + count, std::memory_order_release);
  return count;
}

size_t RingBuffer::read(float *destination, size_t count) {
  RingBuffer &ring = *this;
  size_t read = ring.readIndex.load(std::memory_order_relaxed);
  size_t write = ring.writeIndex.load(std::memory_order_acquire);

  count = std::min(count, write - read);

  size_t start = read & ring.mask;
  size_t firstPart = std::min(count, ring.capacity - start);
  memcpy(destination, ring.data.data() + start, firstPart * sizeof(float));
  memcpy(destination + firstPart, ring.data.data(), (count - firstPart) * sizeof(float));

  ring.readIndex.store(read + count, std::memory_order_release);
  return count;
}

void RingBuffer::reset() {
  writeIndex.store(0);
  readIndex.store(0);
}
