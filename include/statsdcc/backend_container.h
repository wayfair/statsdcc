
/**
 * Class definition for BackendContainer
 */

#ifndef INCLUDE_STATSDCC_BACKEND_CONTAINER_H_
#define INCLUDE_STATSDCC_BACKEND_CONTAINER_H_

#include <memory>
#include <mutex>
#include <vector>

#include "statsdcc/backends/backend.h"
#include "statsdcc/backends/carbon.h"
#include "statsdcc/backends/repeater.h"
#include "statsdcc/backends/stdout.h"
#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/logger.h"

namespace statsdcc {

/**
 * work queue and worker thread combination
 * Processes, aggregates submitted metrics and sends the metrics to backends.
 */
class BackendContainer {
 public:
  inline BackendContainer() {
    if (config->backends.carbon.size() > 0) {
      try {
        this->backends.push_back(std::unique_ptr<backends::Backend>(
              new backends::Carbon()));
      } catch(...) {
        ::logger->error("Failed to instanstiate backend Carbon");
        throw;
      }
    }
    if (config->backends.repeaters.size() > 0) {
      try {
        this->backends.push_back(std::unique_ptr<backends::Backend>(
              new backends::Repeater()));
      } catch(...) {
        ::logger->error("Failed to instanstiate backend Repeater");
        throw;
      }
    }
    if (config->backends.stdout) {
      this->backends.push_back(std::unique_ptr<backends::Backend>(
            new backends::Stdout()));
    }
  }

  BackendContainer(const BackendContainer&) = delete;
  BackendContainer& operator=(const BackendContainer&) = delete;

  BackendContainer(BackendContainer&&) = delete;
  BackendContainer& operator=(BackendContainer&&) = delete;

  ~BackendContainer() = default;

  /**
   * flushes the given ledger to the backends
   *
   * @param ledger to be flushed
   * @param fflusher_id id of the consumer that initiated the flush
   */
  inline void flush(const Ledger ledger, const int flusher_id) {
    std::lock_guard<std::mutex> lock(this->lmut);
    for (auto backend_itr = this->backends.cbegin();
        backend_itr != this->backends.cend();
        ++backend_itr) {
      (*backend_itr)->flush_stats(ledger, flusher_id);
    }
  }

 private:
  // mutex to guard current_ledger
  mutable std::mutex lmut;

  // list of backends the metrics should be flushed to
  std::vector<std::unique_ptr<backends::Backend>> backends;
};

}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_BACKEND_CONTAINER_H_
