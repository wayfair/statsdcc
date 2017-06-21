
/**
 * Source file for class Ledger
 * Please see Ledger.h for documentation
 */

#include "statsdcc/ledger.h"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>

#include "statsdcc/chrono.h"
#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/logger.h"

namespace statsdcc {

// buffers the metric
void Ledger::buffer(const std::string& metric) {
  const char *metric_csty = metric.c_str();

  char name_buffer[MAX_TOKEN_BUFFER_SIZE] = {0};
  char type_buffer[3] = {0};
  char metric_value_buffer[MAX_TOKEN_BUFFER_SIZE] = {0};
  double metric_value = -std::numeric_limits<double>::max();
  double sample_rate = 1;
  MetricType type = MetricType::counter;

  // record number of calls to buffer method
  ++this->statsd_metrics["metrics_processed"];

  // setup the names for the stats stored in counters
  std::string bad_lines_seen = ::config->name + ".bad_lines_seen";

  sscanf(metric_csty,
         "%" MAX_TOKEN_STRING_LENGTH "[^:]:" \
         "%" MAX_TOKEN_STRING_LENGTH "[^|]|" \
         "%2[^|]|"                           \
         "@%lf",
         name_buffer, metric_value_buffer, type_buffer, &sample_rate);

  // trim leading spaces if any
  char *tmp_type = type_buffer;
  while (*tmp_type == ' ' && *tmp_type != '\0')
    ++tmp_type;
  std::string metric_name(std::move(name_buffer));
  std::string metric_type_str(tmp_type);

  // track bad lines
  // using find() intead of == so that we can ignore trailing spaces
  bool bad_line = false;
  if ("" == metric_type_str) {
    bad_line = true;
  } else if (metric_type_str.find("ms") == 0) {
    type = MetricType::timer;
    try {
      metric_value = boost::lexical_cast<double>(metric_value_buffer);
    } catch(...) {
      bad_line = true;
    }
  } else if (metric_type_str.find("c") == 0) {
    type = MetricType::counter;
    try {
      metric_value = boost::lexical_cast<double>(metric_value_buffer);
    } catch(...) {
      bad_line = true;
    }
  } else if (metric_type_str.find("g") == 0) {
    type = MetricType::gauge;
    try {
      metric_value = boost::lexical_cast<double>(metric_value_buffer);
    } catch(...) {
      bad_line = true;
    }
  } else if (metric_type_str.find("s") == 0) {
    type = MetricType::set;
    bad_line = false;
  } else {
    try {
      metric_value = boost::lexical_cast<double>(metric_value_buffer);
    } catch(...) {
      bad_line = true;
    }
  }

  if (bad_line) {
    ::logger->info("Bad line: " + metric);
    ++this->counters[bad_lines_seen];
    return;
  }

  ++this->frequency[metric_name];

  switch (type) {
    case MetricType::timer:
      this->timer_counters[metric_name] += (1 / sample_rate);
      this->timers[metric_name].push_back(metric_value);
      break;

    case MetricType::gauge:
      {
        // check if +/- is specified
        char char_after_colon = metric_csty[metric.find_first_of(":") + 1];
        if (('+' == char_after_colon) || ('-' == char_after_colon)) {
          this->gauges[metric_name] += metric_value;
        } else {
          this->gauges[metric_name] = metric_value;
        }
        break;
      }
    case MetricType::set:
      this->sets[metric_name].insert(metric_value_buffer);
      break;

    default:
      this->counters[metric_name] += metric_value * (1 / sample_rate);
      break;
  }
}

// Aggregates buffered metrics
void Ledger::process() {
  std::uint64_t start_time = chrono::unixtime_ms();

  // process counters
  // calculate "per second" rate
  for (auto counter_itr = this->counters.cbegin();
      counter_itr != this->counters.cend();
      ++counter_itr) {
    this->counter_rates[counter_itr->first] =
      counter_itr->second / ::config->frequency;
  }

  // process timers
  for (auto timer_key_value_pair_itr = this->timers.cbegin();
      timer_key_value_pair_itr != this->timers.cend();
      ++timer_key_value_pair_itr) {
    std::unordered_map<std::string, double> current_timer_data;
    std::string key = timer_key_value_pair_itr->first;

    if (key.length() <= 0) {
      current_timer_data["count"] = current_timer_data["count_ps"] = 0;
    } else {
      // get sorted values
      std::vector<double> values(timer_key_value_pair_itr->second);
      std::sort(values.begin(), values.end());

      // get count, sum, mean, min, and max
      int count = values.size();
      int mid = count / 2;
      double min = values.front();
      double max = values.back();
      double sum = 0;
      for (auto value_itr = values.cbegin();
          value_itr != values.cend();
          ++value_itr) {
        sum += *value_itr;
      }
      double mean = sum / count;

      double median = 0;
      if (count % 2 == 0) {
          median = (values[mid-1] + values[mid]) / 2;
      } else {
          median = values[mid];
      }

      // initialize sum, mean, and threshold boundary
      double threshold_boundary = max;

      for (auto threshold_itr = ::config->percentiles.cbegin();
          threshold_itr != ::config->percentiles.cend();
          ++threshold_itr) {
        double pct =
          (*threshold_itr < 0) ? -(*threshold_itr) : (*threshold_itr);
         // Int is potentially too short since we could have more than 65K metrics in the flush window
         unsigned long num_in_threshold = 0;

        if (count > 1) {
          num_in_threshold = round(pct / 100 * count);
          if (0 == num_in_threshold) {
            continue;
          }
          threshold_boundary = values[num_in_threshold - 1];
        }

        double pct_sum = 0;
        for (unsigned long i = 0; i < num_in_threshold; ++i) {
            pct_sum += values[i];
        }
        double pct_mean = pct_sum / num_in_threshold;

        // generate pct name
        char clean_pct[17] = {0};
        snprintf(clean_pct, sizeof(clean_pct), "%g", pct);
        for (unsigned int i = 0; i < strlen(clean_pct); ++i) {
          clean_pct[i] = ('.' == clean_pct[i]) ? '_' : clean_pct[i];
        }

        current_timer_data[std::string("upper_") + std::string(clean_pct)] = threshold_boundary;
        current_timer_data[std::string("count_") + std::string(clean_pct)] = num_in_threshold;
        current_timer_data[std::string("sum_") + std::string(clean_pct)] = pct_sum;
        current_timer_data[std::string("mean_") + std::string(clean_pct)] = pct_mean;

      }  // foreach percentile

      current_timer_data["upper"] = max;
      current_timer_data["lower"] = min;
      current_timer_data["count"] = this->timer_counters[key];
      current_timer_data["mean"] = mean;
      current_timer_data["median"] = median;
    }

    this->timer_data[key] = current_timer_data;
  }  // foreach metric

  this->statsd_metrics["processing_time"] =
    chrono::unixtime_ms() - start_time;
}

}  // namespace statsdcc
