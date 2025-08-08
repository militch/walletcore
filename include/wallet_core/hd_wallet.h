#ifndef WALLET_HD_WALLET_H
#define WALLET_HD_WALLET_H

#include <vector>
#include <string>
#include <array>
#include <optional>

#include "base.h"
#include "public_key.h"
#include "private_key.h"
#include "secp256k1.h"

namespace wallet {
struct DerivationPath;
class HDWallet {
    using SeedData = std::array<byte, 64>;
private:
    SeedData seed_;
public:
    HDWallet(const std::vector<byte> &seeds);
    HDWallet(const SeedData &seeds);
    HDWallet(const HDWallet& other) = default;
    HDWallet(HDWallet&& other) = default;
    HDWallet& operator=(const HDWallet& other) = default;
    HDWallet& operator=(HDWallet&& other) = default;
    ~HDWallet() = default;
    const SeedData& getSeed() const;
    PrivateKey getRootKey() const;
    PrivateKey getKey(const DerivationPath& path) const;
    std::string getExtendedPublicKeyAccount(uint32_t coin, uint32_t account) const;
    std::string getExtendedPrivateKeyAccount(uint32_t coin, uint32_t account) const;
    static PrivateKey getPrivateKeyFromExtended(const std::string& extended, const DerivationPath& path);
    static PublicKey getPublicKeyFromExtended(const std::string& extended, const DerivationPath& path);
};

} // namespace wallet

#endif // WALLET_HD_WALLET_H