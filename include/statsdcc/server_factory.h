
#ifndef INCLUDE_STATSDCC_SERVER_FACTORY_H_
#define INCLUDE_STATSDCC_SERVER_FACTORY_H_

#include <memory>

#include "statsdcc/net/servers/socket/server.h"
#include "statsdcc/net/servers/socket/tcp_server.h"
#include "statsdcc/net/servers/socket/udp_server.h"

namespace statsdcc {

/**
 * Creates UDP and/or TCP servers
 */
class ServerFactory {
 public:
  ServerFactory() = delete;
  ServerFactory(const ServerFactory&) = delete;
  ServerFactory& operator=(const ServerFactory&) = delete;

  ServerFactory(ServerFactory&&) = delete;
  ServerFactory& operator=(ServerFactory&&) = delete;

  ~ServerFactory() = default;

  /**
   * Creates and returns a TCP server object
   *
   * @param port port to listen for messages on
   * @param consumer a refernce to Consumer object that has implementation for
   *                 consume(std::string& metric) method
   *
   * @return a pointer to TCP server object
   */
  static inline std::unique_ptr<statsdcc::net::servers::socket::Server>
    get_tcp_server(
    int port,
    std::shared_ptr<statsdcc::consumers::Consumer> consumer) {
    return std::unique_ptr<statsdcc::net::servers::socket::Server>(
             new statsdcc::net::servers::socket::TCPServer(port, consumer));
  }

  /**
   * Creates and returns a TCP server object
   *
   * @param port port to listen for messages on
   * @param num_threads number of threads should be reading from the socket
   * @param consumer a refernce to Consumer object that has implementation for
   *                 consume(std::string& metric) method
   *
   * @return a pointer to TCP server object
   */
  static inline std::unique_ptr<statsdcc::net::servers::socket::Server>
    get_udp_server(
    int port,
    int num_threads,
    int recv_buffer,
    std::shared_ptr<statsdcc::consumers::Consumer> consumer) {
    return std::unique_ptr<statsdcc::net::servers::socket::Server>(
             new statsdcc::net::servers::socket::UDPServer(
               port, num_threads, recv_buffer, consumer));
  }
};

}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_SERVER_FACTORY_H_
