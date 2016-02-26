
#ifndef INCLUDE_STATSDCC_NET_SERVERS_SOCKET_UDP_SERVER_H_
#define INCLUDE_STATSDCC_NET_SERVERS_SOCKET_UDP_SERVER_H_

#include <sys/fcntl.h>

#include <memory>
#include <string>
#include <vector>

#include "statsdcc/net/servers/socket/server.h"
#include "statsdcc/net/wrapper.h"

namespace statsdcc { namespace net { namespace servers { namespace socket {

class UDPServer : public Server {
 public:
  /**
   * A constructor
   *
   * @param port port to listen for messages on
   * @param num_threads number of threads should be reading from the socket
   * @param consumer a refernce to Consumer object that has implementation for
   *                 consume(std::string& metric) method
   */
  UDPServer(int port,
            int num_threads,
            int recv_buffer,
            std::shared_ptr<statsdcc::consumers::Consumer> consumer);

  UDPServer(const UDPServer&) = delete;

  UDPServer& operator=(const UDPServer&) = delete;

  UDPServer(UDPServer&&) = delete;
  UDPServer& operator=(UDPServer&&) = delete;

  ~UDPServer() = default;

  /**
   * Starts the treads to process in comming data by calling consumer object
   */
  void start();

 private:
  int port;
  int recv_buffer;

  void listen();
};

}  // namespace socket
}  // namespace servers
}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_SERVERS_SOCKET_UDP_SERVER_H_
