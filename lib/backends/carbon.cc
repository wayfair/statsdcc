
/**
 * Source file for class carbon
 * Please see carbon.h and backend.h for documentaion
 */

#include "statsdcc/backends/carbon.h"

#include <unordered_map>

#include "statsdcc/chrono.h"
#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/hashring/hashring.h"
#include "statsdcc/net/connection.h"
#include "statsdcc/net/lib.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/os.h"

namespace statsdcc { namespace backends {

Carbon::Carbon() {
  try {
    this->hashring = std::unique_ptr<hashring::Hashring>(
                      new hashring::Hashring(::config->backends.carbon));
  } catch(std::string msg) {
    ::logger->error("Unable to create hashring: " + msg);
    throw;
  }
}

void Carbon::flush_stats(const Ledger& ledger, int flusher_id) {
  std::uint64_t time_stamp = chrono::unixtime_ms() / 1000;
  std::string ts_suffix = " " +
    std::to_string(static_cast<long long unsigned int>(time_stamp)) +
    "\n";
  std::uint64_t start_time = chrono::unixtime_ms();
  std::unordered_map<Hostport, std::string> stat_strings;
  std::uint64_t num_stats = 0;
  std::string timer_data_key;

  // prefix for aggregator stats
  this->prefix_stats =
    ::config->name + ".thread_" +
      std::to_string(static_cast<long long int>(flusher_id));

  // prefix for all stats
  this->prefix =
    ::config->prefix;

  // counters
  for (auto counter_itr = ledger.counters.cbegin();
      counter_itr != ledger.counters.cend();
      ++counter_itr) {
    std::string key = counter_itr->first;
    std::string value =
      std::to_string(static_cast<long double>(counter_itr->second));

    std::string value_per_second =
      std::to_string(static_cast<long double>(ledger.counter_rates.at(key)));

    // get the destination carbon hostport
    Hostport n = this->hashring->get(key);

    std::string metric_name = this->prefix + ::config->prefixCounter + this->process_name(key);

    stat_strings[n] +=
      metric_name + ".rate"
                  + " "
                  + value_per_second
                  + ts_suffix;

    stat_strings[n] +=
      metric_name + ".count"
                  + " "
                  + value
                  + ts_suffix;

    ++num_stats;
  }

  // timers
  for (auto timer_itr = ledger.timer_data.cbegin();
      timer_itr != ledger.timer_data.cend();
      ++timer_itr) {
    std::string key = timer_itr->first;
    std::string metric_name = this->prefix + ::config->prefixTimer + this->process_name(key);

    for (auto timer_data_itr = timer_itr->second.cbegin();
        timer_data_itr != timer_itr->second.cend();
        ++timer_data_itr) {
      std::string timer_data_key = timer_data_itr->first;

      std::string value = std::to_string(
        static_cast<long double>(timer_data_itr->second));

      stat_strings[this->hashring->get(key)] +=
        metric_name + '.'
                    + timer_data_key
                    + " "
                    + value
                    + ts_suffix;
    }
    ++num_stats;
  }

  // gauges
  for (auto gauge_itr = ledger.gauges.cbegin();
      gauge_itr != ledger.gauges.cend();
      ++gauge_itr) {
    std::string key = gauge_itr->first;
    std::string metric_name = this->prefix + ::config->prefixGauge + this->process_name(key);

    std::string value = std::to_string(
      static_cast<long double>(gauge_itr->second));

    stat_strings[this->hashring->get(key)] +=
      metric_name + " "
                  + value
                  + ts_suffix;

    ++num_stats;
  }

  // sets
  for (auto set_itr = ledger.sets.cbegin();
      set_itr != ledger.sets.cend();
      ++set_itr) {
    std::string key = set_itr->first;
    auto value = set_itr->second;
    std::string metric_name = this->prefix + ::config->prefixSet + this->process_name(key);

    stat_strings[this->hashring->get(key)] +=
      metric_name + ".count"
                  + " "
                  + std::to_string(static_cast<long long int>(value.size()))
                  + ts_suffix;
     ++num_stats;
  }

  // stats
  std::string num_stats_str =
    std::to_string(static_cast<long long int>(num_stats));

  std::string total_time =
    std::to_string(
      static_cast<long long unsigned int>(chrono::unixtime_ms() - start_time));

  std::string key = this->prefix_stats + ".numStats";
  stat_strings[this->hashring->get(key)] +=
    key + " "
        + num_stats_str
        + ts_suffix;

  key = this->prefix_stats + ".carbonStats.calculationtime";
  stat_strings[this->hashring->get(key)] +=
    key + " "
        + total_time
        + ts_suffix;

  for (auto statsd_metric_itr = ledger.statsd_metrics.cbegin();
      statsd_metric_itr != ledger.statsd_metrics.cend();
      ++statsd_metric_itr) {
    std::string key = this->prefix_stats + '.' + statsd_metric_itr->first;

    std::string value = std::to_string(
      static_cast<long long int>(statsd_metric_itr->second));

    stat_strings[this->hashring->get(key)] +=
      key + " "
          + value
          + ts_suffix;
  }

  for (auto stat_string_pair_itr = stat_strings.cbegin();
      stat_string_pair_itr != stat_strings.cend();
      ++stat_string_pair_itr) {
    auto& dest = stat_string_pair_itr->first;
    auto& stat_string = stat_string_pair_itr->second;

    auto itr = this->connections.find(dest);
    if (itr == this->connections.end()) {
      this->connections[dest] = std::move(net::Connection(dest));
      (this->connections[dest]).write(stat_string);
    } else {
      itr->second.write(stat_string);
    }
  }
}

}  // namespace backends
}  // namespace statsdcc
