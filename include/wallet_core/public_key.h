#ifndef WALLET_PUBLIC_KEY_H
#define WALLET_PUBLIC_KEY_H

#include <array>
#include <vector>

#include "secp256k1.h"

#include "base.h"

namespace wallet {

class PublicKey {
using KeyData = std::array<byte, 33>;
private:
    KeyData data_;
public:
    PublicKey(const KeyData& data);
    PublicKey(const std::vector<byte>& data);
    const KeyData& data() const;
    std::vector<byte> uncompressed() const;
};

}

#endif // WALELT_PUBLIC_KEY_H