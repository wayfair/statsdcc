
#ifndef INCLUDE_STATSDCC_BACKENDS_BACKEND_H_
#define INCLUDE_STATSDCC_BACKENDS_BACKEND_H_

#include <memory>

#include "statsdcc/hostport.h"
#include "statsdcc/ledger.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/connection.h"

namespace statsdcc { namespace backends {

class Backend {
 public:
  Backend() = default;

  Backend(const Backend&) = delete;
  Backend& operator=(const Backend&) = delete;

  Backend(Backend&&) = delete;
  Backend& operator=(Backend&&) = delete;

  virtual ~Backend() = default;

  virtual void flush_stats(const Ledger& ledger, int flusher_id) = 0;

  inline void flush_stats(const Ledger& ledger) {
    flush_stats(ledger, 0);
  }

 protected:
  std::unordered_map<Hostport, net::Connection> connections;
};

}  // namespace backends
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_BACKENDS_BACKEND_H_
