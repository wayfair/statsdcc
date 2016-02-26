
/**
 * readn, writen to read and write to stream sockets
 */
#ifndef INCLUDE_STATSDCC_NET_SOCK_STREAM_IO_H_
#define INCLUDE_STATSDCC_NET_SOCK_STREAM_IO_H_

#include <unistd.h>

#include <cerrno>

namespace statsdcc { namespace net { namespace sock_stream_io {

/**
 * from Unix Network Programming text book:
 *
 * A read or write on a stream socket might input or output fewer bytes than requested, but this is not an error condition.
 * The reason is that buffer limits might be reached for the socket in the kernel.
 * All that is required to input or output the remaining bytes is for the caller to invoke the read or write function again.
 * This scenario is always a possibility on a stream socket with read, but is normally seen with write only if the socket is nonblocking.
 * Nevertheless, we always call our writen function instead of write, in case the implementation returns a short count.
 */

/**
 * Read "n" bytes from a descriptor.
 *
 * @param fd file descriptor to read
 * @param buf buffer to read to
 * @param n number of bytes to be read
 *
 * @return number of bytes read, 0 on EOF, and -1 on error
 */
inline ssize_t readn(int fd, void *buf, size_t n) {
  char *ptr = static_cast<char *>(buf);
  size_t nleft = n;  // number of bytes left to read

  while (nleft > 0) {
    ssize_t nread = 0;  // number of bytes read so far
    if (-1 == (nread = ::read(fd, ptr, nleft))) {
      if (errno == EINTR) {
        nread = 0;  // and call read() again
      } else {
        return (-1);  // real error
      }
    } else if (nread == 0) {
      break;  // EOF
    }
    nleft -= nread;
    ptr += nread;
  }
  return (n - nleft);  // return >= 0
}

/**
 * Write "n" bytes to a descriptor.
 *
 * @param fd file descriptor to read
 * @param buf buffer to write from
 * @param n number of bytes to be written
 *
 * @return number of bytes written, and -1 on error
 */
inline ssize_t writen(int fd, const void *buf, size_t n) {
  const char *ptr = static_cast<const char *>(buf);
  size_t nleft = n;

  while (nleft > 0) {
    ssize_t nwritten;
    if (-1 == (nwritten = ::write(fd, ptr, nleft))) {
      if (nwritten < 0 && errno == EINTR) {
        nwritten = 0;  // and call write() again
      } else {
        return (-1);  // error
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

}  // namespace sock_stream_io
}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_SOCK_STREAM_IO_H_
