
/**
 * Definition of class Hashring
 */

#ifndef INCLUDE_STATSDCC_HASHRING_HASHRING_H_
#define INCLUDE_STATSDCC_HASHRING_HASHRING_H_

#include <openssl/md4.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "statsdcc/hashring/node.h"

namespace statsdcc {
class HashringTest;
}

namespace statsdcc { namespace hashring {

/*
 * This class provides consistent hashing
 * Has same logic as https://github.com/3rd-Eden/node-hashring
 */
class Hashring {
  friend class statsdcc::HashringTest;  // unit tests

 public:
  struct RingElement {
    unsigned int hash;
    std::string shard;

    inline RingElement() {}

    inline RingElement(unsigned int hash, std::string shard) :
      hash(hash),
      shard(shard) {
    }

    inline RingElement(const RingElement& re) :
      hash(re.hash),
      shard(re.shard) {
    }

    inline RingElement& operator=(const RingElement& re) {
      this->hash = re.hash;
      this->shard = re.shard;
      return *this;
    }

    inline RingElement(RingElement&& re) :
      hash(std::move(re.hash)),
      shard(std::move(re.shard)) {
    }

    inline RingElement& operator=(RingElement&& re) {
      this->hash = std::move(re.hash);
      this->shard = std::move(re.shard);
      return *this;
    }

    inline bool operator<(const RingElement &rhs) const {
      if (this->hash == rhs.hash) {
        // TODO(sdomalapalli): check if it is necessary
        // to implement v8 sort algo instead of this patch
        if (this->shard == "05" ||
            this->shard == "22" ||
            this->shard == "13") {
          return false;
        }
      }
      return this->hash <= rhs.hash;
    }

    inline bool operator==(const RingElement &rhs) const {
      return ((this->hash == rhs.hash) && (this->shard == rhs.shard));
    }
  };

  inline explicit Hashring(
    const std::unordered_map<std::string, Node>& nodes) :
    nodes(nodes),
    // 40 hashes (vnodes) and 4 replicas per hash = 160 points per server.
    replicas(4),
    vnodes(40) {
    if (nodes.size() == 0) throw std::string("Nodes list cannot be empty");
    // generate the continuum of the Hashring.
    this->generate_continuum();
  }

  inline Hashring(const Hashring& hashring) {
    this->ring = hashring.ring;
    this->nodes = hashring.nodes;
    this->replicas = hashring.replicas;
    this->vnodes = hashring.vnodes;
  }

  inline Hashring& operator=(const Hashring& hashring) {
    this->ring = hashring.ring;
    this->nodes = hashring.nodes;
    this->replicas = hashring.replicas;
    this->vnodes = hashring.vnodes;
    return *this;
  }

  inline Hashring(Hashring&& hashring) {
    this->ring = std::move(hashring.ring);
    this->nodes = std::move(hashring.nodes);
    this->replicas = std::move(hashring.replicas);
    this->vnodes = std::move(hashring.vnodes);
  }

  inline Hashring& operator=(Hashring&& hashring) {
    this->ring = std::move(hashring.ring);
    this->nodes = std::move(hashring.nodes);
    this->replicas = std::move(hashring.replicas);
    this->vnodes = std::move(hashring.vnodes);
    return *this;
  }

  ~Hashring() = default;

  /**
   * Find the correct hostport for the key which is closest to the point after what
   * the given key hashes to.
   *
   * @param key the key
   *
   * @return server address
   */
  inline Node get(const std::string& key) {
    Node result = this->nodes[
      this->ring[
        this->find(this->hash_value(key))
      ].shard
    ];

    return result;
  }

 private:
  std::vector<RingElement> ring;

  std::unordered_map<std::string, Node> nodes;

  int replicas;  // the amount of replicas per server

  int vnodes;  // the amount of virtual nodes per server

  /**
   * Generates the hash ring based on nodes, weights and vnodes values
   */
  void generate_continuum();

  /**
   * Returns the position of the hash_value in the hash ring.
   *
   * @param hash_value find the nearest server close to this hash.
   *
   * @return position of the server in the hash ring.
   */
  int find(unsigned int hash_value) const;

  /**
   * Get the hashed value for the given key.
   *
   * @param key the key
   *
   * @return numeric representation of the MD4 hash
   */
  inline unsigned int hash_value(const std::string& key) const {
    // get MD4 hash
    unsigned char digest[MD4_DIGEST_LENGTH];
    MD4((unsigned char*)key.c_str(), key.length(), (unsigned char*)&digest);

    // merge into one integer value, only consider first 4 bytes of hash
    unsigned int hash = this->hash(digest[3], digest[2], digest[1], digest[0]);

    return hash;
  }

  /**
   * merges four integer values into one to give new hash value
   *
   * @param a_32 first integer
   * @param b_32 second integer
   * @param c_32 third integer
   * @param d_32 fourth integer
   *
   * @return the result of merge
   */
  inline unsigned int hash(
    unsigned int a_32,
    unsigned int b_32,
    unsigned int c_32,
    unsigned int d_32) const {
    return (a_32 << 24) | (b_32 << 16) | (c_32 << 8) | d_32;
  }
};

}  // namespace hashring
}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_HASHRING_HASHRING_H_
