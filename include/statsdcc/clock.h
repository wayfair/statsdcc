
#ifndef INCLUDE_STATSDCC_CLOCK_H_
#define INCLUDE_STATSDCC_CLOCK_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <utility>
#include <vector>

#include "statsdcc/configs/config.h"
#include "statsdcc/logger.h"
#include "statsdcc/os.h"
#include "statsdcc/threads_info_interface.h"
#include "statsdcc/thread_guard.h"
#include "statsdcc/workers/aggregator/worker.h"

namespace statsdcc {

class Clock : public ThreadsInfoInterface {
 private:
  /**
   * calls interrupt on each Worker every flush interval
   */
  inline void wait_interrupt() {
    ::logger->info("Clock started...");
    this->ttids.push_back(os::get_tid());

    while (!this->done) {
      this->sleep();
      for (auto worker_itr = ::workers.cbegin();
          worker_itr != ::workers.cend();
          ++worker_itr) {
        (*worker_itr)->interrupt();
      }
    }
    ::logger->info("Clock stopped...");
  }

 public:
  inline Clock() :
    done(false) {
    try {
      this->thread_guard = std::unique_ptr<ThreadGuard>(new ThreadGuard(
            std::move(std::thread(&Clock::wait_interrupt, this))));
    } catch(...) {
      ::logger->error("Thread Creation Error: unable to start");
      throw;
    }
  }

  Clock(const Clock&) = delete;
  Clock& operator=(const Clock&) = delete;

  Clock(Clock&&) = delete;
  Clock& operator=(Clock&&) = delete;

  inline ~Clock() {
    this->done = true;
    this->done_cond.notify_one();
  }

  // cond variable to recieve notification from main
  std::condition_variable done_cond;

 private:
  /**
   * sleeps for flush interval or until notified
   */
  inline void sleep() {
    std::mutex mut;
    std::unique_lock<std::mutex> lck(mut);
    this->done_cond.wait_for(lck,
      std::chrono::seconds(config->frequency));
  }

  // thread guard
  std::unique_ptr<ThreadGuard> thread_guard;

  // flag to indicate the thread to stop
  std::atomic_bool done;
};
}  // namespace statsdcc

extern std::unique_ptr<statsdcc::Clock> _clock;

#endif  // INCLUDE_STATSDCC_CLOCK_H_
