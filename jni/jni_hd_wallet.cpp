#include "jni_hd_wallet.h"

#include "wallet_core/hd_wallet.h"
#include "wallet_core/derivation_path.h"
#include <array>
#include <algorithm>
#include <iostream>
#include <span>
#include "wallet_core/tron.h"


static jstring toJavaString(JNIEnv* env, const std::string &str){
    return env->NewStringUTF(str.c_str());
}

static std::string jstringToStdString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* chars = env->GetStringUTFChars(jStr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jStr, chars);
    return result;
}

static std::vector<byte> javaByteArrayToStdVec(JNIEnv* env, jbyteArray data) {
    jsize data_len = env->GetArrayLength(data);
    std::vector<byte> vec(data_len);
    env->GetByteArrayRegion(data, 0, data_len, reinterpret_cast<jbyte*>(vec.data()));
    return vec;
}


static jbyteArray toJavaBytes(JNIEnv *env, const std::span<byte>& span) {
    auto span_size = span.size();
    jbyteArray result = env->NewByteArray(static_cast<jsize>(span_size));
    if (!result) {
        // 内存分配失败
    }
    env->SetByteArrayRegion(
        result, 0, static_cast<jsize>(span.size()),
        reinterpret_cast<const jbyte*>(span.data()));
    return result;
}


JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPrivateKeyFromSeedWithPath(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path) {
    auto seed_data = javaByteArrayToStdVec(env, seed);
    // 通过随机种子构造分层钱包实例
    wallet::HDWallet hd_wallet{seed_data};
    auto path_str = jstringToStdString(env, path);
    // 构造派生路径类型
    auto d_path = wallet::DerivationPath{ path_str };
    // 获取私钥
    auto private_key = hd_wallet.getKey(d_path);
    auto private_key_data = private_key.data();
    return toJavaBytes(env, private_key_data);
}

JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPublicKeyFromSeedWithPath(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path) {
    auto seed_data = javaByteArrayToStdVec(env, seed);
    wallet::HDWallet hd_wallet{seed_data};
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto private_key = hd_wallet.getKey(d_path);
    auto public_key = private_key.getPublicKey();
    auto public_key_data = public_key.data();
    return toJavaBytes(env, public_key_data);
}


JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jstring path) {
    auto seed_data = javaByteArrayToStdVec(env, seed);
    wallet::HDWallet hd_wallet{seed_data};
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto private_key = hd_wallet.getKey(d_path);
    auto public_key = private_key.getPublicKey();
    auto addr = wallet::tron::TronAddress::derive_from_public_key(public_key);
    return toJavaString(env, addr.string());
}

JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getPrivateExtendedFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jint coin, jint account) {
    auto seed_data = javaByteArrayToStdVec(env, seed);
    wallet::HDWallet hd_wallet{seed_data};
    std::string extended = hd_wallet.getExtendedPrivateKeyAccount(coin, account);
    return toJavaString(env, extended);
}

JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getPublicExtendedFromSeed(JNIEnv *env, jclass clazz, jbyteArray seed, jint coin, jint account) {
    auto seed_data = javaByteArrayToStdVec(env, seed);
    wallet::HDWallet hd_wallet{seed_data};
    std::string extended = hd_wallet.getExtendedPublicKeyAccount(coin, account);
    return toJavaString(env, extended);
}

JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPublicKeyFromExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path) {
    auto extended_str = jstringToStdString(env, extended);
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto public_key = wallet::HDWallet::getPublicKeyFromExtended(extended_str, d_path);
    auto public_key_data = public_key.data();
    return toJavaBytes(env, public_key_data);
}

JNIEXPORT jbyteArray Java_com_github_militch_walletj_HDWallet_getPrivateKeyFromExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path) {
    auto extended_str = jstringToStdString(env, extended);
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto private_key = wallet::HDWallet::getPrivateKeyFromExtended(extended_str, d_path);
    auto private_key_data = private_key.data();
    return toJavaBytes(env, private_key_data);
}

JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromPrvExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path) {
    auto extended_str = jstringToStdString(env, extended);
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto private_key = wallet::HDWallet::getPrivateKeyFromExtended(extended_str, d_path);
    auto addr = wallet::tron::TronAddress::derive_from_public_key(private_key.getPublicKey());
    return toJavaString(env, addr.string());
}

JNIEXPORT jstring Java_com_github_militch_walletj_HDWallet_getTronAddressFromPubExtended0(JNIEnv *env, jclass clazz, jstring extended, jstring path) {
    auto extended_str = jstringToStdString(env, extended);
    auto path_str = jstringToStdString(env, path);
    auto d_path = wallet::DerivationPath{ path_str };
    auto public_key = wallet::HDWallet::getPublicKeyFromExtended(extended_str, d_path);
    auto addr = wallet::tron::TronAddress::derive_from_public_key(public_key);
    return toJavaString(env, addr.string());
}
