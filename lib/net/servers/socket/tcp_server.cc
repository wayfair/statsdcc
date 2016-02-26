
/**
 * Source file for class TCPServer
 * Please see tcp_server.h and server.h for documentaion
 */

#include "statsdcc/net/servers/socket/tcp_server.h"

#include <sstream>

#include "statsdcc/logger.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/os.h"

namespace statsdcc { namespace net { namespace servers { namespace socket {

TCPServer::TCPServer(int port, std::shared_ptr<statsdcc::consumers::Consumer> consumer)
  : Server(1, consumer) {
  this->sockfd = statsdcc::net::wrapper::socket(AF_INET, SOCK_STREAM, 0);
  if (this->sockfd == -1) throw std::string("Failed to create socket");

  sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (statsdcc::net::wrapper::bind(
            this->sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) == -1) {
    statsdcc::net::wrapper::close(this->sockfd);
    throw std::string("Failed to bind");
  }

  if (statsdcc::net::wrapper::listen(this->sockfd, 1) == -1) {
    throw std::string("Listen failed");
  }
  try {
    this->servers.emplace_back(&TCPServer::listen, this);
  } catch(...) {
    ::logger->error("Thread Creation Error: unable to start tcp server");
    this->done = true;
    throw;
  }
}

void TCPServer::listen() {
  this->add_tid(os::get_tid());

  fd_set active_fd_set, read_fd_set;

  FD_ZERO(&active_fd_set);
  FD_SET(this->sockfd, &active_fd_set);

  char metric[MAX_MSG_SIZE] = {0};
  int pos = 0;
  while (!this->done) {
    read_fd_set = active_fd_set; // always re-initialize read_fd_set
    struct timeval tv = {0, 0};
    if (statsdcc::net::wrapper::select(FD_SETSIZE,
                                       &read_fd_set,
                                       NULL,
                                       NULL,
                                       &tv) <= 0) continue;

    for (int i = 0; i < FD_SETSIZE; ++i) {
      if (FD_ISSET(i, &read_fd_set)) {
        if (i == this->sockfd) {
          // There was a connection request
          int conn_fd = statsdcc::net::wrapper::accept(this->sockfd,
                                                       NULL,
                                                       NULL);
          if (conn_fd == -1) continue;
          FD_SET(conn_fd, &active_fd_set);
        } else {
          // There is data to read
          char buffer[MAX_MSG_SIZE];
          int bytes_read = statsdcc::net::wrapper::readn(i,
                                                         buffer,
                                                         MAX_MSG_SIZE - 1);
          if (bytes_read <= 0) {
            statsdcc::net::wrapper::close(i);
            FD_CLR(i, &active_fd_set);
          } else {
            for (int i = 0; i <= bytes_read; ++i) {
              if (buffer[i] == '\n') {
                metric[pos] = '\0';
                //this->consumer->consume(metric);
                pos = 0;
              } else {
                metric[pos++] = buffer[i];
              }
            }
          }
        }
      }
    }
  }

  statsdcc::net::wrapper::close(this->sockfd);
}

}  // namespace socket
}  // namespace servers
}  // namespace net
}  // namespace statsdcc
