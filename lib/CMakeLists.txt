add_library (net net/lib.cc)

add_library (tcp_server net/servers/socket/tcp_server.cc)

add_library (udp_server net/servers/socket/udp_server.cc)

add_library (aggregator_http_server net/servers/http/aggregator/http_server.cc)
add_library (proxy_http_server net/servers/http/proxy/http_server.cc)

add_library (carbon backends/carbon.cc)
add_library (stdout backends/stdout.cc)
add_library (repeater backends/repeater.cc)

add_library (aggregator_consumer consumers/aggregator_consumer.cc)

add_library (hashring hashring/hashring.cc)

add_library (logger logger.cc)

add_library (status status.cc)
