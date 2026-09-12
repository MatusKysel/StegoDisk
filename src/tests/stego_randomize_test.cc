// Independent buffers must be safe to randomize and destroy concurrently.
#include <array>
#include <atomic>
#include <iostream>
#include <thread>

#include "utils/memory_buffer.h"

int main() {
  constexpr std::size_t worker_count = 8;
  constexpr std::size_t buffer_size = 4096;
  constexpr unsigned rounds = 128;
  std::array<std::thread, worker_count> workers;
  std::array<unsigned, worker_count> failures{};
  std::atomic<std::size_t> ready{0};
  std::atomic<bool> start{false};

  for (std::size_t worker = 0; worker < worker_count; ++worker) {
    workers[worker] = std::thread([&, worker] {
      stego_disk::MemoryBuffer buffer(buffer_size);
      buffer.Fill(0xA5);
      stego_disk::MemoryBuffer previous(buffer);

      ++ready;
      while (!start.load())
        std::this_thread::yield();

      for (unsigned round = 0; round < rounds; ++round) {
        buffer.Randomize();
        // A complete 4096-byte sample repeating is negligibly probable. This
        // also catches randomization becoming a no-op or a constant fill.
        if (buffer == previous)
          ++failures[worker];
        previous = buffer;
      }
      // Both buffers also randomize their contents during destruction.
    });
  }

  while (ready.load() != worker_count)
    std::this_thread::yield();
  start.store(true);
  for (auto &worker : workers)
    worker.join();

  unsigned total_failures = 0;
  for (const auto count : failures)
    total_failures += count;
  if (total_failures != 0) {
    std::cerr << "Randomize left " << total_failures << " buffers unchanged\n";
    return 1;
  }
  return 0;
}
