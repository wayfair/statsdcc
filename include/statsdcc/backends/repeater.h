
/**
 * Definition of graphite class
 */

#ifndef INCLUDE_STATSDCC_BACKENDS_REPEATER_H_
#define INCLUDE_STATSDCC_BACKENDS_REPEATER_H_

#include <string>
#include <vector>

#include "statsdcc/backends/backend.h"
#include "statsdcc/ledger.h"

namespace statsdcc { namespace backends {

/**
 * Backend class for graphite.
 * Provides methods to flush metrics to graphite.
 */
class Repeater: public statsdcc::backends::Backend {
 public:
  Repeater();

  Repeater(const Repeater&) = delete;
  Repeater& operator=(const Repeater&) = delete;

  Repeater(Repeater&&) = delete;
  Repeater& operator=(Repeater&&) = delete;

  ~Repeater() = default;

  /*
   * Processes mertic buffer for posting to graphite.
   *
   * @param ledger ledger to be flushed
   * @param flusher_id id of the consumer that initiated the flush
   */
  void flush_stats(const Ledger& ledger, int flusher_id);

 private:
  inline void send(std::string metric) {
    for (unsigned int i = 0; i < this->destinations.size(); ++i) {
      metric = metric.substr(0, metric.size()-1);
      net::wrapper::sendto(this->sockfd,
        static_cast<const void*>(metric.c_str()),
        metric.length(),
        0,
        (const sockaddr*) &this->destinations[i],
        sizeof(this->destinations[i]));
    }
  }

  // writer socket
  int sockfd;

  // destinations
  std::vector<sockaddr_in> destinations;
};

}  // namespace backends
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_BACKENDS_REPEATER_H_
