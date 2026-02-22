#ifndef METHODES_H
#define METHODES_H

#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <array>
#include <tuple>
#include <string>
#include <cstdint>
#include <stdexcept>

#include "fonction.h"  // Montgomery + PSA

using namespace std;

// ============================================================
// Opérations modulaires (from operations_modulaires)
// ============================================================

// Euclide étendu : retourne (pgcd, x, y) tels que A*x + B*y = pgcd(A, B)
inline tuple<mpz_class, mpz_class, mpz_class> extended_gcd(mpz_class A, mpz_class B) {
    mpz_class x0 = 1, y0 = 0;
    mpz_class x1 = 0, y1 = 1;

    while (B != 0) {
        mpz_class q = A / B;
        mpz_class r = A - q * B;

        A = B;
        B = r;

        mpz_class x_temp = x0 - q * x1;
        x0 = x1;
        x1 = x_temp;

        mpz_class y_temp = y0 - q * y1;
        y0 = y1;
        y1 = y_temp;
    }

    return {A, x0, y0};
}

// Inverse modulaire via Euclide étendu
inline mpz_class invmod(const mpz_class& A, const mpz_class& n) {
    mpz_class a = A % n;
    if (a < 0) a += n;
    auto [gcd, x, y] = extended_gcd(a, n);
    if (gcd != 1) {
        throw runtime_error("Inverse modulaire inexistant (gcd != 1)");
    }
    mpz_class result = x % n;
    if (result < 0) result += n;
    return result;
}

// ============================================================
// Modulo par doublement + soustraction
// ============================================================
inline void modulo_doublement(mpz_t r, const mpz_t a, const mpz_t n) {
    if (mpz_sgn(n) <= 0) { mpz_set_ui(r, 0); return; }
    if (mpz_cmp_ui(n, 1) == 0) { mpz_set_ui(r, 0); return; }

    mpz_set(r, a);

    if (mpz_sgn(r) < 0) { mpz_mod(r, r, n); return; }
    if (mpz_cmp(r, n) < 0) { return; }

    mpz_t m;
    mpz_init_set(m, n);

    const mp_bitcnt_t r_bits = mpz_sizeinbase(r, 2);
    const mp_bitcnt_t n_bits = mpz_sizeinbase(n, 2);

    if (r_bits > n_bits) { mpz_mul_2exp(m, m, r_bits - n_bits); }
    if (mpz_cmp(m, r) > 0) { mpz_tdiv_q_2exp(m, m, 1); }

    while (mpz_cmp(m, n) >= 0) {
        if (mpz_cmp(r, m) >= 0) { mpz_sub(r, r, m); }
        mpz_tdiv_q_2exp(m, m, 1);
    }

    mpz_clear(m);
}

// ============================================================
// Primalité : division d'essai + Miller-Rabin
// ============================================================

constexpr array<unsigned int, 22> kSmallPrimes = {
    7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u,
    47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u, 89u, 97u
};

inline bool miller_rabin_round(
    const mpz_class& n,
    const mpz_class& n_minus_one,
    const mpz_class& d,
    unsigned long s,
    const mpz_class& a) {
    mpz_class x;
    mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

    if (x == 1 || x == n_minus_one) return true;

    for (unsigned long r = 1; r < s; ++r) {
        mpz_mul(x.get_mpz_t(), x.get_mpz_t(), x.get_mpz_t());
        mpz_mod(x.get_mpz_t(), x.get_mpz_t(), n.get_mpz_t());
        if (x == n_minus_one) return true;
    }

    return false;
}

inline bool common_composite_checks(const mpz_class& n) {
    if (n < 2) return false;
    if (n == 2 || n == 3 || n == 5) return true;
    if (mpz_even_p(n.get_mpz_t()) != 0) return false;
    if (mpz_divisible_ui_p(n.get_mpz_t(), 3u) != 0 ||
        mpz_divisible_ui_p(n.get_mpz_t(), 5u) != 0) return false;

    for (const unsigned int p : kSmallPrimes) {
        if (n == p) return true;
        if (mpz_divisible_ui_p(n.get_mpz_t(), p) != 0) return false;
    }
    return true;
}

inline bool is_probable_prime_reduced(const mpz_class& n) {
    if (!common_composite_checks(n)) return false;

    mpz_class n_minus_one = n - 1;
    const unsigned long s = mpz_scan1(n_minus_one.get_mpz_t(), 0);
    mpz_class d;
    mpz_tdiv_q_2exp(d.get_mpz_t(), n_minus_one.get_mpz_t(), s);

    static const unsigned long reduced_bases[] = {2ul, 3ul, 5ul, 7ul, 11ul};
    mpz_class base_value;
    for (unsigned long base : reduced_bases) {
        if (mpz_cmp_ui(n.get_mpz_t(), base) <= 0) continue;
        mpz_set_ui(base_value.get_mpz_t(), base);
        if (!miller_rabin_round(n, n_minus_one, d, s, base_value)) return false;
    }

    return true;
}

// ============================================================
// RSA key structure
// ============================================================
struct rsa_key {
    mpz_class n;    // modulus  = p * q
    mpz_class e;    // public exponent
    mpz_class d;    // private exponent
    mpz_class p;    // first prime factor
    mpz_class q;    // second prime factor
    mpz_class phi;  // φ(n) = (p-1)(q-1)
};

// ============================================================
// Generate a probable prime of exactly `bits` bits
// ============================================================
inline mpz_class generate_prime(unsigned long bits, gmp_randclass& rng) {
    mpz_class candidate;
    while (true) {
        candidate = rng.get_z_bits(bits);
        mpz_setbit(candidate.get_mpz_t(), bits - 1);  // force MSB
        mpz_setbit(candidate.get_mpz_t(), 0);          // force odd

        if (is_probable_prime_reduced(candidate)) {
            return candidate;
        }
    }
}

// ============================================================
// KeyGen — uses extended_gcd for gcd check + invmod for d
// ============================================================
inline rsa_key rsa_keygen(unsigned long bits, gmp_randclass& rng) {
    rsa_key key;
    unsigned long half = bits / 2;

    cout << "[keygen] generating p (" << half << " bits)..." << endl;
    key.p = generate_prime(half, rng);

    cout << "[keygen] generating q (" << half << " bits)..." << endl;
    do {
        key.q = generate_prime(half, rng);
    } while (key.q == key.p);

    key.n   = key.p * key.q;
    key.phi = (key.p - 1) * (key.q - 1);
    key.e   = 65537;

    // gcd(e, phi) via extended_gcd
    auto [g, x, y] = extended_gcd(key.e, key.phi);
    if (g != 1) {
        throw runtime_error("gcd(e, phi) != 1 — regenerate primes");
    }

    // d = e⁻¹ mod phi via invmod (uses extended_gcd internally)
    key.d = invmod(key.e, key.phi);

    cout << "[keygen] done." << endl;
    return key;
}

// ============================================================
// Enc — Square-and-Multiply + Montgomery
//   c = m^e mod n
// ============================================================
inline mpz_class rsa_encrypt(const mpz_class& message, const mpz_class& e, const mpz_class& n) {
    MontgomeryContext ctx = make_montgomery_context(n);
    mpz_class m_mont = to_montgomery(message, ctx);
    mpz_class result = to_montgomery(1, ctx);

    size_t bit_len = mpz_sizeinbase(e.get_mpz_t(), 2);
    for (long i = static_cast<long>(bit_len) - 1; i >= 0; --i) {
        result = mont_mul(result, result, ctx);
        if (mpz_tstbit(e.get_mpz_t(), i)) {
            result = mont_mul(result, m_mont, ctx);
        }
    }
    return from_montgomery(result, ctx);
}

// ============================================================
// Dec — PSA + Montgomery
//   m = c^d mod n
// ============================================================
inline mpz_class rsa_decrypt(const mpz_class& ciphertext, const mpz_class& d, const mpz_class& n) {
    return psa_montgomery(ciphertext, d, n, 8, 42);
}

// ============================================================
// Sign — PSA + Montgomery  (s = m^d mod n)
// ============================================================
inline mpz_class rsa_sign(const mpz_class& message, const mpz_class& d, const mpz_class& n) {
    return psa_montgomery(message, d, n, 8, 42);
}

// ============================================================
// Verify — Square-and-Multiply + Montgomery
//   recovered = s^e mod n, compare to original message
// ============================================================
inline bool rsa_verify(const mpz_class& message, const mpz_class& signature,
                       const mpz_class& e, const mpz_class& n) {
    mpz_class recovered = rsa_encrypt(signature, e, n);
    return (recovered == message);
}

#endif  // METHODES_H