
/**
 * Source file for Worker class
 * Please see documentation in Worker.h
 */

#include <cstdlib>
#include <iostream>
#include <utility>

#include "statsdcc/os.h"
#include "statsdcc/workers/aggregator/worker.h"

namespace statsdcc { namespace workers { namespace aggregator {

int Worker::max_id;

Worker::Worker(
  const std::shared_ptr<BackendContainer> &backend_container)
  : done(false),
    current_ledger(new Ledger()),
    backend_container(backend_container),
    interrupt_flag(false) {
  // update id
  this->id = ++Worker::max_id;
  try {
    this->thread_guard = std::unique_ptr<ThreadGuard>(new ThreadGuard(
          std::move(std::thread(&Worker::process, this))));
  } catch(...) {
    ::logger->error("Thread Creation Error: unable to start");
    this->done = true;
    throw;
  }
}

void Worker::process() {
  ::logger->info("Worker started...");
  this->ttids.push_back(os::get_tid());

  std::unique_ptr<ThreadGuard> flusher_guard;

  char* metric_ptr = NULL;
  std::string metric;

  int count = 0;
  while (!this->done) {
    if (!this->work_queue.pop(metric_ptr)) {
      // sleeping and polling is better performant than conditional variables
      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
      continue;
    }

    metric = std::string(metric_ptr);
    tc_free(metric_ptr);

    if (this->interrupt_flag == true) {
      // process and flush ledger
      this->current_ledger->process();

      // always pass the copy of the ledger to the flusher
      flusher_guard.reset(
        new ThreadGuard(
          std::thread(&BackendContainer::flush,
            this->backend_container,
            Ledger(*this->current_ledger),
            this->id)));

      this->bad_lines_seen = this->current_ledger->bad_lines_seen();

      this->set_metrics_frequency(this->current_ledger->frequency);

      // delete previous ledger and create new one
      this->current_ledger.reset(new Ledger());

      this->metrics_rate = count / ::config->frequency;
      count = 0;
      this->interrupt_flag = false;
    }

    this->current_ledger->buffer(metric);
    ++count;
  }

  ::logger->info("Worker stopped...");
}

}  // namespace aggregator
}  // namespace workers
}  // namespace statsdcc
