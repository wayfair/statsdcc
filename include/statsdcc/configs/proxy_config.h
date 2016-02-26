
#ifndef INCLUDE_STATSDCC_CONFIGS_PROXY_CONFIG_H_
#define INCLUDE_STATSDCC_CONFIGS_PROXY_CONFIG_H_

#include <json/json.h>

#include <memory>
#include <string>
#include <vector>

#include "statsdcc/configs/config.h"
#include "statsdcc/hostport.h"

namespace statsdcc { namespace configs {

class ProxyConfig : public Config {
 public:
  explicit ProxyConfig(const Json::Value&);

  ProxyConfig(const ProxyConfig&) = delete;
  ProxyConfig& operator=(const ProxyConfig&) = delete;

  ProxyConfig(ProxyConfig&&) = delete;
  ProxyConfig& operator=(ProxyConfig&&) = delete;

  ~ProxyConfig() = default;

  Json::Value to_json();

  struct {
    std::vector<Hostport> aggregators;

    bool stdout;
  } backends;
};

}  // namespace configs
}  // namespace statsdcc

extern std::unique_ptr<statsdcc::configs::ProxyConfig> config;

#endif  // INCLUDE_STATSDCC_CONFIGS_PROXY_CONFIG_H_
