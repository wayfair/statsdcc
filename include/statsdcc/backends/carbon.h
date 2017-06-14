
/**
 * Definition of graphite class
 */

#ifndef INCLUDE_STATSDCC_BACKENDS_CARBON_H_
#define INCLUDE_STATSDCC_BACKENDS_CARBON_H_

#include <boost/regex.hpp>

#include <memory>
#include <string>
#include <unordered_map>

#include "statsdcc//atomic.h"
#include "statsdcc/backends/backend.h"
#include "statsdcc/hashring/hashring.h"
#include "statsdcc/hostport.h"
#include "statsdcc/ledger.h"

namespace statsdcc { namespace backends {

/**
 * Backend class for graphite.
 * Provides methods to flush metrics to graphite.
 */
class Carbon: public statsdcc::backends::Backend {
 public:
  Carbon();

  Carbon(const Carbon&) = delete;
  Carbon& operator=(const Carbon&) = delete;

  Carbon(Carbon&&) = delete;
  Carbon& operator=(Carbon&&) = delete;

  ~Carbon() = default;

  /*
   * Processes mertic buffer for posting to graphite.
   *
   * @param ledger ledger to be flushed
   * @param flusher_id id of the consumer that initiated the flush
   */
  void flush_stats(const Ledger& ledger, int flusher_id);

 private:
  inline std::string process_name(std::string& name) {
    boost::regex regex_space("\\s+");
    boost::regex regex_slash("/");
    boost::regex regex_non_alphanum("[^a-zA-Z_\\-0-9\\.]");

    std::string result_name = boost::regex_replace(name, regex_space, "_");
    result_name = boost::regex_replace(result_name, regex_slash, "-");
    return boost::regex_replace(result_name, regex_non_alphanum, "");
  }

  std::unique_ptr<hashring::Hashring> hashring;

  // prefix for aggregator stats
  std::string prefix_stats;

  // prefix for all satts
  std::string prefix;
};

}  // namespace backends
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_BACKENDS_CARBON_H_
