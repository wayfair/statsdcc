
/**
 * Source file for class Hashring
 * Please see hashring.h for documentation
 */
#include "statsdcc/hashring/hashring.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "statsdcc/hashring/node.h"

namespace statsdcc { namespace hashring {

void Hashring::generate_continuum() {
  // no nodes, bailout.
  if (!this->nodes.size()) {
    return;
  }

  // generate the total weight of all the servers.
  int total = 0;
  for (auto shard_node_pair_itr = this->nodes.cbegin();
      shard_node_pair_itr != this->nodes.cend();
      ++shard_node_pair_itr) {
    total += shard_node_pair_itr->second.weight;
  }

  for (auto shard_node_pair_itr = this->nodes.cbegin();
      shard_node_pair_itr != this->nodes.cend();
      ++shard_node_pair_itr) {
    auto& shard = shard_node_pair_itr->first;
    auto& hostport = shard_node_pair_itr->second;

    double percentage = static_cast<double>(hostport.weight) / total;
    int vnodes = (hostport.vnodes) ? hostport.vnodes : this->vnodes;
    int length = floor(percentage * vnodes * this->nodes.size());
    unsigned int key = 0;

    // if you supply us with a custom vnode size, we will use that instead of
    // our computed distribution.
    if ((0 != vnodes) && (vnodes != this->vnodes)) {
      length = vnodes;
    }

    for (int i = 0; i < length; ++i) {
      unsigned char digest[MD4_DIGEST_LENGTH] = {0};
      char shard_cstr[1024] = {0};
      snprintf(shard_cstr, sizeof(shard_cstr), "%s-%u", shard.c_str(), i);

      MD4((unsigned char*)shard_cstr,
          strlen(shard_cstr),
          (unsigned char*)&digest);

      for (int j = 0; j < this->replicas; ++j) {
        key = this->hash(
            digest[3 + j * 4],
            digest[2 + j *4],
            digest[1 + j *4],
            digest[j * 4]);
        this->ring.push_back(RingElement(key, shard));
      }
    }
  }

  std::stable_sort(this->ring.begin(), this->ring.end());
}

int Hashring::find(unsigned int hash_value) const {
  int size = this->ring.size();
  int low = 0;
  int high = size;
  unsigned int prev = 0;

  // preform a search on the vector to find the server with the next biggest
  // point after what the given key hashes to.
  while (true) {
    int mid = (low + high) >> 1;

    if (mid == size) {
      return 0;
    }

    unsigned int middle = this->ring[mid].hash;
    prev = (0 == mid) ? 0 : this->ring[mid - 1].hash;

    if (hash_value <= middle && hash_value > prev) {
      return mid;
    }

    if (middle < hash_value) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }

    if (low > high) {
      return 0;
    }
  }
}

}  // namespace hashring
}  // namespace statsdcc
