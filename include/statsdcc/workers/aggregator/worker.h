
/**
 * Class definition for Worker
 */

#ifndef INCLUDE_STATSDCC_WORKERS_AGGREGATOR_WORKER_H_
#define INCLUDE_STATSDCC_WORKERS_AGGREGATOR_WORKER_H_

#include <gperftools/tcmalloc.h>
#include <boost/lockfree/queue.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "statsdcc/atomic.h"
#include "statsdcc/backend_container.h"
#include "statsdcc/chrono.h"
#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/logger.h"
#include "statsdcc/threads_info_interface.h"
#include "statsdcc/thread_guard.h"

namespace statsdcc { namespace workers { namespace aggregator {

/**
 * work queue and worker thread combination
 * Processes, aggregates submitted metrics and sends the metrics to backends.
 */
class Worker : public ThreadsInfoInterface {
 public:
  inline Worker() {}

  explicit Worker(const std::shared_ptr<BackendContainer> &backend_container);

  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;

  Worker(Worker&&) = delete;
  Worker& operator=(Worker&&) = delete;

  inline ~Worker() {
    this->done = true;
  }

  // makes worker process the current ledger
  inline void interrupt() {
    this->interrupt_flag = true;
  }

  /**
   * Use this method to submit metric for processing.
   *
   * @param metric metric to be buffered and processed
   */
  inline void submit(char* metric) {
    if (metric == NULL) return;
    if (!this->work_queue.push(metric)) {
      ::logger->info(std::string("Queue full, dropped metric: ") + metric);
      ++this->metrics_dropped;
      tc_free(metric);
    }
  }

  inline std::unordered_map<std::string, long long int>
  get_metrics_frequency() {
    std::lock_guard<std::mutex> lck(this->fmut);
    return this->frequency;
  }

  template<class T1>
  inline void set_metrics_frequency(T1&& frequency) {
    std::lock_guard<std::mutex> lck(this->fmut);
    this->frequency = std::forward<T1>(frequency);
  }

  // counters used by http server - appstat
  std::atomic_ullong metrics_dropped;
  std::atomic_ulong metrics_rate;

  std::unordered_map<std::string, long long int> frequency;
  std::atomic_int bad_lines_seen;

 private:
  // current max id
  static int max_id;

  // id of this worker object
  int id;

  // thread guard
  std::unique_ptr<ThreadGuard> thread_guard;

  // flag to indicate the thread to stop
  std::atomic_bool done;

  // job that worker thread keeps running until done
  void process();

  // the metrics submitted by the main thread
  // will be saved to this queue for processing
  // TODO(sdomalapalli): figure out the right queue capacity for prod
  boost::lockfree::queue<char*,
                         boost::lockfree::fixed_sized<true>,
                         boost::lockfree::capacity<10000>> work_queue;

  // mutex to guard current_ledger
  mutable std::mutex lmut;

  // mutex to guard metric_frequency
  mutable std::mutex fmut;

  // current ledger being processed
  std::unique_ptr<Ledger> current_ledger;

  // pointer to backend container
  std::shared_ptr<BackendContainer> backend_container;

  // interrupt flag
  std::atomic_bool interrupt_flag;
};

}  // namespace aggregator
}  // namespace workers
}  // namespace statsdcc

extern
std::vector<std::unique_ptr<statsdcc::workers::aggregator::Worker>> workers;

#endif  // INCLUDE_STATSDCC_WORKERS_AGGREGATOR_WORKER_H_
