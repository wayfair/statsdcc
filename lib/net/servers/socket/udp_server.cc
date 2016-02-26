
/**
 * Source file for class UDPServer
 * Please see udp_server.h and server.h for documentaion
 */

#include "statsdcc/net/servers/socket/udp_server.h"

#include <iostream>
#include <sstream>

#include "statsdcc/net/wrapper.h"
#include "statsdcc/os.h"

namespace statsdcc { namespace net { namespace servers { namespace socket {

UDPServer::UDPServer(int port,
                     int num_threads,
                     int recv_buffer,
                     std::shared_ptr<statsdcc::consumers::Consumer> consumer)
  : Server(num_threads, consumer),
    port(port),
    recv_buffer(recv_buffer) {
  try {
    for (int i = 0; i < this->num_threads; ++i) {
      this->servers.push_back(
        std::move(std::thread(&UDPServer::listen, this)));
    }
  } catch(...) {
    ::logger->error(
      "Thread Creation Error: unable to start udp server");
    this->done = true;
    throw;
  }
}

void UDPServer::listen() {
  this->add_tid(os::get_tid());

  int sockfd;

  sockfd = statsdcc::net::wrapper::socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    ::logger->info("Exiting server");
    return;
  }

  sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(this->port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;  // 100 millisec timeout

  setsockopt(sockfd,
             SOL_SOCKET,
             SO_RCVTIMEO,
             (char *)&tv,
             sizeof(struct timeval));

  // reuse port
  int reuse = 1;
  setsockopt(sockfd,
             SOL_SOCKET,
             SO_REUSEADDR,
             &reuse,
             sizeof(reuse));
  setsockopt(sockfd,
             SOL_SOCKET,
             SO_REUSEPORT,
             &reuse,
             sizeof(reuse));

  setsockopt(sockfd,
             SOL_SOCKET,
             SO_RCVBUF,
             &this->recv_buffer,
             sizeof(this->recv_buffer));

  if (statsdcc::net::wrapper::bind(
            sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) == -1) {
    statsdcc::net::wrapper::close(sockfd);
    ::logger->info("Exiting server");
    return;
  }

  ssize_t length = 0;
  char buff[MAX_MSG_SIZE + 1] = {0};
  while (!this->done) {
    length = statsdcc::net::wrapper::recvfrom(
        sockfd,
        buff,
        MAX_MSG_SIZE,
        0,
        NULL,
        NULL);
    if (length < 0) {
     if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
     else break;
    }

    buff[length] = '\0';

    std::istringstream pkt(buff);
    std::string metric;
    while (std::getline(pkt, metric)) {
      this->consumer->consume(metric);
    }
  }

  statsdcc::net::wrapper::close(sockfd);
}

}  // namespace socket
}  // namespace servers
}  // namespace net
}  // namespace statsdcc
