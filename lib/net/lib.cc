
/**
 * Source file for library
 * Please see lib.h for documentation
 */

#include "statsdcc/net/lib.h"

#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>
#include <iostream>

#include "statsdcc/hostport.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/wrapper.h"

namespace statsdcc { namespace net {

int connect(const Hostport& hostport) {
  return connect(hostport.host, hostport.port);
}

int connect(const std::string& host, unsigned int port) {
  int sockfd;
  struct sockaddr_in server_host;

  // initialize server address
  bzero(&server_host, sizeof(server_host));
  server_host.sin_family = AF_INET;
  server_host.sin_port = htons(port);

  int ip_res =
    wrapper::inet_pton(AF_INET, host.c_str(), &server_host.sin_addr);

  if (ip_res == 0) {
    if (resolve_ip(host, &server_host.sin_addr) == false) {
      return -1;
    }
  } else if (ip_res == -1) {
    return -1;
  }

  // create socket
  sockfd = wrapper::socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sockfd) return -1;

  // detect if the peer host crashes or becomes unreachable
  int optval = 1;
  socklen_t optlen = sizeof(optval);
  wrapper::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);

  // connect
  if (-1 == wrapper::connect(sockfd,
                             (struct sockaddr *)&server_host,
                             sizeof(server_host))) {
    wrapper::close(sockfd);
    return -1;
  }

  return sockfd;
}

bool resolve_ip(const std::string& host, in_addr* sin_addr) {
  struct hostent *hptr = gethostbyname(host.c_str());
  auto logger = Logger::get_logger();
  if (hptr == NULL) {
    logger->error(
      std::string("gethostbyname error: ") + hstrerror(h_errno)
    );
    h_errno = 0;
    return false;
  } else {
    if (hptr->h_addr_list[0] != NULL) {
      memcpy(sin_addr, hptr->h_addr_list[0], hptr->h_length);
      return true;
    } else {
      logger->error("gethostbyname error: No IP was resolved");
      return false;
    }
  }
}

}  // namespace net
}  // namespace statsdcc
