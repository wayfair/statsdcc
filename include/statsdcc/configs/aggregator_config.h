
#ifndef INCLUDE_STATSDCC_CONFIGS_AGGREGATOR_CONFIG_H_
#define INCLUDE_STATSDCC_CONFIGS_AGGREGATOR_CONFIG_H_

#include <json/json.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "statsdcc/configs/config.h"
#include "statsdcc/hashring/node.h"

namespace statsdcc { namespace configs {

class AggregatorConfig : public Config {
 public:
  inline AggregatorConfig() {}
  explicit AggregatorConfig(const Json::Value&);

  AggregatorConfig(const AggregatorConfig&) = delete;
  AggregatorConfig& operator=(const AggregatorConfig&) = delete;

  AggregatorConfig(AggregatorConfig&&) = delete;
  AggregatorConfig& operator=(AggregatorConfig&&) = delete;

  ~AggregatorConfig() = default;

  Json::Value to_json();

  std::string name;  // default: statsdcc

  std::string prefix;

  std::string prefixCounter;
  std::string prefixTimer;
  std::string prefixGauge;
  std::string prefixSet;

  int frequency;       // default: 10

  std::vector<int> percentiles;  // default: [90]

  struct {
    // map carbon shard to weighted host:port
    std::unordered_map<std::string, hashring::Node> carbon;

    std::vector<statsdcc::Hostport> repeaters;

    bool stdout;
  } backends;
};

}  // namespace configs
}  // namespace statsdcc

extern std::unique_ptr<statsdcc::configs::AggregatorConfig> config;

#endif  // INCLUDE_STATSDCC_CONFIGS_AGGREGATOR_CONFIG_H_
