
/**
 * Wrappers for socket system calls
 */

#ifndef INCLUDE_STATSDCC_NET_WRAPPER_H_
#define INCLUDE_STATSDCC_NET_WRAPPER_H_

#include <arpa/inet.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "statsdcc/chrono.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/sock_stream_io.h"

namespace statsdcc { namespace net { namespace wrapper {

/**
 * wrapper for socket()
 * please see man socket
 */
inline int socket(int domain, int type, int protocol) {
  errno = 0;
  int sockfd = ::socket(domain, type, protocol);
  if (-1 == sockfd) {
    statsdcc::Logger::get_logger()->error(
      std::string("socket error: ") + std::strerror(errno));
  }
  return sockfd;
}

/**
 * wrapper for setsockopt()
 * please see setsockopt
 */
inline int setsockopt(int sockfd,
  int level,
  int optname,
  const void *optval,
  socklen_t optlen) {
  errno = 0;
  int ret = ::setsockopt(sockfd, level, optname, optval, optlen);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("setsockopt: ")+ std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for bind()
 * please see man bind
 */
inline int bind(int sockfd, const struct sockaddr *addr, socklen_t addlen) {
  errno = 0;
  int ret = ::bind(sockfd, addr, addlen);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("bind error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for listen()
 * please see man listen
 */
inline int listen(int sockfd, int backlog) {
  errno = 0;
  int ret = ::listen(sockfd, backlog);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("listen error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for accept()
 * please see man accept
 */
inline int accept(int sockfd,
  struct sockaddr *addr,
  socklen_t *addrlen) {
  int ret;

  // call accept again on EINTR
  do {
    errno = 0;
    ret = ::accept(sockfd, addr, addrlen);
  } while ((-1 == ret) && (errno == EINTR));

  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("accept error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for inet_pton()
 * please see inet_pton
 */
inline int inet_pton(int af, const char *src, void *dst) {
  int ret = ::inet_pton(af, src, dst);

  if (ret == 0) {  // may be hostname is provided instead of IP
    errno = 0;
  } else if (-1 == ret) {  // check if af contains a valid family
    statsdcc::Logger::get_logger()->error(
      std::string("inet_pton error: ") + std::strerror(errno));
    errno = 0;  // reset errno
  }

  return ret;
}

/**
 * wrapper for connect()
 * please see connect
 */
inline int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  errno = 0;
  int ret = ::connect(sockfd, addr, addrlen);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("connect error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper function for close()
 * please see close
 */
inline int close(int fd) {
  errno = 0;
  int ret = ::close(fd);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("close error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for writen()
 * please see sock_stream_io.h
 */
inline ssize_t writen(int fd, const void *buf, size_t n) {
  errno = 0;
  std::signal(SIGPIPE, SIG_IGN);  // ignore SIGPIPE

  ssize_t ret = sock_stream_io::writen(fd, buf, n);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("writen error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for readn()
 * please see sock_stream_io.h
 */
inline ssize_t readn(int fd, void *buf, size_t n) {
  errno = 0;
  ssize_t ret = sock_stream_io::readn(fd, buf, n);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("readn error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for recvfrom()
 * please see man recvfrom
 */
inline ssize_t recvfrom(int sockfd,
  void *buf,
  size_t len,
  int flags,
  struct sockaddr *src_addr,
  socklen_t *addrlen) {
  int ret;

  // call recvfrom again on EINTR
  do {
    errno = 0;
    ret = ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
  } while ((-1 == ret) && (errno == EINTR));

  if (-1 == ret && errno != EAGAIN && errno != EWOULDBLOCK) {
    statsdcc::Logger::get_logger()->error(
      std::string("recvfrom error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for sendto()
 * please see man sendto
 */
inline ssize_t sendto(int sockfd,
  const void *buf,
  size_t len,
  int flags,
  const struct sockaddr *dest_addr,
  socklen_t addrlen) {
  int ret;

  // call sendto again on EINTR
  do {
    errno = 0;
    ret = ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
  } while ((-1 == ret) && (errno == EINTR));

  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("sendto error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for select()
 * please see man select
 */
inline int select(int nfds,
  fd_set *readfds,
  fd_set *writefds,
  fd_set *exceptfds,
  struct timeval *timeout) {
  errno = 0;

  int ret = ::select(nfds, readfds, writefds, exceptfds, timeout);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("select error: ") + std::strerror(errno));
  }
  return ret;
}

/**
 * wrapper for poll()
 * please see man poll
 */
inline int poll(struct pollfd *fdarray, std::uint64_t nfds, int timeout) {
  errno = 0;

  int ret = ::poll(fdarray, nfds, timeout);
  if (-1 == ret) {
    statsdcc::Logger::get_logger()->error(
      std::string("poll error: ") + std::strerror(errno));
  }
  return ret;
}

}  // namespace wrapper
}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_WRAPPER_H_
