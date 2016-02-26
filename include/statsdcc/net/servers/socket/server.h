
#ifndef INCLUDE_STATSDCC_NET_SERVERS_SOCKET_SERVER_H_
#define INCLUDE_STATSDCC_NET_SERVERS_SOCKET_SERVER_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "statsdcc/consumers/consumer.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/threads_info_interface.h"
#include "statsdcc/thread_guard.h"

#define MAX_MSG_SIZE 1500

namespace statsdcc { namespace net { namespace servers { namespace socket {

/**
 * Interface for servers
 */
class Server : public statsdcc::ThreadsInfoInterface {
 public:
  /**
   * A constructor
   *
   * @param num_threads number of threads should be reading from the socket
   * @param consumer a refernce to Consumer object that has implementation for
   *                 consume(std::string& metric) method
   */
  inline Server(int num_threads,
                std::shared_ptr<statsdcc::consumers::Consumer> consumer)
    : num_threads(num_threads),
      done(false),
      consumer(consumer) {}

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  Server(Server&&) = delete;
  Server& operator=(Server&&) = delete;

  inline virtual ~Server() {
    this->done = true;
    this->join();
  }

 protected:
  inline void join() {
    for (auto server_itr = this->servers.begin();
         server_itr != this->servers.end();
         ++server_itr) {
      if (server_itr->joinable()) {
        server_itr->join();
        ::logger->debug("Server joined...");
      }
    }
  }

  int num_threads;
  std::atomic_bool done;
  std::shared_ptr<statsdcc::consumers::Consumer> consumer;
  std::vector<std::thread> servers;
};

}  // namespace socket
}  // namespace servers
}  // namespace net
}  // namespace statsdcc

extern std::vector<std::unique_ptr<statsdcc::net::servers::socket::Server>>
  servers;

#endif  // INCLUDE_STATSDCC_NET_SERVERS_SOCKET_SERVER_H_
