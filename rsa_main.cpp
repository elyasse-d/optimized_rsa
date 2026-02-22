// ============================================================
// rsa_main.cpp — Full RSA: KeyGen, Enc, Dec, Sign, Verify
// Uses:
//   methodes.h  → is_probable_prime_reduced (trial division + Miller-Rabin)
//   fonction.h  → rsa_psa::* (Montgomery context, PSA exponentiation)
// ============================================================

#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <string>
#include <cstdint>

// Project headers
#include "methodes.h"   // is_probable_prime_reduced
#include "fonction.h"   // rsa_psa::* (Montgomery + PSA)

using namespace std;

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
mpz_class generate_prime(unsigned long bits, gmp_randclass& rng) {
    mpz_class candidate;
    while (true) {
        candidate = rng.get_z_bits(bits);
        mpz_setbit(candidate.get_mpz_t(), bits - 1);  // force MSB → exactly b bits
        mpz_setbit(candidate.get_mpz_t(), 0);          // force LSB → odd

        if (is_probable_prime_reduced(candidate)) {
            return candidate;
        }
    }
}

// ============================================================
// KeyGen — generate an RSA key pair
// ============================================================
rsa_key rsa_keygen(unsigned long bits, gmp_randclass& rng) {
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

    mpz_class g;
    mpz_gcd(g.get_mpz_t(), key.e.get_mpz_t(), key.phi.get_mpz_t());
    if (g != 1) {
        throw runtime_error("gcd(e, phi) != 1 — regenerate primes");
    }

    if (mpz_invert(key.d.get_mpz_t(), key.e.get_mpz_t(), key.phi.get_mpz_t()) == 0) {
        throw runtime_error("e is not invertible mod phi(n)");
    }

    cout << "[keygen] done." << endl;
    return key;
}

// ============================================================
// Enc — Square-and-Multiply + Montgomery
//   c = m^e mod n
// ============================================================
mpz_class rsa_encrypt(const mpz_class& message, const mpz_class& e, const mpz_class& n) {
    rsa_psa::MontgomeryContext ctx = rsa_psa::make_montgomery_context(n);
    mpz_class m_mont = rsa_psa::to_montgomery(message, ctx);
    mpz_class result = rsa_psa::to_montgomery(1, ctx);

    size_t bit_len = mpz_sizeinbase(e.get_mpz_t(), 2);
    for (long i = static_cast<long>(bit_len) - 1; i >= 0; --i) {
        result = rsa_psa::mont_mul(result, result, ctx);
        if (mpz_tstbit(e.get_mpz_t(), i)) {
            result = rsa_psa::mont_mul(result, m_mont, ctx);
        }
    }
    return rsa_psa::from_montgomery(result, ctx);
}

// ============================================================
// Dec — PSA + Montgomery
//   m = c^d mod n
// ============================================================
mpz_class rsa_decrypt(const mpz_class& ciphertext, const mpz_class& d, const mpz_class& n) {
    return rsa_psa::rsa_encrypt_psa_montgomery(ciphertext, d, n, 8, 42);
}

// ============================================================
// Sign — PSA + Montgomery  (signature = hash^d mod n)
//   s = h(m)^d mod n
// ============================================================
mpz_class rsa_sign(const mpz_class& hash, const mpz_class& d, const mpz_class& n) {
    return rsa_psa::rsa_encrypt_psa_montgomery(hash, d, n, 8, 42);
}

// ============================================================
// Verify — Square-and-Multiply + Montgomery
//   h' = s^e mod n  then compare h' == h(m)
// ============================================================
bool rsa_verify(const mpz_class& hash, const mpz_class& signature,
                const mpz_class& e, const mpz_class& n) {
    mpz_class recovered = rsa_encrypt(signature, e, n);  // s^e mod n
    return (recovered == hash);
}

// ============================================================
// Main — demonstrate all five operations
// ============================================================
int main() {
    // --- GMP random init ---
    gmp_randclass rng(gmp_randinit_default);
    rng.seed(chrono::steady_clock::now().time_since_epoch().count());

    rsa_key key;
    bool has_key = false;
    int choice = 0;

    while (true) {
        cout << "\n========================================" << endl;
        cout << "           RSA Menu" << endl;
        cout << "========================================" << endl;
        cout << "  1. KeyGen" << endl;
        cout << "  2. Encrypt" << endl;
        cout << "  3. Decrypt" << endl;
        cout << "  4. Sign" << endl;
        cout << "  5. Verify" << endl;
        cout << "  0. Quit" << endl;
        cout << "========================================" << endl;
        cout << "  Choice: ";
        cin >> choice;

        switch (choice) {

        case 1: {
            unsigned long bits;
            cout << "  Enter key size in bits (e.g. 1024, 2048): ";
            cin >> bits;
            cout << "\n── KeyGen (" << bits << " bits) ──" << endl;
            key = rsa_keygen(bits, rng);
            has_key = true;
            cout << "  n   = " << key.n.get_str(16) << endl;
            cout << "  e   = " << key.e << endl;
            cout << "  d   = " << key.d.get_str(16) << endl;
            cout << "  p   = " << key.p.get_str(16) << endl;
            cout << "  q   = " << key.q.get_str(16) << endl;
            cout << "  phi = " << key.phi.get_str(16) << endl;
            break;
        }

        case 2: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Encrypt (Square-and-Multiply + Montgomery) ──" << endl;
            string input;
            cout << "  Enter message m (integer): ";
            cin >> input;
            mpz_class m(input);
            if (m >= key.n) { cout << "  [!] m must be < n." << endl; break; }
            mpz_class c = rsa_encrypt(m, key.e, key.n);
            cout << "  c = " << c.get_str(16) << endl;
            break;
        }

        case 3: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Decrypt (PSA + Montgomery) ──" << endl;
            string input;
            cout << "  Enter ciphertext c (hex): ";
            cin >> input;
            mpz_class c(input, 16);
            if (c >= key.n) { cout << "  [!] c must be < n." << endl; break; }
            mpz_class m = rsa_decrypt(c, key.d, key.n);
            cout << "  m = " << m << endl;
            break;
        }

        case 4: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Sign (PSA + Montgomery) ──" << endl;
            string input;
            cout << "  Enter message m (integer): ";
            cin >> input;
            mpz_class m(input);
            if (m >= key.n) { cout << "  [!] m must be < n." << endl; break; }
            mpz_class s = rsa_sign(m, key.d, key.n);
            cout << "  signature s = " << s.get_str(16) << endl;
            break;
        }

        case 5: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Verify (Square-and-Multiply + Montgomery) ──" << endl;
            string s_input, m_input;
            cout << "  Enter signature s (hex): ";
            cin >> s_input;
            cout << "  Enter original message m (integer): ";
            cin >> m_input;
            mpz_class s(s_input, 16);
            mpz_class m(m_input);
            bool valid = rsa_verify(m, s, key.e, key.n);
            cout << "  s^e mod n = " << rsa_encrypt(s, key.e, key.n) << endl;
            cout << "  valid = " << (valid ? "YES" : "NO") << endl;
            break;
        }

        case 0:
            cout << "  Bye." << endl;
            return 0;

        default:
            cout << "  [!] Invalid choice." << endl;
            break;
        }
    }

    return 0;
}