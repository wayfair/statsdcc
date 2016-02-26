#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>

#include "statsdcc/hashring/hashring.h"
#include "gtest/gtest.h"

namespace statsdcc {

class HashringTest: public ::testing::Test {
 protected:
  virtual void SetUp() {
    nodes["01"] = statsdcc::hashring::Node("127.0.0.1", 1000, 1000, 1);
    nodes["02"] = statsdcc::hashring::Node("127.0.0.2", 1000, 1000, 1);
    nodes["03"] = statsdcc::hashring::Node("127.0.0.3", 1000, 1000, 1);
    nodes["04"] = statsdcc::hashring::Node("127.0.0.4", 1000, 1000, 1);
    nodes["05"] = statsdcc::hashring::Node("127.0.0.5", 1000, 1000, 1);
    nodes["06"] = statsdcc::hashring::Node("127.0.0.6", 2000, 1000, 1);
    nodes["07"] = statsdcc::hashring::Node("127.0.0.7", 3000, 1000, 1);
    nodes["08"] = statsdcc::hashring::Node("127.0.0.8", 4000, 1000, 1);
    nodes["09"] = statsdcc::hashring::Node("127.0.0.9", 1000, 1000, 1);
    nodes["10"] = statsdcc::hashring::Node("127.0.0.10", 2000, 1000, 1);
    nodes["11"] = statsdcc::hashring::Node("127.0.0.11", 1000, 1000, 1);
    nodes["12"] = statsdcc::hashring::Node("127.0.0.12", 2000, 1000, 1);
    nodes["13"] = statsdcc::hashring::Node("127.0.0.13", 3000, 1000, 1);
    nodes["14"] = statsdcc::hashring::Node("127.0.0.14", 4000, 1000, 1);
    nodes["15"] = statsdcc::hashring::Node("127.0.0.15", 1000, 1000, 1);
    nodes["16"] = statsdcc::hashring::Node("127.0.0.16", 2000, 1000, 1);
    nodes["17"] = statsdcc::hashring::Node("127.0.0.17", 3000, 1000, 1);
    nodes["18"] = statsdcc::hashring::Node("127.0.0.18", 4000, 1000, 1);
    nodes["19"] = statsdcc::hashring::Node("127.0.0.19", 1000, 1000, 1);
    nodes["20"] = statsdcc::hashring::Node("127.0.0.20", 2000, 1000, 1);
    nodes["21"] = statsdcc::hashring::Node("127.0.0.21", 1000, 1000, 1);
    nodes["22"] = statsdcc::hashring::Node("127.0.0.22", 2000, 1000, 1);
    nodes["23"] = statsdcc::hashring::Node("127.0.0.23", 3000, 1000, 1);
    nodes["24"] = statsdcc::hashring::Node("127.0.0.24", 4000, 1000, 1);
    nodes["25"] = statsdcc::hashring::Node("127.0.0.25", 1000, 1000, 1);
    nodes["26"] = statsdcc::hashring::Node("127.0.0.26", 2000, 1000, 1);
    nodes["27"] = statsdcc::hashring::Node("127.0.0.27", 3000, 1000, 1);
    nodes["28"] = statsdcc::hashring::Node("127.0.0.28", 4000, 1000, 1);
    nodes["29"] = statsdcc::hashring::Node("127.0.0.29", 1000, 1000, 1);
    nodes["30"] = statsdcc::hashring::Node("127.0.0.30", 1000, 1000, 1);
    nodes["31"] = statsdcc::hashring::Node("127.0.0.31", 1000, 1000, 1);
    nodes["32"] = statsdcc::hashring::Node("127.0.0.32", 2000, 1000, 1);
    nodes["33"] = statsdcc::hashring::Node("127.0.0.33", 3000, 1000, 1);
    nodes["34"] = statsdcc::hashring::Node("127.0.0.34", 4000, 1000, 1);
    nodes["35"] = statsdcc::hashring::Node("127.0.0.35", 1000, 1000, 1);
    nodes["36"] = statsdcc::hashring::Node("127.0.0.36", 2000, 1000, 1);

    hashring = std::unique_ptr<statsdcc::hashring::Hashring>(
      new statsdcc::hashring::Hashring(nodes));

    std::ifstream data;
    data.open("./test/data/nodejs_hashring.data");
    if (data.is_open()) {
      std::string line;
      while (getline(data, line)) {
        unsigned int hash;
        char shard[3];
        if (line[0] != '%') {
          sscanf(line.c_str(), "%u %s", &hash, shard);
          ring_good.push_back(
            statsdcc::hashring::Hashring::RingElement(hash, shard));
        }
      }
    }
    data.close();
    hashring_ring = hashring->ring;

    data.open("./test/data/nodejs_hashring_get.data");
    if (data.is_open()) {
      std::string line;
      while (getline(data, line)) {
        char key[11];
        char shard[3];
        if (line[0] != '%') {
          sscanf(line.c_str(), "%s %s", key, shard);
          node_hashring_get_results[key] = shard;
        }
      }
    }
    data.close();

    for (auto itr = node_hashring_get_results.cbegin();
         itr != node_hashring_get_results.cend();
         ++itr) {
      hashring_get_results[itr->first] =
        hashring->ring[hashring->find(hashring->hash_value(itr->first))].shard;
    }
  }

  std::unordered_map<std::string, statsdcc::hashring::Node> nodes;
  std::unique_ptr<statsdcc::hashring::Hashring> hashring;
  std::vector<statsdcc::hashring::Hashring::RingElement> ring_good;
  std::vector<statsdcc::hashring::Hashring::RingElement> hashring_ring;

  std::unordered_map<std::string, std::string> hashring_get_results;
  std::unordered_map<std::string, std::string> node_hashring_get_results;
};

TEST_F(HashringTest, ring_test) {
  if (ring_good.size() != hashring_ring.size()) {
    EXPECT_EQ(ring_good.size(), hashring_ring.size());
    return;
  }
  for (unsigned int i = 0 ; i < ring_good.size(); ++i) {
    EXPECT_EQ(ring_good[i].hash, hashring_ring[i].hash);
    EXPECT_EQ(ring_good[i].shard, hashring_ring[i].shard);
  }
}

TEST_F(HashringTest, ring_get_test) {
  for (auto itr = node_hashring_get_results.cbegin();
       itr != node_hashring_get_results.cend();
       ++itr) {
    EXPECT_EQ(node_hashring_get_results[itr->first],
      hashring_get_results[itr->first]);
  }
}

}  // namespace statsdcc
