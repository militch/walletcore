#include "wallet_core/private_key.h"

#include <stdexcept>
#include "curve.h"

using namespace wallet;

PrivateKey::PrivateKey(const PrivateKey::KeyData& data)
    :data_(data) {

}


PublicKey PrivateKey::getPublicKey() const {
    auto ctx = get_secp256k1_context();
     secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, data_.data())) {
        throw std::runtime_error("Failed to create public key");
    }
    std::vector<byte> pub_data(65);
    size_t out_len = pub_data.size();
    if (!secp256k1_ec_pubkey_serialize(ctx, pub_data.data(), &out_len, &pub, SECP256K1_EC_COMPRESSED)) {
        throw std::runtime_error("Failed to serialize public key");
    }
    pub_data.reserve(out_len);
    return PublicKey {pub_data};
}

const PrivateKey::KeyData& PrivateKey::data() const{
    return data_;
}

