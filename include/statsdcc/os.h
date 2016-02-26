
#ifndef INCLUDE_STATSDCC_OS_H_
#define INCLUDE_STATSDCC_OS_H_

#include <linux/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace statsdcc { namespace os {

/**
 * @return the thread id of the caller
 */
inline int get_tid() {
  return static_cast<int>(syscall(__NR_gettid));
}

/**
 * wrapper for pause()
 * please see man pause
 */
inline int pause() {
  int ret;

  do {
    errno = 0;
    ret = ::pause();
  } while ((ret == -1) && (errno == EINTR));

  return ret;
}

}  // namespace os
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_OS_H_
