#include "wallet_core/tron.h"
#include "wallet_core/public_key.h"
#include "keccak.h"
#include "crypto/hex_base.h"
#include "base58.h"

using namespace wallet::tron;

TronAddress::TronAddress(const TronAddress::Data& data) : data_(data) {

}

std::string TronAddress::string() {
    return EncodeBase58Check(data_);
}
std::string TronAddress::hex() {
    return HexStr(data_);
}

TronAddress TronAddress::derive_from_public_key(const PublicKey& key) {
    std::vector<byte> pub_key = key.uncompressed();
    std::array<byte, 32> hash;
    Keccak256(pub_key.data() +1, 64, hash.data());
    std::array<uint8_t, 21> addr_bytes{};
    addr_bytes[0] = 0x41;
    std::memcpy(addr_bytes.data() + 1, hash.data() + 12, 20);
    return TronAddress(addr_bytes);

}