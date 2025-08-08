#ifndef WALLET_BIP32_H
#define WALLET_BIP32_H

#include <vector>
#include <array>
#include <optional>
#include "wallet_core/base.h"

namespace wallet {

struct HDNode {
    static const size_t PRIVATE_KEY_LEN = 32;
    static const size_t PUBLIC_KEY_LEN = 33;
    static const size_t CHAIN_CODE_LEN = 32;
    using ChainCode = std::array<byte, CHAIN_CODE_LEN>;
    using PrivateKey = std::array<byte, PRIVATE_KEY_LEN>;
    using PublicKey = std::array<byte, PUBLIC_KEY_LEN>;
    byte private_key_data[PRIVATE_KEY_LEN];
    byte public_key_data[PUBLIC_KEY_LEN];
    ChainCode chain_code;
    uint32_t depth;
    uint32_t child_num;
    static HDNode fromSeed(const std::array<byte, 64>& seed);
    void fillPublicKey();
    PrivateKey privateKey() const;
    PublicKey publicKey() const;
    HDNode privateCkd(uint32_t child);
    HDNode publicCkd(uint32_t child);
};

}


#endif // WALLET_BIP32_H