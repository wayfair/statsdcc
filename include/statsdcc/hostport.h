
#ifndef INCLUDE_STATSDCC_HOSTPORT_H_
#define INCLUDE_STATSDCC_HOSTPORT_H_

#include <functional>
#include <string>

namespace statsdcc {

struct Hostport {
  inline Hostport() {}

  inline explicit Hostport(const std::string& hostport) {
    std::size_t pos = hostport.find(":");
    if (pos == std::string::npos) throw;  // FIXME
    this->host = hostport.substr(0, pos);
    this->port = std::stoi(hostport.substr(pos + 1));
  }

  inline Hostport(const std::string& host, int port) :
    host(host),
    port(port) {
  }

  inline Hostport(const std::string&& host, int&& port) :
    host(std::move(host)),
    port(std::move(port)) {
  }

  inline Hostport(const Hostport& hostport) :
    host(hostport.host),
    port(hostport.port) {
  }

  inline Hostport& operator=(const Hostport& hostport) {
    this->host = hostport.host;
    this->port = hostport.port;
    return *this;
  }

  inline Hostport(Hostport&& hostport) :
    host(std::move(hostport.host)),
    port(std::move(hostport.port)) {
  }

  inline Hostport& operator=(Hostport&& hostport) {
    this->host = std::move(hostport.host);
    this->port = std::move(hostport.port);
    return *this;
  }

  virtual ~Hostport() = default;

  inline bool operator==(Hostport const& hostport) const {
    return (hostport.port == this->port) && (hostport.host == this->host);
  }

  std::string host;
  int port;
};

}  // namespace statsdcc

namespace std {

template<>
struct hash<statsdcc::Hostport> {
  inline std::size_t operator()(const statsdcc::Hostport& hostport) const {
    return hash<std::string>()(hostport.host) ^ hash<int>()(hostport.port);
  }
};

}  // namespace std

#endif  // INCLUDE_STATSDCC_HOSTPORT_H_
