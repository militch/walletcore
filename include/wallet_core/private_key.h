#ifndef WALLET_PRIVATE_KEY_H
#define WALLET_PRIVATE_KEY_H

#include "base.h"
#include <vector>
#include "public_key.h"
#include "secp256k1.h"

namespace wallet {
class PrivateKey {
using KeyData = std::array<byte,32>;
private:
    KeyData data_;
public:
    PrivateKey(const KeyData& data);
    PublicKey getPublicKey() const;
    const KeyData& data() const;
};
}

#endif // WALLET_PRIVATE_KEY_H