
#ifndef INCLUDE_STATSDCC_CONSUMERS_CONSUMER_H_
#define INCLUDE_STATSDCC_CONSUMERS_CONSUMER_H_

#include <string>

namespace statsdcc { namespace consumers {

/**
 * Interface for consumers
 */
class Consumer {
 public:
  Consumer() = default;

  Consumer(const Consumer&) = delete;
  Consumer& operator=(const Consumer&) = delete;

  Consumer(Consumer&&) = delete;
  Consumer& operator=(Consumer&&) = delete;

  virtual ~Consumer() = default;

  /**
   * This should defines what should be done with the metric
   * read from the socket
   *
   * @param metric metric to be processed
   */
  virtual void consume(const std::string& metric) const = 0;
};

}  // namespace consumers
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_CONSUMERS_CONSUMER_H_
