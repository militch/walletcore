#ifndef KECCAK_H
#define KECCAK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // for size_t

// 类型定义
typedef unsigned char u8;
typedef unsigned long long int u64;
typedef unsigned int ui;

// 宏定义
#define FOR(i, n) for ((i) = 0; (i) < (n); ++(i))

// 核心通用 Keccak 函数
void Keccak(ui r, ui c, const u8 *in, u64 inLen, u8 sfx, u8 *out, u64 outLen);

// 标准 SHA3 系列
void FIPS202_SHA3_224(const u8 *in, u64 inLen, u8 *out);
void FIPS202_SHA3_256(const u8 *in, u64 inLen, u8 *out);
void FIPS202_SHA3_384(const u8 *in, u64 inLen, u8 *out);
void FIPS202_SHA3_512(const u8 *in, u64 inLen, u8 *out);

// 可变输出 SHAKE 系列
void FIPS202_SHAKE128(const u8 *in, u64 inLen, u8 *out, u64 outLen);
void FIPS202_SHAKE256(const u8 *in, u64 inLen, u8 *out, u64 outLen);

// Keccak-256（非标准 SHA3，推荐你自己封装使用）
static inline void Keccak256(const u8 *in, u64 inLen, u8 *out) {
    Keccak(1088, 512, in, inLen, 0x01, out, 32);
}

#ifdef __cplusplus
}
#endif

#endif // KECCAK_H