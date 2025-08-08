#include "curve.h"
#include "secp256k1.h"

secp256k1_context *get_secp256k1_context() {
    static secp256k1_context* ctx = []() -> secp256k1_context* {
        auto* c = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
        return c;
    }();
    return ctx;
}