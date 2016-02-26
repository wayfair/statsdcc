
#ifndef INCLUDE_STATSDCC_CONFIGS_CONFIG_H_
#define INCLUDE_STATSDCC_CONFIGS_CONFIG_H_

#include <json/json.h>

#include "statsdcc/logger.h"

namespace statsdcc { namespace configs {

class Config {
 public:
  inline Config() {}
  explicit Config(const Json::Value& json);

  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  Config(Config&&) = delete;
  Config& operator=(Config&&) = delete;

  ~Config() = default;

  virtual Json::Value to_json();

  // tcp, udp, http servers and ports to listen on
  struct {
    struct {
      int port;
      bool enabled;
    } tcp;
    struct {
      int port;
      int threads;
      bool enabled;
      int recv_buffer;
    } udp;
    struct {
      int port;
      bool enabled;
    } http;
  } servers;

  int workers;         // default: 1

  Logger::LogLevel log_level;  // default: warn
};

}  // namespace configs
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_CONFIGS_CONFIG_H_
