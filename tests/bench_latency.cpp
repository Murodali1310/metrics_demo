#include "metrics.hpp"
#include <iostream>

int main() {
  metrics::Metrics m("bench.log", std::chrono::hours(1)); // flush редко

  constexpr int N = 1'000'000;
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; ++i)
    m.record("X", 1.0);
  auto end = std::chrono::high_resolution_clock::now();

  double avg_ns = std::chrono::duration<double, std::nano>(end - start).count() / N;
  std::cout << "RECORD avg latency ≈ " << avg_ns << " ns\\n";
  return avg_ns < 1000 ? 0 : 1;
}
