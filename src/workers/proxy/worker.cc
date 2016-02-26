
/**
 * Source file for Worker class
 * Please see documentation in Worker.h
 */

#include <iostream>

#include "statsdcc/os.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/net/lib.h"
#include "statsdcc/workers/proxy/worker.h"
#include "statsdcc/configs/proxy_config.h"

namespace statsdcc { namespace workers { namespace proxy {

Worker::Worker(int num_threads)
  : done(false) {
  // create write socket
  this->sockfd = net::wrapper::socket(AF_INET, SOCK_DGRAM, 0);

  // set destinations
  for (auto itr = config->backends.aggregators.cbegin();
       itr != config->backends.aggregators.cend();
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

  // start threads
  try {
    for (int i = 0; i < num_threads; ++i) {
      this->thread_guards.push_back(
        std::unique_ptr<ThreadGuard>(
          new ThreadGuard(
            std::move(
              std::thread(&Worker::process, this)))));
    }
  } catch(...) {
    ::logger->error("Thread Creation Error: unable to start");
    this->done = true;
    throw;
  }
}

void Worker::process() {
  ::logger->info("Worker started...");
  this->ttids.push_back(os::get_tid());

  char* metric_ptr = NULL;
  std::string metric;
  std::hash<std::string> hash;
  while (!this->done) {
    if (!this->work_queue.pop(metric_ptr)) {
      // sleeping and polling is better performant than conditional variables
      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
      continue;
    }
    metric = std::string(metric_ptr);
    tc_free(metric_ptr);

    if (config->backends.stdout) {
      std::cout<< metric<< std::endl;
    }

    if (this->destinations.size() == 0) continue;

    std::size_t pos = metric.find(':');
    if (pos != std::string::npos) {
      std::size_t idx = hash(metric.substr(0, pos)) % this->destinations.size();
      net::wrapper::sendto(this->sockfd,
        static_cast<const void*>(metric.c_str()),
        metric.length(),
        0,
        (const sockaddr*) &this->destinations[idx],
        sizeof(this->destinations[idx]));
    } else {
      ++bad_lines_seen;
    }
  }

  ::logger->info("Worker stopped...");
}

}  // namespace proxy
}  // namespace workers
}  // namespace statsdcc
