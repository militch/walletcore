package com.github.militch.walletj;

/**
 * 数字货币类型
 */
public enum CoinType {
    /**
     * Tron(波场)
     */
    TRON(195);
    /**
     * 枚举数值
     */
    private final int id;

    CoinType(int id){
        this.id = id;
    }

    int getId(){
        return id;
    }
}
