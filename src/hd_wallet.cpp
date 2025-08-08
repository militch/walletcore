#include "wallet_core/hd_wallet.h"

#include <iostream>
#include <stdexcept>

#include "crypto/common.h"
#include "base58.h"
#include "bip32.h"
#include "hash.h"
#include "wallet_core/derivation_path.h"
#include "curve.h"

namespace {
using namespace wallet;
static HDNode get_root_node(const HDWallet& wallet) {
  return HDNode::fromSeed(wallet.getSeed());
}

static HDNode get_node(const HDWallet& wallet, const DerivationPath& path) {
  auto node = get_root_node(wallet);
  for (auto& ind : path.indices) {
    node = node.privateCkd(ind.derivationIndex());
  }
  return node;
}

static uint32_t node_fingerprint(HDNode& node) {
  node.fillPublicKey();
  std::array<byte, CHash160::OUTPUT_SIZE> digest;
  CHash160().Write(node.public_key_data).Finalize(digest);
  return ((uint32_t)digest[0] << 24) + (digest[1] << 16) + (digest[2] << 8) +
         digest[3];
}

static std::string node_serialize(const HDNode& node, uint32_t fingerprint,
                                  bool use_public) {
  std::array<byte, 78> buf;
  uint32_t version;
  byte* ptr = buf.data();
  // 4字节版本号
  if (use_public) {
    {
      WriteBE32(ptr, 0x0488B21E);
      ptr += 4;
    }
  } else {
    {
      WriteBE32(ptr, 0x0488ADE4);
      ptr += 4;
    }
  }
  // 1字节的深度
  {
    *ptr++ = node.depth;
  }
  // 4字节的父节点指纹
  {
    WriteBE32(ptr, fingerprint);
    ptr += 4;
  }
  // 4字节的子节点编号
  {
    WriteBE32(ptr, node.child_num);
    ptr += 4;
  }
  // 32字节的链码
  {
    std::copy(node.chain_code.begin(), node.chain_code.end(), ptr);
    ptr += 32;
  }
  if (use_public) {
    // 填充33字节公钥
    std::copy(std::begin(node.public_key_data), std::end(node.public_key_data),
              ptr);
  } else {
    // 填充0x00 || ser256(k)
    {
      *ptr++ = 0;
    }
    std::copy(std::begin(node.private_key_data),
              std::end(node.private_key_data), ptr);
  }
  return EncodeBase58Check(buf);
}

static void node_deserialize(const std::string& extended, HDNode* node) {
  std::vector<unsigned char> buf;
  if (!DecodeBase58Check(extended, buf, 78)) {
    throw std::runtime_error("");
  }
  const byte* ptr = buf.data();
  bool is_public;
  uint32_t version = ReadBE32(ptr);
  ptr += 4;
  if (version == 0x0488B21E) {
    // 扩展公钥
    is_public = true;
  } else if (version == 0x0488ADE4) {
    // 扩展私钥
    is_public = false;
  } else {
    throw std::runtime_error("");
  }
  node->depth = *ptr++;
  uint32_t _ = ReadBE32(ptr);
  ptr += 4;
  node->child_num = ReadBE32(ptr);
  ptr += 4;
  std::copy(ptr, ptr + 32, node->chain_code.begin());
  ptr += 32;
  if (is_public) {
    std::copy(ptr, ptr + 33, node->public_key_data);
  } else {
    if (*ptr++ != 0x00) {
      throw std::runtime_error("");
    }
    std::copy(ptr, ptr + 32, node->private_key_data);
  }
}
}  // namespace

namespace wallet {
static const uint32_t PURPOSE_BIP44 = static_cast<uint32_t>(Purpose::BIP44);

HDWallet::HDWallet(const std::vector<byte>& seed) {
  std::copy_n(seed.begin(), 64, this->seed_.begin());
}

HDWallet::HDWallet(const SeedData& seed)
    :seed_(seed) {}

const std::array<byte, 64>& HDWallet::getSeed() const { return this->seed_; }

PrivateKey HDWallet::getRootKey() const {
  auto root = get_root_node(*this);
  return PrivateKey(root.privateKey());
}

PrivateKey HDWallet::getKey(const DerivationPath& path) const {
  const auto node = get_node(*this, path);
  return PrivateKey(node.privateKey());
}

std::string HDWallet::getExtendedPrivateKeyAccount(uint32_t coin,
                                                   uint32_t account) const {
  const auto path = DerivationPath{{
      DerivationPathIndex{PURPOSE_BIP44, true},
      DerivationPathIndex{coin, true},
  }};
  auto node = get_node(*this, path);
  auto fingerprintValue = node_fingerprint(node);
  node = node.privateCkd(account + 0x80000000);
  return node_serialize(node, fingerprintValue, false);
}

std::string HDWallet::getExtendedPublicKeyAccount(uint32_t coin,
                                                  uint32_t account) const {
  const auto path = DerivationPath{{
      DerivationPathIndex{PURPOSE_BIP44, true},
      DerivationPathIndex{coin, true},
  }};
  auto node = get_node(*this, path);
  auto fingerprintValue = node_fingerprint(node);
  node = node.privateCkd(account + 0x80000000);
  node.fillPublicKey();
  return node_serialize(node, fingerprintValue, true);
}
PublicKey HDWallet::getPublicKeyFromExtended(const std::string& extended,
                                             const DerivationPath& path) {
  HDNode node = {};
  node_deserialize(extended, &node);
  node = node.publicCkd(path.change());
  node = node.publicCkd(path.address());
  node.fillPublicKey();
  return PublicKey{node.publicKey()};
}
PrivateKey HDWallet::getPrivateKeyFromExtended(const std::string& extended,
                                               const DerivationPath& path) {
  HDNode node = {};
  node_deserialize(extended, &node);
  node = node.privateCkd(path.change());
  node = node.privateCkd(path.address());
  return PrivateKey{node.privateKey()};
}
}  // namespace wallet
