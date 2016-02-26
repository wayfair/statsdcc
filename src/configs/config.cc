
#include "statsdcc/configs/config.h"

namespace statsdcc { namespace configs {

Config::Config(const Json::Value& json) {
  this->servers.tcp.port  = json["servers"]["tcp"].get("port", -1).asInt();
  this->servers.tcp.enabled = this->servers.tcp.port != -1;

  this->servers.udp.port  = json["servers"]["udp"].get("port", -1).asInt();
  this->servers.udp.threads = json["servers"]["udp"].get("threads", 1).asInt();
  this->servers.udp.recv_buffer =
    json["servers"]["udp"].get("recv_buffer", 8388608).asInt();
  this->servers.udp.enabled = this->servers.udp.port != -1;

  this->servers.http.port  = json["servers"]["http"].get("port", 8080).asInt();
  this->servers.http.enabled =
    json["servers"]["http"].get("enabled", true).asBool();

  this->workers = json.get("workers", 1).asInt();

  auto level = json.get("log_level", "warn").asString();
  if (level == "debug") {
    this->log_level = Logger::LogLevel::debug;
  } else if (level == "info") {
    this->log_level = Logger::LogLevel::info;
  } else if (level == "warn") {
    this->log_level = Logger::LogLevel::warn;
  } else if (level == "error") {
    this->log_level = Logger::LogLevel::error;
  } else {
    this->log_level = Logger::LogLevel::warn;
  }
}

Json::Value Config::to_json() {
  Json::Value root;


  if (this->servers.tcp.enabled) {
    root["servers"]["tcp"]["port"] = this->servers.tcp.port;
  }

  if (this->servers.udp.enabled) {
    root["servers"]["udp"]["port"] = this->servers.udp.port;
    root["servers"]["udp"]["threads"] = this->servers.udp.threads;
  }

  if (this->servers.http.enabled) {
    root["servers"]["http"]["port"] = this->servers.http.port;
  }

  if (this->log_level == Logger::LogLevel::debug) {
    root["log_level"] = "debug";
  } else if (this->log_level == Logger::LogLevel::info) {
    root["log_level"] = "info";
  } else if (this->log_level == Logger::LogLevel::info) {
    root["log_level"] = "warn";
  } else if (this->log_level == Logger::LogLevel::error) {
    root["log_level"] = "error";
  }

  return root;
}

}  // namespace configs
}  // namespace statsdcc
