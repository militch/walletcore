#ifndef WALLET_DERIVATION_PATH
#define WALLET_DERIVATION_PATH

#include <cstdint>
#include <string>
#include <vector>

namespace wallet {

enum class Purpose {
    BIP44 = 44,
};

struct DerivationPathIndex {
    uint32_t value;
    bool hardened;
    DerivationPathIndex() = default;
    DerivationPathIndex(uint32_t value, bool hardened = true)
        :value(value),hardened(hardened) {}
    
    uint32_t derivationIndex() const {
        if (hardened) {
            return value | 0x80000000;
        } else {
            return value;
        }
    }

    std::string string() const {
        if (hardened) {
            return std::to_string(value) + "'";
        } else {
            return std::to_string(value);
        }
    }
};

struct DerivationPath {
    std::vector<DerivationPathIndex> indices;
    Purpose purpose() const {
        if (indices.size() == 0) {
            return Purpose::BIP44;
        }
        return static_cast<Purpose>(indices[0].value);
    }

    void setPurpose(Purpose v) {
        if (indices.size() == 0) {
            return;
        }
        indices[0] = DerivationPathIndex(static_cast<uint32_t>(v),
         /* hardened: */ true);
    }

    uint32_t coin() const {
        if (indices.size() <= 1) {
            return 0;
        }
        return indices[1].value;
    }

    void setCoin(uint32_t v) {
        if (indices.size() <= 1) {
            return;
        }
        indices[1] = DerivationPathIndex(v, /* hardened: */ true);
    }

    uint32_t account() const {
        if (indices.size() <= 2) {
            return 0;
        }
        return indices[2].value;
    }

    void setAccount(uint32_t v) {
        if (indices.size() <= 2) {
            return;
        }
        indices[2] = DerivationPathIndex(v, /* hardened: */ true);
    }

    uint32_t change() const {
        if (indices.size() <= 3) {
            return 0;
        }
        return indices[3].value;
    }

    void setChange(uint32_t v) {
        if (indices.size() <= 3) {
            return;
        }
        indices[3] = DerivationPathIndex(v, /* hardened: */ false);
    }

    uint32_t address() const {
        if (indices.size() <= 4) {
            return 0;
        }
        return indices[4].value;
    }

    void setAddress(uint32_t v) {
        if (indices.size() <= 4) {
            return;
        }
        indices[4] = DerivationPathIndex(v, /* hardened: */ false);
    }

    DerivationPath() = default;
    explicit DerivationPath(std::initializer_list<DerivationPathIndex> l)
        : indices(l) {}
    explicit DerivationPath(std::vector<DerivationPathIndex> indices)
        : indices(std::move(indices)) {}

    /// Creates a `DerivationPath` by BIP44 components.
    DerivationPath(Purpose purpose, uint32_t coin, uint32_t account, uint32_t change,
                   uint32_t address)
        : indices(std::vector<DerivationPathIndex>(5)) {
        setPurpose(purpose);
        setCoin(coin);
        setAccount(account);
        setChange(change);
        setAddress(address);
    }

    /// Creates a derivation path with a string description like `m/10/0/2'/3`
    ///
    /// \throws std::invalid_argument if the string is not a valid derivation
    /// path.
    explicit DerivationPath(const std::string& string);

    /// String representation.
    std::string string() const noexcept;

};

}

#endif // WALLET_DERIVATION_PATH