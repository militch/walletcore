#ifndef JNI_HD_WALLET_H
#define JNI_HD_WALLET_H

#include <jni.h>

#include "jni_base.h"

EXTERN_C_BEGIN

// 获取从随机种子派生的私钥
JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPrivateKeyFromSeedWithPath(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path);
// 获取从随机种子派生的公钥
JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPublicKeyFromSeedWithPath(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path);
// 返回从随机种子派生的TRON地址
JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path);
// 获取从随机种子派生的扩展公钥
JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getPrivateExtendedFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jint coin, jint account);
// 获取从随机种子派生的扩展私钥
JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getPublicExtendedFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jint coin, jint account);
// 返回公钥从序列化的扩展公钥中派生
JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPublicKeyFromExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path);
// 返回私钥从序列化的扩展私钥中派生
JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPrivateKeyFromExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path);
// 返回从扩展私钥中派生的TRON地址
JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromPrvExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path);
// 返回从扩展公钥中派生的TRON地址
JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromPubExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path);

EXTERN_C_END

#endif // JNI_HD_WALLET_H