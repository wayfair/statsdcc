
#ifndef INCLUDE_STATSDCC_CONSUMERS_AGGREGATOR_CONSUMER_H_
#define INCLUDE_STATSDCC_CONSUMERS_AGGREGATOR_CONSUMER_H_

#include <memory>
#include <string>
#include <vector>

#include "statsdcc/consumers/consumer.h"

namespace statsdcc { namespace consumers {

class AggregatorConsumer : public Consumer {
 public:
  AggregatorConsumer();

  AggregatorConsumer(const AggregatorConsumer&) = delete;
  AggregatorConsumer& operator=(
    const AggregatorConsumer&) = delete;

  AggregatorConsumer(AggregatorConsumer&&) = delete;
  AggregatorConsumer& operator=(AggregatorConsumer&&) = delete;

  ~AggregatorConsumer() = default;

  /**
   * Submits the metric to the worker thread
   * to be processed and sent to the backends
   *
   * @param metric metric to be processed
   */
  void consume(const std::string& metric) const;

 private:
  std::hash<std::string> str_hash;
  std::string bad_keys_metric;
  unsigned int bad_keys_queue_hash;
};

}  // namespace consumers
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_CONSUMERS_AGGREGATOR_CONSUMER_H_
