
/**
 * Class definition for Worker
 */

#ifndef INCLUDE_STATSDCC_WORKERS_PROXY_WORKER_H_
#define INCLUDE_STATSDCC_WORKERS_PROXY_WORKER_H_

#include <gperftools/tcmalloc.h>
#include <boost/lockfree/queue.hpp>

#include <string>
#include <vector>
#include <memory>
#include <queue>

#include "statsdcc/atomic.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/threads_info_interface.h"
#include "statsdcc/thread_guard.h"

namespace statsdcc { namespace workers { namespace proxy {

/**
 * work queue and worker thread combination
 * Processes, aggregates submitted metrics and sends the metrics to backends.
 */
class Worker : public ThreadsInfoInterface {
 public:
  inline Worker() {}

  explicit Worker(int num_threads);

  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;

  Worker(Worker&&) = delete;
  Worker& operator=(Worker&&) = delete;

  inline ~Worker() {
    this->done = true;
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

  // counters used by http server - appstat
  std::atomic_ullong metrics_dropped;
  std::atomic_ullong bad_lines_seen;

 private:
  // job that worker thread keeps running until done
  void process();

  // flag to indicate the thread to stop
  std::atomic_bool done;

  // the metrics submitted by the main thread
  // will be saved to this queue for processing
  // TODO(sdomalapalli): figure out the right queue capacity for prod
  boost::lockfree::queue<char*,
                         boost::lockfree::fixed_sized<true>,
                         boost::lockfree::capacity<10000>> work_queue;

  // writer socket
  int sockfd;

  // destinations
  std::vector<sockaddr_in> destinations;

  // thread guard
  std::vector<std::unique_ptr<ThreadGuard>> thread_guards;
};

}  // namespace proxy
}  // namespace workers
}  // namespace statsdcc

extern std::unique_ptr<statsdcc::workers::proxy::Worker> worker;

#endif  // INCLUDE_STATSDCC_WORKERS_PROXY_WORKER_H_
