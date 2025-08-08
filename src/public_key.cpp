#include "wallet_core/public_key.h"
#include <stdexcept>
#include "curve.h"

using namespace wallet;


PublicKey::PublicKey(const KeyData& data)
    :data_(data) {

}

PublicKey::PublicKey(const std::vector<byte>& data) {
    std::copy_n(data.begin(), 33, data_.begin());
}

const PublicKey::KeyData& PublicKey::data() const{
    return data_;
}

std::vector<byte> PublicKey::uncompressed() const{
    secp256k1_pubkey pubkey;
    auto ctx = get_secp256k1_context();
    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, data_.data(), data_.size())) {
        throw std::runtime_error("");
    }
    std::vector<byte> uncompressed(65);
    size_t key_size = uncompressed.size();
    if (!secp256k1_ec_pubkey_serialize(
            ctx,
            uncompressed.data(),
            &key_size,
            &pubkey,
            SECP256K1_EC_UNCOMPRESSED)) {
        throw std::runtime_error("");
    }
    uncompressed.resize(key_size);
    return uncompressed;
}