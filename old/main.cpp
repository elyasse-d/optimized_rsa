#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include "../fonction.h"
using namespace std;

struct  rsa_st
{
    mpz_class n; 
    mpz_class e; 
    mpz_class d; 
    mpz_class r;
    mpz_class t;
    mpz_class pp;
    mpz_class m;

};

mpz_class multpGem_optimized(const mpz_class& m, const mpz_class& e, const mpz_class& n) {
    mpz_t r0, r1, temp;
    mpz_inits(r0, r1, temp, NULL);
    
    mpz_set_ui(r0, 1);
    mpz_set(r1, m.get_mpz_t());

    
    for (int i = mpz_sizeinbase(e.get_mpz_t(), 2) - 1; i >= 0; i--) {
        if (mpz_tstbit(e.get_mpz_t(), i)) {
            mpz_mul(r0, r0, r1);
            mpz_mod(r0, r0, n.get_mpz_t());
            mpz_mul(r1, r1, r1);
            mpz_mod(r1, r1, n.get_mpz_t());
        } else {
            mpz_mul(r1, r0, r1);
            mpz_mod(r1, r1, n.get_mpz_t());

            mpz_mul(r0, r0, r0);
            mpz_mod(r0, r0, n.get_mpz_t());
        }
    }
    mpz_class result(r0);
    mpz_clears(r0, r1, temp, NULL);
    return result;
}
mpz_class SquareAndMultiply(mpz_class base,mpz_class exp,mpz_class mod) {
    mpz_t result;
    mpz_inits(result, NULL);
    mpz_set_ui(result, 1);
    mpz_mod(base.get_mpz_t(), base.get_mpz_t(), mod.get_mpz_t());
    size_t bit_length = mpz_sizeinbase(exp.get_mpz_t(), 2);
    for (long i = bit_length - 1; i >= 0; i--) {
        mpz_mul(result, result, result);
        mpz_mod(result, result, mod.get_mpz_t());

        if (mpz_tstbit(exp.get_mpz_t(), i)) {
            mpz_mul(result, result, base.get_mpz_t());
            mpz_mod(result, result, mod.get_mpz_t());
        }
    }
    mpz_class result_class(result);
    mpz_clears(result, NULL);
    return result_class;
    
}


int main(){


    mpz_class base("123456715125125151289012345678890");
    mpz_class exp("65537");
    mpz_class mod(
        "bce6748570af7574618ec452b443a561c5c784b62b081dc979f7c61f4fc3b1527f11b1f02d97f137e419440bb7918882a3b0ca3988c82d6983e8e555ab44683954ce83cd189f6206ab8ad37cb6a3fa2e3a75f205f42849585d86fc033b4b0bdb01137e0939dd1bc022134900737bd27582d0ac279d03ce5fbcde6cf01f4be279",
        16);
    mpz_class d(
        "4eed5fac4dbc1230717ecc8adde511d9fb6075140480dca94d3bf8dd265fd6dc685985669c364b44961af4728cddd312fac0288ec797145a6d12479876fa1b2d796de5bfcad3b501799811aaf5a99bf68d864f0a02f8524e6f8de7c7117fb24ef48d967dc2946af2efcb2c1680d95176e242eb66da10ef9ae0eea7bf3b948181",
        16);

    base %= mod;

    bool n_valid = true;
    bool e_valid = true;

    if (mod <= 1) {
        cout << "[check n] invalid: n must be > 1" << endl;
        n_valid = false;
    }
    if (mpz_even_p(mod.get_mpz_t())) {
        cout << "[check n] invalid: n must be odd for RSA/Montgomery" << endl;
        n_valid = false;
    }
    if (n_valid) {
        cout << "[check n] ok" << endl;
    }

    if (exp <= 1) {
        cout << "[check e] invalid: e must be > 1" << endl;
        e_valid = false;
    }
    if (mpz_even_p(exp.get_mpz_t())) {
        cout << "[check e] invalid: e should be odd" << endl;
        e_valid = false;
    }
    if (exp >= mod) {
        cout << "[check e] invalid: e should be < n" << endl;
        e_valid = false;
    }

    mpz_class gcd_en;
    mpz_gcd(gcd_en.get_mpz_t(), exp.get_mpz_t(), mod.get_mpz_t());
    if (gcd_en != 1) {
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

    mpz_class ciphertext = rsa_psa::rsa_encrypt_psa_montgomery(base, exp, mod, 8, 42);
    mpz_class decrypted;
    mpz_powm(decrypted.get_mpz_t(), ciphertext.get_mpz_t(), d.get_mpz_t(), mod.get_mpz_t());
    cout << "[rsa round-trip] " << (decrypted == base ? "ok" : "failed") << endl;

    mpz_class expected;
    mpz_powm(expected.get_mpz_t(), base.get_mpz_t(), exp.get_mpz_t(), mod.get_mpz_t());

    auto benchmark = [&](auto func, const string& name) {
        const int warmup = 2;
        const int runs = 20;
        vector<long long> samples;
        samples.reserve(runs);
        mpz_class last_result;

        for (int i = 0; i < warmup; ++i) {
            last_result = func();
        }

        for (int i = 0; i < runs; ++i) {
            auto start = chrono::steady_clock::now();
            last_result = func();
            auto end = chrono::steady_clock::now();
            samples.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }

        sort(samples.begin(), samples.end());
        long long median = samples[samples.size() / 2];
        long long sum = accumulate(samples.begin(), samples.end(), 0LL);
        double average = static_cast<double>(sum) / samples.size();
        bool ok = (last_result == expected);

        cout << name << " avg: " << average << " us, median: " << median
             << " us, correct: " << (ok ? "yes" : "no") << endl;
    };

    benchmark([&]() { return rsa_psa::rsa_encrypt_psa_montgomery(base, exp, mod, 8, 42); }, "PSA Montgomery");
    benchmark([&]() { return multpGem_optimized(base, exp, mod); }, "multpGem");
    benchmark([&]() { return SquareAndMultiply(base, exp, mod); }, "SquareAndMultiply");

    return 0;
}