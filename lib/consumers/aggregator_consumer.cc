
/**
 * Source file for class AggregatorConsumer
 * Please see statsdcc_consumer.h and consumer.h for documentaion
 */

#include "statsdcc/consumers/aggregator_consumer.h"

#include <gperftools/tcmalloc.h>
#include <string.h>

#include <cstring>
#include <sstream>

#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/workers/aggregator/worker.h"

namespace statsdcc { namespace consumers {

AggregatorConsumer::AggregatorConsumer() {
  std::string bad_keys_key = ::config->name + ".bad_keys";
  this->bad_keys_metric = bad_keys_key + ":1|c";
  this->bad_keys_queue_hash =
    this->str_hash(bad_keys_key) % ::config->workers;
}

void AggregatorConsumer::consume(const std::string& metric) const {

  std::size_t pos = metric.find_first_of(":");
  if (std::string::npos != pos) {
    auto idx = str_hash(metric.substr(0, pos)) % ::config->workers;
    // explicit tc_malloc is faster than strdup
    //  - possibly because length() is O(1)
    char* metric_ptr = (char*) tc_malloc(metric.length() + 1);
    if (metric_ptr == NULL) {
      ::logger->warn("unable to tc_malloc");
      return;
    }

    std::strcpy(metric_ptr, metric.c_str());
    ::workers[idx]->submit(metric_ptr);
  } else {
    char* metric_ptr = (char*) tc_malloc(this->bad_keys_metric.length() + 1);
    if (metric_ptr == NULL) {
      ::logger->warn("unable to tc_malloc");
      return;
    }

    std::strcpy(metric_ptr, this->bad_keys_metric.c_str());
    ::workers[this->bad_keys_queue_hash]->submit(metric_ptr);
  }

}

}  // namespace consumers
}  // namespace statsdcc
