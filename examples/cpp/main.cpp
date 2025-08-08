#include <openssl/rand.h>

#include <iostream>
#include <span>

#include "secp256k1.h"
#include "wallet_core/walletcore.h"

namespace {

using ByteAsHex = std::array<char, 2>;

constexpr std::array<ByteAsHex, 256> CreateByteToHexMap() {
  constexpr char hexmap[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::array<ByteAsHex, 256> byte_to_hex{};
  for (size_t i = 0; i < byte_to_hex.size(); ++i) {
    byte_to_hex[i][0] = hexmap[i >> 4];
    byte_to_hex[i][1] = hexmap[i & 15];
  }
  return byte_to_hex;
}

}  // namespace

std::string HexStr(const std::span<const uint8_t> s) {
  std::string rv(s.size() * 2, '\0');
  static constexpr auto byte_to_hex = CreateByteToHexMap();
  static_assert(sizeof(byte_to_hex) == 512);

  char* it = rv.data();
  for (uint8_t v : s) {
    std::memcpy(it, byte_to_hex[v].data(), 2);
    it += 2;
  }

  return rv;
}

const signed char p_util_hexdigit[256] = {
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  1,   2,   3,   4,   5,   6,   7,  8,  9,  -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

int main() {
  std::array<uint8_t, 64> seed;
  if (RAND_bytes(seed.data(), seed.size()) != 1) {
    std::cout << "Failed get rand bytes" << std::endl;
    exit(1);
  }
  auto ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  std::string seed_hex = HexStr(seed);
  std::cout << "random seed: " << seed_hex << std::endl;
  wallet::HDWallet hd_wallet{seed};
  wallet::DerivationPath fullpath{"m/44'/195'/0'/0/0"};
  auto priv_key = hd_wallet.getKey(fullpath);
  auto priv_key_data = priv_key.data();
  std::string priv_key_hex = HexStr(priv_key_data);
  std::cout << "priv key: " << priv_key_hex << std::endl;
  auto pub_key = priv_key.getPublicKey();
  std::cout << "pub key: " << HexStr(pub_key.data()) << std::endl;
  auto pub_key_un = pub_key.uncompressed();
  std::cout << "unpub key: " << HexStr(pub_key_un) << std::endl;
  auto addr = wallet::tron::TronAddress::derive_from_public_key(pub_key);
  std::cout << "addr: " << addr.string() << std::endl;
  std::string apub = hd_wallet.getExtendedPublicKeyAccount(195, 0);
  std::cout << "apub: " << apub << std::endl;
  auto puba = hd_wallet.getPublicKeyFromExtended(apub,fullpath);
  std::cout << "pub a: " << HexStr(puba.data()) << std::endl;
  auto addra = wallet::tron::TronAddress::derive_from_public_key(puba);
  std::cout << "addra: " << addra.string() << std::endl;
  std::string apriv = hd_wallet.getExtendedPrivateKeyAccount(195, 0);
  std::cout << "apriv: " << apriv << std::endl;
  auto priva = hd_wallet.getPrivateKeyFromExtended(apriv, fullpath);
  auto pubaa = priva.getPublicKey();
  std::cout << "pubaa: " << HexStr(pubaa.data()) << std::endl;
  auto addraa = wallet::tron::TronAddress::derive_from_public_key(pubaa);
  std::cout << "addraa: " << addraa.string() << std::endl;

  secp256k1_context_destroy(ctx);
  return 0;
}