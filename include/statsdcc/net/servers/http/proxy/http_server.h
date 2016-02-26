
#ifndef INCLUDE_STATSDCC_NET_SERVERS_HTTP_PROXY_HTTP_SERVER_H_
#define INCLUDE_STATSDCC_NET_SERVERS_HTTP_PROXY_HTTP_SERVER_H_

#include <microhttpd.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "statsdcc/logger.h"
#include "statsdcc/net/servers/socket/server.h"

namespace statsdcc {
namespace net {
namespace servers {
namespace http {
namespace proxy {

/**
 * starts http server to process requests
 */
class HttpServer {
 public:
  /**
   * A constructor
   *
   * @param port that server should listen on
   * @param main_pid pid of the main thread
   */
  inline HttpServer(int port, int main_pid) {
    HttpServer::main_pid = main_pid;
    this->server = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                                    port,
                                    NULL,
                                    NULL,
                                    &HttpServer::handle_request,
                                    NULL,
                                    MHD_OPTION_END);
    if (this->server == NULL) throw std::string("Failed to start HTTP server");
  }

  HttpServer(const HttpServer&) = delete;
  HttpServer& operator=(const HttpServer&) = delete;

  HttpServer(HttpServer&&) = delete;
  HttpServer& operator=(HttpServer&&) = delete;

  inline ~HttpServer() {
    MHD_stop_daemon(this->server);
  }

 private:
  static int handle_request(void *cls,
                            struct MHD_Connection *connection,
                            const char *url,
                            const char *method,
                            const char *version,
                            const char *upload_data,
                            size_t *upload_data_size,
                            void **con_cls);

   /**
    * Returns the status of clock, server, and worker threads
    *
    * @param keys params that should be included in the result (e.g., State)
    *
    * @return the status of clock, server, and worker threads
    */
  static Json::Value get_proc_status(const std::vector<std::string> &keys);

   /**
    * Returns the status of cpu and memory usage on the system
    *
    * @param keys params that should be included in the result (e.g., MemFree)
    *
    * @return the status of cpu and memory usage on the system
    */
  static Json::Value get_sys_info(const std::vector<std::string> &keys);

   /**
    * Returns the application health
    *
    * @return the application health
    */
  static Json::Value get_app_status();

  struct MHD_Daemon *server;

  static int main_pid;
};

}  // namespace proxy
}  // namespace http
}  // namespace servers
}  // namespace net
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_NET_SERVERS_HTTP_PROXY_HTTP_SERVER_H_
