#ifndef WALLET_CURVE_H
#define WALLET_CURVE_H

struct secp256k1_context_struct;

secp256k1_context_struct* get_secp256k1_context();

#endif // WALLET_CURVE_H