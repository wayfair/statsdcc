
/**
 * Definition of graphite class
 */

#ifndef INCLUDE_STATSDCC_BACKENDS_STDOUT_H_
#define INCLUDE_STATSDCC_BACKENDS_STDOUT_H_

#include <string>

#include "statsdcc/backends/backend.h"
#include "statsdcc/ledger.h"

namespace statsdcc { namespace backends {

/**
 * Backend class for graphite.
 * Provides methods to flush metrics to graphite.
 */
class Stdout: public statsdcc::backends::Backend {
 public:
  Stdout() = default;

  Stdout(const Stdout&) = delete;
  Stdout& operator=(const Stdout&) = delete;

  Stdout(Stdout&&) = delete;
  Stdout& operator=(Stdout&&) = delete;

  ~Stdout() = default;

  /*
   * Processes mertic buffer for posting to graphite.
   *
   * @param ledger ledger to be flushed
   * @param flusher_id id of the consumer that initiated the flush
   */
  void flush_stats(const Ledger& ledger, int flusher_id);
};

}  // namespace backends
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_BACKENDS_STDOUT_H_
