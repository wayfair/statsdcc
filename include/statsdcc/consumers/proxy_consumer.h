
#ifndef INCLUDE_STATSDCC_CONSUMERS_PROXY_CONSUMER_H_
#define INCLUDE_STATSDCC_CONSUMERS_PROXY_CONSUMER_H_

#include <gperftools/tcmalloc.h>

#include <cstring>
#include <string>
#include <vector>

#include "statsdcc/consumers/consumer.h"
#include "statsdcc/workers/proxy/worker.h"

namespace statsdcc { namespace consumers {

class ProxyConsumer : public Consumer {
 public:
  inline explicit ProxyConsumer(statsdcc::workers::proxy::Worker& worker)
    : worker(worker) {}

  ProxyConsumer(const ProxyConsumer&) = delete;
  ProxyConsumer& operator=(const ProxyConsumer&) = delete;

  ProxyConsumer(ProxyConsumer&&) = delete;
  ProxyConsumer& operator=(ProxyConsumer&&) = delete;

  ~ProxyConsumer() = default;

  inline void consume(const std::string& metric) const {
    char* metric_ptr = NULL;
    metric_ptr = static_cast<char*>(tc_malloc(metric.length() + 1));

    if (metric_ptr == NULL) {
      ::logger->warn("unable to tc_malloc");
      return;
    }

    std::strcpy(metric_ptr, metric.c_str());
    this->worker.submit(metric_ptr);
  }

 private:
  // refernce to worker
  statsdcc::workers::proxy::Worker& worker;
};

}  // namespace consumers
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_CONSUMERS_PROXY_CONSUMER_H_
