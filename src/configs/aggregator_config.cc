
#include "statsdcc/configs/aggregator_config.h"

#include <cstdint>

#include <iostream>

namespace statsdcc { namespace configs {

AggregatorConfig::AggregatorConfig(const Json::Value& json)
  : Config(json) {
  this->name = json.get("name", "statsdcc").asString();
  this->prefix = json.get("prefix", "").asString();
  if(!this->prefix.empty()) {
    this->prefix = this->prefix + ".";
  }

  this->prefixCounter = json.get("prefix_counter", "counters").asString();
  if(!this->prefixCounter.empty()) {
    this->prefixCounter = this->prefixCounter + ".";
  }
  this->prefixTimer = json.get("prefix_timer", "timers").asString();
  if(!this->prefixTimer.empty()) {
    this->prefixTimer = this->prefixTimer + ".";
  }
  this->prefixGauge = json.get("prefix_gauge", "gauges").asString();
  if(!this->prefixGauge.empty()) {
    this->prefixGauge = this->prefixGauge + ".";
  }
  this->prefixSet = json.get("prefix_set", "sets").asString();
  if(!this->prefixSet.empty()) {
    this->prefixSet = this->prefixSet + ".";
  }

  this->frequency = json.get("frequency", 10).asInt();

  Json::Value ps = json["percentiles"];
  if (ps.size() == 0) {
    this->percentiles.push_back(90);
  } else {
    for (auto itr = ps.begin(); itr != ps.end(); ++itr) {
      this->percentiles.push_back(itr->asInt());
    }
  }

  this->backends.stdout = false;
  Json::Value::Members ms = json["backends"].getMemberNames();
  for (auto itr = ms.begin(); itr != ms.end(); ++itr) {
    if (*itr == "carbon") {
      auto cs = json["backends"]["carbon"];
      int i = 0;
      for (auto citr = cs.begin(); citr != cs.end(); ++citr) {
        auto shard = citr->get("shard", std::to_string(
                       static_cast<long long int>(i))).asString();
        auto host = citr->get("host", "127.0.0.1").asString();
        auto port = citr->get("port", 3000).asInt();
        auto vnodes = citr->get("vnodes", 1000).asInt();
        auto weight = citr->get("weight", 1).asInt();
        this->backends.carbon[shard]
          = hashring::Node(host, port, vnodes, weight);
        ++i;
      }
    } else if (*itr == "repeater") {
      auto rs = json["backends"]["repeater"];
      for (auto ritr = rs.begin(); ritr != rs.end(); ++ritr) {
        auto host = ritr->get("host", "127.0.0.1").asString();
        auto port = ritr->get("port", 3000).asInt();
        this->backends.repeaters.push_back(statsdcc::Hostport(host, port));
      }
    } else if (*itr == "stdout") {
      this->backends.stdout = true;
    }
  }
}

Json::Value AggregatorConfig::to_json() {
  Json::Value root = Config::to_json();

  root["name"] = this->name;
  root["workers"] = this->workers;
  root["frequency"] = this->frequency;

  for (auto itr = this->percentiles.cbegin();
       itr != this->percentiles.cend();
       ++itr) {
    root["percentiles"].append(*itr);
  }

  root["backends"]["stdout"] = this->backends.stdout;

  for (auto itr = this->backends.carbon.cbegin();
       itr != this->backends.carbon.cend();
       ++itr) {
    Json::Value carbon;
    carbon["shard"] = itr->first;
    carbon["host"] = itr->second.host;
    carbon["port"] = itr->second.port;
    carbon["vnodes"] = itr->second.vnodes;
    carbon["weight"] = itr->second.weight;
    root["backends"]["carbon"].append(carbon);
  }

  for (auto itr = this->backends.repeaters.cbegin();
       itr != this->backends.repeaters.cend();
       ++itr) {
    Json::Value repeater;
    repeater["host"] = itr->host;
    repeater["port"] = itr->port;
    root["backends"]["repeater"].append(repeater);
  }

  return root;
}

}  // namespace configs
}  // namespace statsdcc
