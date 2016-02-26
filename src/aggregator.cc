
#include "statsdcc/aggregator.h"

#include <netinet/in.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>

#include "statsdcc/backend_container.h"
#include "statsdcc/server_factory.h"
#include "statsdcc/workers/aggregator/worker.h"

/**
 * Main thread for aggregator
 */
int main(int argc, char **argv) {
  std::signal(SIGINT, sig_handler);
  std::signal(SIGTERM, sig_handler);

  if (argc == 1) usage(argv[0]);
  options(argc, argv);

  ::logger = Logger::get_logger();
  configure(argv[optind]);

  if (!(::config->servers.tcp.enabled || ::config->servers.udp.enabled)) {
    ::logger->error("Please configure a least one server");
    exit(1);
  }

  ::logger->info("Initializing backends...");
  try {
    ::backend_container = std::make_shared<BackendContainer>();
  } catch(...) {
    ::logger->error("Failed to create BackendContainer");
    exit(1);
  }

  ::logger->info("Starting workers...");
  for (int i = 0; i < ::config->workers; ++i) {
    try {
      ::workers.push_back(
        std::unique_ptr<workers::aggregator::Worker>(
          new workers::aggregator::Worker(::backend_container)));
    } catch(...) {
      ::logger->error("Failed to instanstiate Worker");
      ::logger->info("Stopping threads and exiting");
      stop();
      exit(1);
    }
  }

  ::logger->info("Starting clock...");
  ::_clock = std::unique_ptr<Clock>(new Clock());

  ::logger->info("Starting server...");
  ::consumer = std::make_shared<consumers::AggregatorConsumer>();

  try {
    // start udp servers
    if (::config->servers.udp.enabled) {
      ::servers.push_back(
        ServerFactory::get_udp_server(
          ::config->servers.udp.port,
          ::config->servers.udp.threads,
          ::config->servers.udp.recv_buffer,
          ::consumer));
    }

    // start tcp servers
    if (::config->servers.tcp.enabled) {
        ::servers.push_back(
          ServerFactory::get_tcp_server(
            ::config->servers.tcp.port,
            ::consumer));
    }

    // start http server
    if (::config->servers.http.enabled) {
      http_server = std::unique_ptr<net::servers::http::aggregator::HttpServer>(
                      new net::servers::http::aggregator::HttpServer(
                        ::config->servers.http.port, os::get_tid()));
    }
  } catch (std::string err) {
    ::logger->error(err);
    ::logger->info("Stopping threads and exiting");
    stop();
    exit(1);
  }

  os::pause();
  return 0;
}
