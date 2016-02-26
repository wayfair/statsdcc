
/**
 * Source file for class graphite
 * Please see graphite.h and backend.h for documentaion
 */

#include "statsdcc/backends/repeater.h"

#include <iostream>
#include <unordered_map>

#include "statsdcc/os.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/net/lib.h"
#include "statsdcc/configs/aggregator_config.h"

namespace statsdcc { namespace backends {

Repeater::Repeater() {
  // create write socket
  this->sockfd = net::wrapper::socket(AF_INET, SOCK_DGRAM, 0);

  // set destinations
  for (auto itr = config->backends.repeaters.cbegin();
       itr != config->backends.repeaters.cend();
       ++itr) {
    struct sockaddr_in dest_host;
    bzero(&dest_host, sizeof(dest_host));
    dest_host.sin_family = AF_INET;
    dest_host.sin_port = htons(itr->port);

    int ip_res =
      net::wrapper::inet_pton(AF_INET, itr->host.c_str(), &dest_host.sin_addr);

    if (ip_res == 0) {
      if (net::resolve_ip(itr->host.c_str(), &dest_host.sin_addr) == false) {
        throw "unable to resolve ip";
      }
    } else if (ip_res == -1) {
        throw "inet_pton error";
    }

    this->destinations.push_back(dest_host);
  }
}

void Repeater::flush_stats(const Ledger& ledger, int flusher_id) {
  std::string timer_data_key;

  // prefix for aggregator stats
  auto prefix = ::config->name + ".thread_" +
                  std::to_string(static_cast<long long int>(flusher_id));

  // counters
  for (auto counter_itr = ledger.counters.cbegin();
      counter_itr != ledger.counters.cend();
      ++counter_itr) {
    std::string key = counter_itr->first;
    std::string value =
      std::to_string(static_cast<long double>(counter_itr->second));

    std::string value_per_second =
      std::to_string(static_cast<long double>(ledger.counter_rates.at(key)));

    this->send(key + ".rate:" + value_per_second + "|c\n" +
               key + ".count:" + value + "|c");

  }

  // timers
  for (auto timer_itr = ledger.timer_data.cbegin();
      timer_itr != ledger.timer_data.cend();
      ++timer_itr) {
    std::string key = timer_itr->first;
    std::string out = "";

    for (auto timer_data_itr = timer_itr->second.cbegin();
        timer_data_itr != timer_itr->second.cend();
        ++timer_data_itr) {
      std::string timer_data_key = timer_data_itr->first;

      std::string value = std::to_string(
        static_cast<long double>(timer_data_itr->second));

      out += key + "." + timer_data_key + ":" + value + "|ms";
    }
    out.erase(out.end() - 1);
    this->send(out);
  }

  // gauges
  for (auto gauge_itr = ledger.gauges.cbegin();
      gauge_itr != ledger.gauges.cend();
      ++gauge_itr) {
    std::string key = gauge_itr->first;

    std::string value = std::to_string(
      static_cast<long double>(gauge_itr->second));

    this->send(key + ":" + value + "|g");
  }

  // sets
  for (auto set_itr = ledger.sets.cbegin();
      set_itr != ledger.sets.cend();
      ++set_itr) {
    std::string key = set_itr->first;
    auto value = set_itr->second;

    this->send(key + ".count:" +
           std::to_string(static_cast<long long int>(value.size())) + "|s");

  }
}

}  // namespace backends
}  // namespace statsdcc
