
/**
 * Definition of class Ledger
 */

#ifndef INCLUDE_STATSDCC_LEDGER_H_
#define INCLUDE_STATSDCC_LEDGER_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "statsdcc/configs/aggregator_config.h"

// <name>:<val>|<type> 1496 1  1  1  1  = 1500
#define MAX_TOKEN_BUFFER_SIZE 1497
#define MAX_TOKEN_STRING_LENGTH "1496"

namespace statsdcc {

namespace backends {
class Carbon;  // forward declare to avoid cyclic include
class Stdout;
class Repeater;
}

/**
 * Parses the metric submitted in one of the following format,
 * into std collections for easy processing
 *   METRIC:METRIC_VALUE|c (counter)
 *   METRIC:METRIC_VALUE|c@SAMPLING_RATE (counter with sampling)
 *   METRIC:METRIC_VALUE|ms (timer)
 *   METRIC:METRIC_VALUE|ms@SAMPLING_RATE (timer with sampling)
 *   METRIC:METRIC_VALUE|g (gauge)
 *   METRIC:METRIC_VALUE|s (set)
 *
 * Also performs aggregation similar to https://github.com/etsy/statsd
 */
class Ledger {
  friend class backends::Carbon;
  friend class backends::Stdout;
  friend class backends::Repeater;
  friend class LedgerTest;  // unit tests

 private:
  enum class MetricType { counter, timer, gauge, set };

 public:
  inline Ledger() {}

  inline Ledger(const Ledger& ledger) :
    counters(ledger.counters),
    timers(ledger.timers),
    timer_counters(ledger.timer_counters),
    gauges(ledger.gauges),
    sets(ledger.sets),
    counter_rates(ledger.counter_rates),
    timer_data(ledger.timer_data),
    statsd_metrics(ledger.statsd_metrics) {
  }

  inline Ledger(const Ledger&& ledger) :
    counters(std::move(ledger.counters)),
    timers(std::move(ledger.timers)),
    timer_counters(std::move(ledger.timer_counters)),
    gauges(std::move(ledger.gauges)),
    sets(std::move(ledger.sets)),
    counter_rates(std::move(ledger.counter_rates)),
    timer_data(std::move(ledger.timer_data)),
    statsd_metrics(std::move(ledger.statsd_metrics)) {
  }

  inline Ledger& operator=(const Ledger& ledger) {
    this->counters = ledger.counters;
    this->timers = ledger.timers;
    this->timer_counters = ledger.timer_counters;
    this->gauges = ledger.gauges;
    this->sets = ledger.sets;
    this->counter_rates = ledger.counter_rates;
    this->timer_data = ledger.timer_data;
    this->statsd_metrics = ledger.statsd_metrics;
    return *this;
  }

  inline Ledger& operator=(const Ledger&& ledger) {
    this->counters = std::move(ledger.counters);
    this->timers = std::move(ledger.timers);
    this->timer_counters = std::move(ledger.timer_counters);
    this->gauges = std::move(ledger.gauges);
    this->sets = std::move(ledger.sets);
    this->counter_rates = std::move(ledger.counter_rates);
    this->timer_data = std::move(ledger.timer_data);
    this->statsd_metrics = std::move(ledger.statsd_metrics);
    return *this;
  }

  ~Ledger() = default;

  /**
   * Buffers the metric.
   *
   * @param metric The metric value to be buffered for processing later
   */
  void buffer(const std::string& metric);

  /**
   * Aggregates the metric values buffered by buffer method.
   */
  void process();

  inline int bad_lines_seen() {
    auto bad_lines_key = ::config->name + ".bad_lines_seen";
    return (this->counters.find(bad_lines_key) != this->counters.end())
      ? static_cast<int>(this->counters[bad_lines_key])
      : 0;
  }

  std::unordered_map<std::string, long long int> frequency;

 private:
  std::unordered_map<std::string, double> counters;
  std::unordered_map<std::string, std::vector<double> > timers;
  std::unordered_map<std::string, double> timer_counters;
  std::unordered_map<std::string, double> gauges;
  std::unordered_map<std::string, std::unordered_set<std::string> > sets;

  std::unordered_map<std::string, double> counter_rates;

  std::unordered_map<
    std::string, std::unordered_map<std::string, double>
  > timer_data;

  std::unordered_map<std::string, std::int64_t> statsd_metrics;
};

}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_LEDGER_H_
