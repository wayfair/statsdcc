
/**
 * Source file for class graphite
 * Please see graphite.h and backend.h for documentaion
 */

#include "statsdcc/backends/stdout.h"

#include <iostream>
#include <unordered_map>

#include "statsdcc/chrono.h"
#include "statsdcc/configs/aggregator_config.h"

namespace statsdcc { namespace backends {

void Stdout::flush_stats(const Ledger& ledger, int flusher_id) {
  std::uint64_t time_stamp = chrono::unixtime_ms() / 1000;
  std::string ts_suffix = " " +
    std::to_string(static_cast<long long unsigned int>(time_stamp)) +
    "\n";
  std::uint64_t start_time = chrono::unixtime_ms();
  std::uint64_t num_stats = 0;
  std::string timer_data_key;
  std::string out = "";

  // prefix for aggregator stats
  auto prefix = ::config->name + ".thread_" +
                  std::to_string(static_cast<long long int>(flusher_id));

  // counters
  for (auto counter_itr = ledger.counters.cbegin();
      counter_itr != ledger.counters.cend();
      ++counter_itr) {
    std::string key = counter_itr->first;
    std::string value =
      std::to_string(static_cast<long double>(counter_itr->second));

    std::string value_per_second =
      std::to_string(static_cast<long double>(ledger.counter_rates.at(key)));

    out += key + ".rate " + value_per_second + ts_suffix +
           key + ".count " + value + ts_suffix;

    ++num_stats;
  }

  // timers
  for (auto timer_itr = ledger.timer_data.cbegin();
      timer_itr != ledger.timer_data.cend();
      ++timer_itr) {
    std::string key = timer_itr->first;

    for (auto timer_data_itr = timer_itr->second.cbegin();
        timer_data_itr != timer_itr->second.cend();
        ++timer_data_itr) {
      std::string timer_data_key = timer_data_itr->first;

      std::string value = std::to_string(
        static_cast<long double>(timer_data_itr->second));

      out += key + "." + timer_data_key + " " + value + ts_suffix;
    }
    ++num_stats;
  }

  // gauges
  for (auto gauge_itr = ledger.gauges.cbegin();
      gauge_itr != ledger.gauges.cend();
      ++gauge_itr) {
    std::string key = gauge_itr->first;

    std::string value = std::to_string(
      static_cast<long double>(gauge_itr->second));

    out += key + " " + value + ts_suffix;

    ++num_stats;
  }

  // sets
  for (auto set_itr = ledger.sets.cbegin();
      set_itr != ledger.sets.cend();
      ++set_itr) {
    std::string key = set_itr->first;
    auto value = set_itr->second;

    out += key + ".count " +
           std::to_string(static_cast<long long int>(value.size())) +
           ts_suffix;

     ++num_stats;
  }

  // stats
  std::string num_stats_str =
    std::to_string(static_cast<long long int>(num_stats));

  std::string total_time =
    std::to_string(
      static_cast<long long unsigned int>(chrono::unixtime_ms() - start_time));

  std::string key = prefix + ".numStats";
  out += key + " " + num_stats_str + ts_suffix;

  key = prefix + ".graphiteStats.calculationtime";
  out += key + " " + total_time + ts_suffix;

  for (auto statsd_metric_itr = ledger.statsd_metrics.cbegin();
      statsd_metric_itr != ledger.statsd_metrics.cend();
      ++statsd_metric_itr) {
    std::string key = prefix + '.' + statsd_metric_itr->first;

    std::string value = std::to_string(
      static_cast<long long int>(statsd_metric_itr->second));

    out += key + " " + value + ts_suffix;
  }

  std::cout<< out << std::endl;
}
}  // namespace backends
}  // namespace statsdcc
