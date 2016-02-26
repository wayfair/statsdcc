
#ifndef INCLUDE_STATSDCC_AGGREGATOR_H_
#define INCLUDE_STATSDCC_AGGREGATOR_H_

#include <getopt.h>
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "statsdcc/configs/aggregator_config.h"
#include "statsdcc/consumers/aggregator_consumer.h"
#include "statsdcc/logger.h"
#include "statsdcc/version.h"
#include "statsdcc/net/servers/socket/server.h"
#include "statsdcc/net/servers/http/aggregator/http_server.h"
#include "statsdcc/clock.h"

using namespace statsdcc;

std::shared_ptr<Logger> logger;
std::unique_ptr<configs::AggregatorConfig> config;

std::shared_ptr<BackendContainer> backend_container;
std::vector<std::unique_ptr<workers::aggregator::Worker>> workers;
std::unique_ptr<Clock> _clock;

std::shared_ptr<consumers::AggregatorConsumer> consumer;
std::vector<std::unique_ptr<net::servers::socket::Server>> servers;

std::unique_ptr<net::servers::http::aggregator::HttpServer>
http_server;

inline void usage(const char* name) {
  std::cout << "Usage:   " << name << " [options] <config file>" << std::endl;
  std::cout << "  -h, --help     show this help" << std::endl;
  std::cout << "      --version  show build version" << std::endl;
  exit(0);
}

inline void version(const char* name) {
  std::cout << name << " " << __VERSION
                           << " "
                           << __BUILD
                           << (__DEBUG ? " (debug)" : "")
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

  ::config.reset(new configs::AggregatorConfig(json));
  ::logger = Logger::get_logger(::config->log_level);
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

  // clock should be stopped before destructing workers
  _clock.reset();

  // stop workers
  for (auto worker_ptr_itr = ::workers.begin();
      worker_ptr_itr != ::workers.end();
      ++worker_ptr_itr) {
    (*worker_ptr_itr).reset();
  }

  exit(0);
}

/**
 * release all resources on SIGINT, and SIGTERM
 */
void sig_handler(int signum) {
  Logger::get_logger(
    Logger::LogLevel::info)->info(
      "Caught " + std::to_string(static_cast<long long int>(signum))
                + ", stopping threads...");
  stop();
}

#endif  // INCLUDE_STATSDCC_AGGREGATOR_H_

