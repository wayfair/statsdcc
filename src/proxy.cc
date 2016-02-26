
#include "statsdcc/proxy.h"

#include <csignal>

#include "statsdcc/os.h"
#include "statsdcc/server_factory.h"

int main(int argc, char** argv) {
  std::signal(SIGINT, sig_handler);
  std::signal(SIGTERM, sig_handler);

  if (argc == 1) usage(argv[0]);
  options(argc, argv);

  ::logger = Logger::get_logger();
  configure(argv[optind]);

  try {
    ::worker = std::unique_ptr<workers::proxy::Worker>(
               new workers::proxy::Worker(config->workers));
  } catch(...) {
    ::logger->error("Failed to instanstiate Worker");
    exit(0);
  }

  ::consumer = std::make_shared<consumers::ProxyConsumer>(*::worker);

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
      http_server = std::unique_ptr<net::servers::http::proxy::HttpServer>(
                      new net::servers::http::proxy::HttpServer(
                        ::config->servers.http.port, os::get_tid()));
    }
  } catch (std::string err) {
    ::logger->error(err);
    ::logger->info("Stopping threads and exiting");
    stop();
  } catch (...) {
    ::logger->error("Expection raised");
    ::logger->info("Stopping threads and exiting");
    stop();
  }

  os::pause();
  return 0;
}
