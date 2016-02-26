
#ifndef INCLUDE_STATSDCC_NET_SERVERS_SOCKET_TCP_SERVER_H_
#define INCLUDE_STATSDCC_NET_SERVERS_SOCKET_TCP_SERVER_H_

#include <memory>
#include <string>
#include <vector>

#include "statsdcc/net/servers/socket/server.h"
#include "statsdcc/net/wrapper.h"

namespace statsdcc { namespace net { namespace servers { namespace socket {

class TCPServer : public Server {
 public:
  /**
   * A constructor
   *
   * @param port port to listen for messages on
   * @param consumer a refernce to Consumer object that has implementation for
   *                 consume(std::string& metric) method
   */
  TCPServer(int port, std::shared_ptr<statsdcc::consumers::Consumer> consumer);

  TCPServer(const TCPServer&) = delete;
  TCPServer& operator=(const TCPServer&) = delete;

  TCPServer(TCPServer&&) = delete;
  TCPServer& operator=(TCPServer&&) = delete;

  ~TCPServer() = default;

  /**
   * Starts the treads to process in comming data by calling consumer object
   */
  void start();

 private:
  void listen();

  int sockfd;
};

}  // namespace socket
}  // namespace servers
}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_SERVERS_SOCKET_TCP_SERVER_H_
