
#ifndef INCLUDE_STATSDCC_NET_LIB_H_
#define INCLUDE_STATSDCC_NET_LIB_H_

#include <netinet/in.h>

#include <string>

#include "statsdcc/hostport.h"

namespace statsdcc { namespace net {

/**
 * Create a TCP connection to the given hostport.
 *
 * @param Hostport A Hostport object
 *
 * @return socket file descriptor
 */
int connect(const Hostport& hostport);

/**
 * Create a TCP connection to the given host and port.
 *
 * @param host An IP or hostname
 * @param port A port number
 *
 * @return socket file descriptor
 */
int connect(const std::string& host, unsigned int port);

/**
 * Resolve the IP for the given hostname.
 *
 * @param host     A hostname
 * @param sin_addr An IP addr struct
 *
 * @post The IP address for the hostname is placed in sin_addr
 *
 * @return true on success
 */
bool resolve_ip(const std::string& host, in_addr* sin_addr);

}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_LIB_H_
