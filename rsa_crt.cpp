#include <iostream>
#include <gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"
#include "lib/op_mod.h"

using namespace std;

// ============================================================
// KeyGen CRT — génère les clés RSA + paramètres CRT
//   dp = d mod (p-1)
//   dq = d mod (q-1)
//   qinv = q^(-1) mod p
// ============================================================
void keyGen_crt(unsigned long bits, gmp_randclass& rng,
                mpz_class& n, mpz_class& e, mpz_class& d,
                mpz_class& p, mpz_class& q, mpz_class& phi,
                mpz_class& dp, mpz_class& dq, mpz_class& qinv) {
    p = genAlea(rng, bits / 2);
    do {
        q = genAlea(rng, bits / 2);
    } while (q == p);

    n   = p * q;
    phi = (p - 1) * (q - 1);
    e   = 65537;

    while (op_pgcd(e, phi) != 1) {
        e += 2;
    }

    d = invmod(e, phi);
    if (d == 0) {
        throw runtime_error("Failed to compute modular inverse for d");
    }

    // Paramètres CRT
    dp   = modulo(d, p - 1);       // d mod (p-1)
    dq   = modulo(d, q - 1);       // d mod (q-1)
    qinv = invmod(q, p);           // q^(-1) mod p
}

// ============================================================
// Dec CRT — déchiffrement via le Théorème des Restes Chinois
//   m1 = c^dp mod p
//   m2 = c^dq mod q
//   h  = qinv * (m1 - m2) mod p
//   m  = m2 + h * q
// ============================================================
void dec_crt(string& m, const mpz_class& c,
             const mpz_class& p, const mpz_class& q,
             const mpz_class& dp, const mpz_class& dq,
             const mpz_class& qinv) {
    mpz_class m1 = mod_exp_window(c, dp, p);   // c^dp mod p
    mpz_class m2 = mod_exp_window(c, dq, q);   // c^dq mod q

    mpz_class h = modulo(qinv * (m1 - m2 + p), p);  // +p pour éviter négatif
    mpz_class result = m2 + h * q;

    numToString(m, result);
}

// ============================================================
// Sign CRT — signature via CRT  (s = m^d mod n via CRT)
// ============================================================
void sing_crt(mpz_class& signature, const string& message,
              const mpz_class& p, const mpz_class& q,
              const mpz_class& dp, const mpz_class& dq,
              const mpz_class& qinv) {
    mpz_class m_num;
    stringToNum(m_num, message);

    mpz_class s1 = mod_exp_window(m_num, dp, p);
    mpz_class s2 = mod_exp_window(m_num, dq, q);

    mpz_class h = modulo(qinv * (s1 - s2 + p), p);
    signature = s2 + h * q;
}
