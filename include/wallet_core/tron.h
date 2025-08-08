#ifndef WALLET_TRON_H
#define WALLET_TRON_H

#include "base.h"
#include <array>
#include <string>
#include "secp256k1.h"

namespace wallet {
    class PublicKey;
}

namespace wallet::tron {

class TronAddress {
  using Data = std::array<byte, 21>;

 private:
  Data data_;

 public:
  TronAddress(const Data& data);
  std::string string();
  std::string hex();
  static TronAddress derive_from_public_key(const PublicKey& key);
};
}  // namespace wallet::tron

#endif  // WALLET_TRON_H