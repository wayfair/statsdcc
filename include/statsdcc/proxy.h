
#ifndef INCLUDE_STATSDCC_PROXY_H_
#define INCLUDE_STATSDCC_PROXY_H_

#include <getopt.h>
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "statsdcc/configs/proxy_config.h"
#include "statsdcc/workers/proxy/worker.h"
#include "statsdcc/consumers/proxy_consumer.h"
#include "statsdcc/logger.h"
#include "statsdcc/net/servers/socket/server.h"
#include "statsdcc/net/servers/http/proxy/http_server.h"
#include "statsdcc/version.h"

using namespace statsdcc;

std::shared_ptr<Logger> logger;
std::unique_ptr<configs::ProxyConfig> config;

std::unique_ptr<workers::proxy::Worker> worker;

std::shared_ptr<consumers::ProxyConsumer> consumer;

std::vector<std::unique_ptr<net::servers::socket::Server>> servers;

std::unique_ptr<net::servers::http::proxy::HttpServer>
http_server;

inline void usage(const char* name) {
  std::cout << "Usage:   " << name << " [options] <config file>" << std::endl;
  std::cout << "  -h, --help     show this help" << std::endl;
  std::cout << "      --version  show build version" << std::endl;
  exit(0);
}

inline void version(const char* name) {
  std::cout << name << " " << __VERSION << " "
                           << __BUILD << (__DEBUG ? " (debug)" : "")
                           << std::endl;
  exit(0);
}

inline void options(const int argc, char* const argv[]) {
  static std::string short_options = "h";
  static struct option long_options[] = {
    {"help",    no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {NULL,      0, NULL, 0}
  };

  int opt;
  while (true) {
    opt = getopt_long(argc, argv, short_options.c_str(), long_options, NULL);
    if (opt == -1) break;
    switch (opt) {
      case 'h':
        usage(argv[0]);
        break;

      case 'V':
        version(argv[0]);
        break;

      default:
        usage(argv[0]);
    }
  }
}

inline void configure(const std::string& filepath) {
  std::ifstream file(filepath, std::ifstream::in);
  if (!file.is_open()) {
    ::logger->error("unable to open config file");
    exit(1);
  }

  Json::Value json;
  file >> json;
  file.close();

  config.reset(new configs::ProxyConfig(json));
  ::logger = Logger::get_logger(config->log_level);
}

/**
 * releases all resources
 */
void stop() {
  // stop http server
  http_server.reset();

  // stop servers
  for (auto server_ptr_itr = ::servers.begin();
      server_ptr_itr != ::servers.end();
      ++server_ptr_itr) {
    (*server_ptr_itr).reset();
  }

  // stop workers
  worker.reset();

  exit(0);
}

void sig_handler(int signum) {
  stop();
}

#endif  // INCLUDE_STATSDCC_PROXY_H_
