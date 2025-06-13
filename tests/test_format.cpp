#include "metrics.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <thread>

using namespace std::chrono_literals;

int main() {
  std::filesystem::remove("test.log");
  metrics::Metrics m("test.log", 200ms);

  m.record("CPU", 1.23);
  m.record("HTTP requests RPS", 77);

  std::this_thread::sleep_for(250ms);

  std::ifstream f("test.log");
  std::string line;
  std::getline(f, line);

  std::regex re(
      R"(^(?:\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})(?: "[^"]+" (?:\d+\.\d+|\d+))+ ?$)"
  );

  if (std::regex_match(line, re)) {
    std::cout << "FORMAT  OK!\n";
    return 0;
  }
  std::cerr << "FORMAT  FAIL!!  --  " << line << '\n';
  return 1;
}
