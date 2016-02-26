
#ifndef INCLUDE_STATSDCC_THREAD_GUARD_H_
#define INCLUDE_STATSDCC_THREAD_GUARD_H_

#include <thread>

#include "statsdcc/atomic.h"
#include "statsdcc/configs/config.h"
#include "statsdcc/logger.h"

namespace statsdcc {

class ThreadGuard {
 public:
  inline explicit ThreadGuard(std::thread&& t)
    : thr(std::move(t)) {}

  ThreadGuard(const ThreadGuard&) = delete;
  ThreadGuard& operator=(const ThreadGuard&) = delete;

  ThreadGuard(ThreadGuard&&) = delete;
  ThreadGuard& operator=(ThreadGuard&&) = delete;

  inline virtual ~ThreadGuard() {
    if (this->thr.joinable()) {
      this->thr.join();
      ::logger->debug("Thread Joined...");
    }
  }

 protected:
  std::thread thr;
};

}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_THREAD_GUARD_H_
