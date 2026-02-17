#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <openssl/bn.h>

using namespace std;

using BNPtr = unique_ptr<BIGNUM, decltype(&BN_free)>;

BNPtr make_bn() {
    BIGNUM* raw = BN_new();
    if (raw == nullptr) {
        throw runtime_error("BN_new failed");
    }
    return BNPtr(raw, BN_free);
}

BNPtr dup_bn(const BIGNUM* source) {
    BIGNUM* raw = BN_dup(source);
    if (raw == nullptr) {
        throw runtime_error("BN_dup failed");
    }
    return BNPtr(raw, BN_free);
}

BNPtr bn_from_dec(const string& dec) {
    BIGNUM* raw = nullptr;
    if (BN_dec2bn(&raw, dec.c_str()) == 0 || raw == nullptr) {
        throw runtime_error("BN_dec2bn failed");
    }
    return BNPtr(raw, BN_free);
}

BNPtr bn_from_hex(const string& hex) {
    BIGNUM* raw = nullptr;
    if (BN_hex2bn(&raw, hex.c_str()) == 0 || raw == nullptr) {
        throw runtime_error("BN_hex2bn failed");
    }
    return BNPtr(raw, BN_free);
}

int bit_length(const BIGNUM* bn) {
    return BN_num_bits(bn);
}

bool bit_is_set(const BIGNUM* bn, int bit) {
    return BN_is_bit_set(bn, bit) != 0;
}

BNPtr mod_mul(const BIGNUM* a, const BIGNUM* b, const BIGNUM* n, BN_CTX* ctx) {
    auto out = make_bn();
    if (BN_mod_mul(out.get(), a, b, n, ctx) == 0) {
        throw runtime_error("BN_mod_mul failed");
    }
    return out;
}

BNPtr mod_pow(const BIGNUM* base, const BIGNUM* exp, const BIGNUM* n, BN_CTX* ctx) {
    auto out = make_bn();
    if (BN_mod_exp(out.get(), base, exp, n, ctx) == 0) {
        throw runtime_error("BN_mod_exp failed");
    }
    return out;
}

BNPtr square_and_multiply(const BIGNUM* base, const BIGNUM* exp, const BIGNUM* n, BN_CTX* ctx) {
    auto result = make_bn();
    if (BN_one(result.get()) == 0) {
        throw runtime_error("BN_one failed");
    }

    auto base_mod = make_bn();
    if (BN_nnmod(base_mod.get(), base, n, ctx) == 0) {
        throw runtime_error("BN_nnmod failed");
    }

    for (int i = bit_length(exp) - 1; i >= 0; --i) {
        auto sq = mod_mul(result.get(), result.get(), n, ctx);
        result = move(sq);
        if (bit_is_set(exp, i)) {
            auto mul = mod_mul(result.get(), base_mod.get(), n, ctx);
            result = move(mul);
        }
    }

    return result;
}

BNPtr multp_bigint(const BIGNUM* message, const BIGNUM* exp, const BIGNUM* n, BN_CTX* ctx) {
    auto r0 = make_bn();
    auto r1 = make_bn();
    if (BN_one(r0.get()) == 0) {
        throw runtime_error("BN_one failed");
    }
    if (BN_nnmod(r1.get(), message, n, ctx) == 0) {
        throw runtime_error("BN_nnmod failed");
    }

    for (int i = bit_length(exp) - 1; i >= 0; --i) {
        if (bit_is_set(exp, i)) {
            auto new_r0 = mod_mul(r0.get(), r1.get(), n, ctx);
            auto new_r1 = mod_mul(r1.get(), r1.get(), n, ctx);
            r0 = move(new_r0);
            r1 = move(new_r1);
        } else {
            auto new_r1 = mod_mul(r0.get(), r1.get(), n, ctx);
            auto new_r0 = mod_mul(r0.get(), r0.get(), n, ctx);
            r0 = move(new_r0);
            r1 = move(new_r1);
        }
    }

    return r0;
}

BNPtr rsa_encrypt_psa_montgomery_bigint(
    const BIGNUM* message,
    const BIGNUM* exp,
    const BIGNUM* n,
    size_t split_size,
    uint64_t seed,
    BN_CTX* ctx) {

    if ((BN_is_odd(n)) == 0) {
        throw invalid_argument("n must be odd for Montgomery");
    }

    BN_MONT_CTX* mont = BN_MONT_CTX_new();
    if (mont == nullptr) {
        throw runtime_error("BN_MONT_CTX_new failed");
    }
    if (BN_MONT_CTX_set(mont, n, ctx) == 0) {
        BN_MONT_CTX_free(mont);
        throw runtime_error("BN_MONT_CTX_set failed");
    }

    auto one = make_bn();
    if (BN_one(one.get()) == 0) {
        BN_MONT_CTX_free(mont);
        throw runtime_error("BN_one failed");
    }

    auto m_mont = make_bn();
    auto one_mont = make_bn();
    if (BN_to_montgomery(m_mont.get(), message, mont, ctx) == 0 ||
        BN_to_montgomery(one_mont.get(), one.get(), mont, ctx) == 0) {
        BN_MONT_CTX_free(mont);
        throw runtime_error("BN_to_montgomery failed");
    }

    vector<int> bits;
    bits.reserve(static_cast<size_t>(bit_length(exp)));
    for (int i = 0; i < bit_length(exp); ++i) {
        if (bit_is_set(exp, i)) {
            bits.push_back(i);
        }
    }

    mt19937_64 rng(seed);
    shuffle(bits.begin(), bits.end(), rng);

    int max_bit = 0;
    for (int b : bits) {
        max_bit = max(max_bit, b);
    }

    vector<BNPtr> powers;
    powers.reserve(static_cast<size_t>(max_bit + 1));
    for (int i = 0; i <= max_bit; ++i) {
        powers.push_back(make_bn());
        if (BN_copy(powers.back().get(), one_mont.get()) == nullptr) {
            BN_MONT_CTX_free(mont);
            throw runtime_error("BN_copy failed");
        }
    }

    if (!powers.empty()) {
        if (BN_copy(powers[0].get(), m_mont.get()) == nullptr) {
            BN_MONT_CTX_free(mont);
            throw runtime_error("BN_copy failed");
        }
        for (int i = 1; i <= max_bit; ++i) {
            if (BN_mod_mul_montgomery(powers[i].get(), powers[i - 1].get(), powers[i - 1].get(), mont, ctx) == 0) {
                BN_MONT_CTX_free(mont);
                throw runtime_error("BN_mod_mul_montgomery failed");
            }
        }
    }

    auto acc = make_bn();
    if (BN_copy(acc.get(), one_mont.get()) == nullptr) {
        BN_MONT_CTX_free(mont);
        throw runtime_error("BN_copy failed");
    }

    for (size_t i = 0; i < bits.size(); i += split_size) {
        size_t end = min(i + split_size, bits.size());
        auto local = make_bn();
        if (BN_copy(local.get(), one_mont.get()) == nullptr) {
            BN_MONT_CTX_free(mont);
            throw runtime_error("BN_copy failed");
        }

        for (size_t j = i; j < end; ++j) {
            int idx = bits[j];
            if (BN_mod_mul_montgomery(local.get(), local.get(), powers[idx].get(), mont, ctx) == 0) {
                BN_MONT_CTX_free(mont);
                throw runtime_error("BN_mod_mul_montgomery failed");
            }
        }

        if (BN_mod_mul_montgomery(acc.get(), acc.get(), local.get(), mont, ctx) == 0) {
            BN_MONT_CTX_free(mont);
            throw runtime_error("BN_mod_mul_montgomery failed");
        }
    }

    auto out = make_bn();
    if (BN_from_montgomery(out.get(), acc.get(), mont, ctx) == 0) {
        BN_MONT_CTX_free(mont);
        throw runtime_error("BN_from_montgomery failed");
    }

    BN_MONT_CTX_free(mont);
    return out;
}

int main() {
    BN_CTX* raw_ctx = BN_CTX_new();
    if (raw_ctx == nullptr) {
        cerr << "BN_CTX_new failed" << endl;
        return 1;
    }
    unique_ptr<BN_CTX, decltype(&BN_CTX_free)> ctx(raw_ctx, BN_CTX_free);

    auto base = bn_from_dec("123456715125125151289012345678890");
    auto e = bn_from_dec("65537");
    auto n = bn_from_hex(
        "bce6748570af7574618ec452b443a561c5c784b62b081dc979f7c61f4fc3b1527f11b1f02d97f137e419440bb7918882a3b0ca3988c82d6983e8e555ab44683954ce83cd189f6206ab8ad37cb6a3fa2e3a75f205f42849585d86fc033b4b0bdb01137e0939dd1bc022134900737bd27582d0ac279d03ce5fbcde6cf01f4be279");
    auto d = bn_from_hex(
        "4eed5fac4dbc1230717ecc8adde511d9fb6075140480dca94d3bf8dd265fd6dc685985669c364b44961af4728cddd312fac0288ec797145a6d12479876fa1b2d796de5bfcad3b501799811aaf5a99bf68d864f0a02f8524e6f8de7c7117fb24ef48d967dc2946af2efcb2c1680d95176e242eb66da10ef9ae0eea7bf3b948181");

    if (BN_nnmod(base.get(), base.get(), n.get(), ctx.get()) == 0) {
        cerr << "BN_nnmod failed" << endl;
        return 1;
    }

    bool n_valid = true;
    bool e_valid = true;

    if (BN_cmp(n.get(), BN_value_one()) <= 0) {
        cout << "[check n] invalid: n must be > 1" << endl;
        n_valid = false;
    }
    if (BN_is_odd(n.get()) == 0) {
        cout << "[check n] invalid: n must be odd for RSA/Montgomery" << endl;
        n_valid = false;
    }
    if (n_valid) {
        cout << "[check n] ok" << endl;
    }

    if (BN_cmp(e.get(), BN_value_one()) <= 0) {
        cout << "[check e] invalid: e must be > 1" << endl;
        e_valid = false;
    }
    if (BN_is_odd(e.get()) == 0) {
        cout << "[check e] invalid: e should be odd" << endl;
        e_valid = false;
    }
    if (BN_cmp(e.get(), n.get()) >= 0) {
        cout << "[check e] invalid: e should be < n" << endl;
        e_valid = false;
    }

    auto gcd = make_bn();
    if (BN_gcd(gcd.get(), e.get(), n.get(), ctx.get()) == 0) {
        cerr << "BN_gcd failed" << endl;
        return 1;
    }
    if (BN_cmp(gcd.get(), BN_value_one()) != 0) {
        cout << "[check e] warning: gcd(e, n) != 1" << endl;
        e_valid = false;
    }
    if (e_valid) {
        cout << "[check e] ok" << endl;
    }

    if (!n_valid || !e_valid) {
        cout << "Invalid RSA public parameters. Benchmark aborted." << endl;
        return 1;
    }

    auto ciphertext = rsa_encrypt_psa_montgomery_bigint(base.get(), e.get(), n.get(), 8, 42, ctx.get());
    auto decrypted = mod_pow(ciphertext.get(), d.get(), n.get(), ctx.get());
    cout << "[rsa round-trip] " << (BN_cmp(decrypted.get(), base.get()) == 0 ? "ok" : "failed") << endl;

    auto expected = mod_pow(base.get(), e.get(), n.get(), ctx.get());

    auto benchmark = [&](auto func, const string& name) {
        const int warmup = 2;
        const int runs = 20;
        vector<long long> samples;
        samples.reserve(runs);
        BNPtr last = make_bn();

        for (int i = 0; i < warmup; ++i) {
            last = func();
        }
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::steady_clock::now();
            last = func();
            auto end = chrono::steady_clock::now();
            samples.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }

        sort(samples.begin(), samples.end());
        long long median = samples[samples.size() / 2];
        long long sum = accumulate(samples.begin(), samples.end(), 0LL);
        double average = static_cast<double>(sum) / samples.size();

        cout << name << " avg: " << average << " us, median: " << median
             << " us, correct: " << (BN_cmp(last.get(), expected.get()) == 0 ? "yes" : "no") << endl;
    };

    benchmark([&]() { return rsa_encrypt_psa_montgomery_bigint(base.get(), e.get(), n.get(), 8, 42, ctx.get()); }, "PSA Montgomery (BIGINT)");
    benchmark([&]() { return multp_bigint(base.get(), e.get(), n.get(), ctx.get()); }, "multpGem (BIGINT)");
    benchmark([&]() { return square_and_multiply(base.get(), e.get(), n.get(), ctx.get()); }, "SquareAndMultiply (BIGINT)");

    return 0;
}
