#pragma once

#include <atomic>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace metrics {

struct MetricRecord {
  std::string name;
  double      value{};
};

namespace detail {

template <std::size_t CapacityPow2>
class RingBuffer {
  static_assert((CapacityPow2 & (CapacityPow2 - 1)) == 0,
                "capacity must be power-of-two");

  struct Slot {
    std::string name;
    double      value{};
  };

 public:
  bool push(std::string&& name, double value) noexcept {
    auto head = head_.load(std::memory_order_relaxed);
    while (true) {
      auto tail = tail_.load(std::memory_order_acquire);
      if (head - tail >= CapacityPow2)
        return false;
      if (head_.compare_exchange_weak(head, head + 1,
                                      std::memory_order_acq_rel))
        break;
    }
    slots_[head & mask_].name  = std::move(name);
    slots_[head & mask_].value = value;
    return true;
  }

  bool pop(MetricRecord& out) noexcept {
    auto tail = tail_.load(std::memory_order_relaxed);
    auto head = head_.load(std::memory_order_acquire);
    if (tail == head) return false;
    auto& s  = slots_[tail & mask_];
    out.name = std::move(s.name);
    out.value = s.value;
    tail_.store(tail + 1, std::memory_order_release);
    return true;
  }

 private:
  static constexpr std::size_t mask_ = CapacityPow2 - 1;
  std::array<Slot, CapacityPow2> slots_{};
  std::atomic<std::size_t> head_{0};
  std::atomic<std::size_t> tail_{0};
};

} // namespace detail

class Metrics {
 public:
  explicit Metrics(const std::string& filename,
                   std::chrono::milliseconds flush_interval = std::chrono::seconds(1));
  ~Metrics();

  void record(const std::string& name, double value);

  Metrics(const Metrics&)            = delete;
  Metrics& operator=(const Metrics&) = delete;

 private:
  void writerLoop();
  void flushOnce();

  detail::RingBuffer<(1u << 13)> queue_;

  std::unordered_map<std::string, double> store_;

  std::ofstream file_;

  std::thread              writer_;
  std::condition_variable  cv_;
  std::mutex               cv_mtx_;
  std::chrono::milliseconds interval_;
  std::atomic<bool>         running_{true};
};

} // namespace metrics
