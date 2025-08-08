#include "bip32.h"
#include <stdexcept>

#include "crypto/common.h"
#include "crypto/hmac_sha512.h"
#include "curve.h"
#include "secp256k1.h"

using namespace wallet;

static const unsigned char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};

HDNode HDNode::fromSeed(const std::array<byte, 64>& seed) {
    byte hash[CSHA512::OUTPUT_SIZE];
    CHMAC_SHA512 {hashkey, sizeof(hashkey)}.Write(seed.data(), seed.size()).Finalize(hash);
    HDNode node;
    node.child_num = 0;
    node.depth = 0;
    std::copy(hash, hash + 32, node.private_key_data);
    std::copy(hash + 32, hash + 64, node.chain_code.begin());
    std::memset(node.public_key_data, 0, sizeof(node.public_key_data));
    return node;
}

void HDNode::fillPublicKey() {
    if (public_key_data[0] != 0) { 
        return; 
    }
    secp256k1_pubkey pub;
    auto ctx = get_secp256k1_context();
    if (!secp256k1_ec_pubkey_create(ctx, &pub, private_key_data)) {
        throw std::runtime_error("Failed to create public key");
    }
    size_t out_len = PUBLIC_KEY_LEN;
    if (!secp256k1_ec_pubkey_serialize(ctx, public_key_data, &out_len, &pub, SECP256K1_EC_COMPRESSED)) {
        throw std::runtime_error("Failed to serialize public key");
    }
}


HDNode::PrivateKey HDNode::privateKey() const {
    std::array<byte, 32> result;
    std::copy(std::begin(private_key_data), std::end(private_key_data), result.begin());
    return result;
}

HDNode::PublicKey HDNode::publicKey() const {
    std::array<byte, 33> result;
    std::copy(std::begin(public_key_data), std::end(public_key_data), result.begin());
    return result;
}

HDNode HDNode::privateCkd(uint32_t index) {
    auto ctx = get_secp256k1_context();
    std::array<uint8_t, 37> data;
    if (index & 0x80000000) {
        // Hardened: data = 0x00 || parent_privkey || i
        data[0] = 0;
        std::copy(std::begin(private_key_data), std::end(private_key_data), data.begin() + 1);
    } else {
        // Normal: data = parent_pubkey || i
        fillPublicKey();
        std::copy(std::begin(public_key_data), std::end(public_key_data), data.begin());
    }
    WriteBE32(data.data() + 33, index);
    byte hash[64];
    CHMAC_SHA512 hmac(chain_code.data(), chain_code.size());
    hmac.Write(data.data(), data.size()).Finalize(hash);
    std::array<byte, 32> il;
    std::array<byte, 32> ir;
    std::copy(hash, hash + 32, il.begin());
    std::copy(hash + 32, hash + 64, ir.begin());
    auto child_key = privateKey();
    if (!secp256k1_ec_seckey_verify(ctx, il.data())) {
        throw std::runtime_error("Invalid derived key (il)");
    }
    if (!secp256k1_ec_seckey_tweak_add(ctx, child_key.data(), il.data())) {
        throw std::runtime_error("secp256k1 tweak_add failed");
    }
    HDNode out;
    std::copy(child_key.begin(),child_key.end(), out.private_key_data);
    out.chain_code = std::move(ir);
    out.depth = depth + 1;
    out.child_num = index;
    std::memset(out.public_key_data, 0, sizeof(out.public_key_data));
    return out;
}


HDNode HDNode::publicCkd(uint32_t index) {
    if (index & 0x80000000) {
        throw std::runtime_error("Public derivation does not support hardened indexes");
    }
    std::array<uint8_t, 37> data;
    std::copy(std::begin(public_key_data), std::end(public_key_data), data.begin());
    WriteBE32(data.data() + 33, index);

    byte hash[64];
    CHMAC_SHA512 hmac(chain_code.data(), chain_code.size());
    hmac.Write(data.data(), data.size()).Finalize(hash);

    std::array<byte, 32> il, ir;
    std::copy(hash, hash + 32, il.begin());
    std::copy(hash + 32, hash + 64, ir.begin());
    auto ctx = get_secp256k1_context();
    if (!secp256k1_ec_seckey_verify(ctx, il.data())) {
        throw std::runtime_error("Invalid IL value");
    }

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, public_key_data, sizeof(public_key_data))) {
        throw std::runtime_error("Failed to parse public key");
    }

    if (!secp256k1_ec_pubkey_tweak_add(ctx, &pubkey, il.data())) {
        throw std::runtime_error("Public key tweak failed");
    }

    size_t out_len = 33;
    std::array<uint8_t, 33> out_pubkey;
    if (!secp256k1_ec_pubkey_serialize(ctx, out_pubkey.data(), &out_len, &pubkey, SECP256K1_EC_COMPRESSED)) {
        throw std::runtime_error("Failed to serialize tweaked public key");
    }

    HDNode out;
    std::copy(out_pubkey.begin(), out_pubkey.end(), out.public_key_data);
    out.chain_code = std::move(ir);
    out.depth = depth + 1;
    out.child_num = index;
    std::memset(out.private_key_data, 0, sizeof(out.private_key_data));
    return out;
}

