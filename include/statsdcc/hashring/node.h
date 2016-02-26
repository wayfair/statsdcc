
#ifndef INCLUDE_STATSDCC_HASHRING_NODE_H_
#define INCLUDE_STATSDCC_HASHRING_NODE_H_

#include <string>

#include "statsdcc/hostport.h"

namespace statsdcc { namespace hashring {

struct Node : public statsdcc::Hostport {
 private:
  typedef statsdcc::Hostport super;

 public:
  inline Node() {}
  inline Node(const std::string& host, int port, int vnodes, int weight) :
    super(host, port),
    vnodes(vnodes),
    weight(weight) {
  }

  inline Node(const Node& node) :
    super(node.host, node.port),
    vnodes(node.vnodes),
    weight(node.weight) {
  }

  inline Node& operator=(const Node& node) {
    this->host = node.host;
    this->port = node.port;
    this->vnodes = node.vnodes;
    this->weight = node.weight;
    return *this;
  }

  inline Node(Node&& node) :
    super(std::move(node.host), std::move(node.port)),
    vnodes(std::move(node.vnodes)),
    weight(std::move(node.weight)) {
  }

  inline Node& operator=(Node&& node) {
    this->host = std::move(node.host);
    this->port = std::move(node.port);
    this->vnodes = std::move(node.vnodes);
    this->weight = std::move(node.weight);
    return *this;
  }

  ~Node() = default;

  int vnodes;
  int weight;
  int key;
};

}  // namespace hashring
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_HASHRING_NODE_H_
