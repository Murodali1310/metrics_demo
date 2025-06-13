#include "metrics.hpp"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace metrics {

static std::string timestamp()
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::time_t tt = system_clock::to_time_t(now);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
      << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

Metrics::Metrics(const std::string& filename,
                 std::chrono::milliseconds flush_interval)
    : file_(filename, std::ios::out | std::ios::trunc),
      interval_(flush_interval)
{
  if (!file_)
    throw std::runtime_error("Can't open metrics file: " + filename);
  writer_ = std::thread(&Metrics::writerLoop, this);
}

Metrics::~Metrics()
{
  { std::lock_guard lk(cv_mtx_); running_ = false; }
  cv_.notify_one();
  if (writer_.joinable()) writer_.join();

  flushOnce();
}

void Metrics::record(const std::string& name, double value)
{
  queue_.push(std::string(name), value);   // если очередь переполнена → drop
}

void Metrics::writerLoop()
{
  using clock = std::chrono::steady_clock;
  auto next = clock::now() + interval_;

  std::unique_lock lk(cv_mtx_);
  while (running_) {
    if (cv_.wait_until(lk, next, [this]{ return !running_; }))
      break;
    lk.unlock();
    flushOnce();
    next += interval_;
    lk.lock();
  }
}

void Metrics::flushOnce()
{
  MetricRecord rec;
  while (queue_.pop(rec))
    store_[std::move(rec.name)] = rec.value;

  if (store_.empty()) return;

  std::vector<std::pair<std::string,double>> v(store_.begin(), store_.end());
  std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.first < b.first; });

  std::ostringstream line;
  line << timestamp();
  for (auto& [k, val] : v)
    line << " \"" << k << "\" " << val;
  line << '\n';

  file_ << line.str();
  file_.flush();

  store_.clear();
}

} // namespace metrics
