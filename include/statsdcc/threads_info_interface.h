
#ifndef INCLUDE_STATSDCC_THREADS_INFO_INTERFACE_H_
#define INCLUDE_STATSDCC_THREADS_INFO_INTERFACE_H_

#include <vector>
#include <mutex>

namespace statsdcc {

class ThreadsInfoInterface {
 public:
  ThreadsInfoInterface() = default;

  ThreadsInfoInterface(const ThreadsInfoInterface&) = delete;
  ThreadsInfoInterface& operator=(const ThreadsInfoInterface&) = delete;

  ThreadsInfoInterface(ThreadsInfoInterface&&) = delete;
  ThreadsInfoInterface& operator=(ThreadsInfoInterface&&) = delete;

  ~ThreadsInfoInterface() = default;

  /**
   * Returns the thread id
   *
   * @return the thread id
   */
  inline std::vector<int> get_tids() {
    return ttids;
  }

  /**
   * adds tid to the list
   *
   * @param tid thread id to be added
   */
  inline void add_tid(int tid) {
    std::lock_guard<std::mutex> lck(this->tmut);
    this->ttids.push_back(tid);
  }

 protected:
  // thread tid
  std::vector<int> ttids;

  // mutex to guard current_ledger
  mutable std::mutex tmut;
};
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_THREADS_INFO_INTERFACE_H_
