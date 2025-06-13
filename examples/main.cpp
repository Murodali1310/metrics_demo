#include "metrics.hpp"
#include <random>
#include <thread>
#include <atomic>
#include <iostream>

int main()
{
  metrics::Metrics collector("metrics.log");

  std::atomic<bool> running{true};

  std::thread cpu([&] {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(0.0, 2.0);
    while (running) {
      collector.record("CPU", dist(gen));
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  std::thread rps([&] {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 100);
    while (running) {
      collector.record("HTTP requests RPS", dist(gen));
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  std::this_thread::sleep_for(std::chrono::seconds(5));
  running = false;
  cpu.join();
  rps.join();

  std::cout << "Готово! Чекни файл metrics.log\n";
}
