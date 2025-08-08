package com.github.militch.walletj;

import java.security.SecureRandom;

/**
 * 这个类提供了分层确定性钱包相关操作方法的实现
 * <p>
 * 关于分层确定性钱包的定义参阅 <a href="https://en.bitcoin.it/wiki/BIP_0032">BIP 0032 - Bitcoin Wiki</a>
 * <p/>
 * 使用示例:
 * <pre>
 * HDWallet hdWallet = new HDWallet();
 * String address = hdWallet.getTronAddress("m/44'/195'/0'/0/0");
 * <pre>
 */
public final class HDWallet {
    static {
        loadLibrary();
    }
    private final static int SEED_LEN = 64;
    private static final SecureRandom secureRandom = new SecureRandom();
    private final byte[] seed;

    private static void loadLibrary() {
        NativeLibraryLoader.load("walletcore", HDWallet.class.getClassLoader());
    }

    /**
     * 使用随机种子构造
     * @param seed 随机种子
     */
    public HDWallet(byte[] seed) {
        this.seed = seed;
    }

    /**
     * 默认构造
     */
    public HDWallet() {
        byte[] seed = new byte[SEED_LEN];
        secureRandom.nextBytes(seed);
        this.seed = seed;
    }

    /**
     * 返回指定派生路径的私钥
     *
     * @param path 派生路径
     * @return 私钥
     */
    public byte[] getPrivateKey(String path) {
        return getPrivateKeyFromSeedWithPath(seed, path);
    }

    /**
     * 返回指定派生路径的公钥
     *
     * @param path 派生路径
     * @return 公钥
     */
    public byte[] getPublicKey(String path) {
        return getPublicKeyFromSeedWithPath(seed, path);
    }

    /**
     * 返回指定派生路径的TRON地址
     *
     * @param path 派生路径
     * @return 地址
     */
    public String getTronAddress(String path) {
        return getTronAddressFromSeed(seed, path);
    }

    /**
     * 获取扩展私钥
     *
     * @param coin    币种
     * @param account 账户
     * @return 扩展私钥
     */
    public String getPrivateExtended(CoinType coin, int account) {
        return getPrivateExtendedFromSeed(seed, coin.getId(), account);
    }

    /**
     * 获取扩展公钥序列化
     *
     * @param coin    币种
     * @param account 账户
     * @return 扩展公钥
     */
    public String getPublicExtended(CoinType coin, int account) {
        return getPublicExtendedFromSeed(seed, coin.getId(), account);
    }

    /**
     * 获取从扩展公钥中派生的子公钥
     *
     * @param extended 扩展公钥
     * @param path     派生路径
     * @return 公钥
     */
    public static byte[] getPublicKeyFromExtended(String extended, String path) {
        return getPublicKeyFromExtended0(extended, path);
    }

    /**
     * 获取从扩展私钥中派生的子密钥
     *
     * @param extended 扩展私钥
     * @param path     派生路径
     * @return 扩展私钥
     */
    public static byte[] getPrivateKeyFromExtended(String extended, String path) {
        return getPrivateKeyFromExtended0(extended, path);
    }

    /**
     * 返回从扩展私钥中派生的TRON地址
     *
     * @param extended 扩展私钥
     * @param path     派生路径
     * @return TRON地址
     */
    public static String getTronAddressFromPrvExtended(String extended, String path) {
        return getTronAddressFromPrvExtended0(extended, path);
    }

    /**
     * 返回从扩展公钥中派生的TRON地址
     *
     * @param extended 扩展公钥
     * @param path     派生路径
     * @return TRON 地址
     */
    public static String getTronAddressFromPubExtended(String extended, String path) {
        return getTronAddressFromPubExtended0(extended, path);
    }

    private static native byte[] getPrivateKeyFromSeedWithPath(byte[] seed, String path);
    private static native byte[] getPublicKeyFromSeedWithPath(byte[] seed, String path);
    private static native String getTronAddressFromSeed(byte[] seed, String path);
    private static native String getPrivateExtendedFromSeed(byte[] seed, int coin, int account);
    private static native String getPublicExtendedFromSeed(byte[] seed, int coin, int account);
    private static native byte[] getPublicKeyFromExtended0(String extended, String path);
    private static native byte[] getPrivateKeyFromExtended0(String extended, String path);
    private static native String getTronAddressFromPrvExtended0(String extended, String path);
    private static native String getTronAddressFromPubExtended0(String extended, String path);
}
