
#include "statsdcc/configs/proxy_config.h"

namespace statsdcc { namespace configs {

ProxyConfig::ProxyConfig(const Json::Value& json)
  : Config(json) {
  this->backends.stdout = false;
  Json::Value::Members ms = json["backends"].getMemberNames();
  for (auto itr = ms.begin(); itr != ms.end(); ++itr) {
    if (*itr == "aggregator") {
      auto ss = json["backends"]["aggregator"];
      for (auto sitr = ss.begin(); sitr != ss.end(); ++sitr) {
        auto host = sitr->get("host", "127.0.0.1").asString();
        auto port = sitr->get("port", 8125).asInt();
        this->backends.aggregators.emplace_back(Hostport(host, port));
      }
    } else if (*itr == "stdout") {
      this->backends.stdout = true;
    }
  }

  auto blklist = json["blacklist"];
  for (auto itr = blklist.begin(); itr != blklist.end(); ++itr) {
    this->blacklist.push_back(boost::regex(itr->asString()));
  }
}

Json::Value ProxyConfig::to_json() {
  Json::Value root = Config::to_json();

  root["backends"]["stdout"] = this->backends.stdout;

  for (auto itr = this->backends.aggregators.cbegin();
       itr != this->backends.aggregators.cend();
       ++itr) {
    Json::Value aggregator;
    aggregator["host"] = itr->host;
    aggregator["port"] = itr->port;
    root["backends"]["aggregator"] = aggregator;
  }

  return root;
}

}  // namespace configs
}  // namespace statsdcc

