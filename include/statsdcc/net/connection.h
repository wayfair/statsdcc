
#ifndef INCLUDE_STATSDCC_NET_CONNECTION_H_
#define INCLUDE_STATSDCC_NET_CONNECTION_H_

#include <iostream>
#include <string>

#include "statsdcc/hostport.h"
#include "statsdcc/net/lib.h"
#include "statsdcc/net/wrapper.h"

namespace statsdcc { namespace net {

class Connection {
 public:
  inline Connection()
    : sockfd(-1) {}

  inline explicit Connection(const Hostport& hostport) {
    this->hostport = hostport;
    this->sockfd = net::connect(hostport);
  }

  inline Connection(const Connection& other)
    : hostport(other.hostport),
      sockfd(other.sockfd) {}

  inline Connection& operator=(const Connection& other) {
    this->hostport = other.hostport;
    this->sockfd = other.sockfd;
    return *this;
  }

  inline Connection(Connection&& other)
    : hostport(std::move(other.hostport)),
      sockfd(std::move(other.sockfd)) {
    other.sockfd = -1;
  }

  inline Connection& operator=(Connection&& other) {
    this->hostport = std::move(other.hostport);
    this->sockfd = std::move(other.sockfd);
    other.sockfd = -1;
    return *this;
  }

  inline ~Connection() {
    this->close();
  }

  inline ssize_t read(void* buf, size_t n) {
    return wrapper::readn(this->sockfd, buf, n);
  }

  inline bool write(const std::string& str) {
    if (this->is_bad() ||
        wrapper::writen(this->sockfd, str.c_str(), str.length()) == -1) {
      return this->refresh() && this->write(str);
    }
    return true;
  }

  inline bool is_bad() {
    return this->sockfd == -1;
  }

  inline int close() {
    if (this->is_bad()) return 0;
    return wrapper::close(this->sockfd);
  }

  Hostport hostport;
  int sockfd;

 private:
  inline bool refresh() {
    if (this->is_bad()) {
      this->sockfd = net::connect(this->hostport);
    } else {
      pollfd sockfd_a[] = { { this->sockfd, 0, 0 } };
      wrapper::poll(sockfd_a, 1, 0);
      if (sockfd_a[0].revents & POLLHUP) {
        this->close();
        this->sockfd = net::connect(this->hostport);
      }
    }
    return !this->is_bad();
  }
};

}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_CONNECTION_H_
