
/**
 * Library of utility functions
 */

#ifndef INCLUDE_STATSDCC_CHRONO_H_
#define INCLUDE_STATSDCC_CHRONO_H_

#include <chrono>
#include <cstdint>
#include <string>

namespace statsdcc { namespace chrono {

/**
 * Returns currect time since epoch in milliseconds.
 *
 * @return epoch time in milliseconds
 */
inline uint64_t unixtime_ms() {
  return (std::chrono::system_clock::now().time_since_epoch()
           / std::chrono::milliseconds(1));
}

/**
 * Returns current time as string
 *
 * @return current time in format appended by a space char,
 *   [WD MMM DD HH:MM [AM/PM]] (Example: [Thu Aug 20 02:55 PM])
 */
inline std::string current_time() {
  std::time_t t = std::time(NULL);
  char time_str[100];

  std::strftime(time_str, sizeof(time_str),
                "%a %b %d %I:%M %p", std::localtime(&t));

  return time_str;
}

}  // namespace chrono
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_CHRONO_H_
