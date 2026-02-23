#include<iostream>
#include<gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"
#include "lib/op_mod.h"


using namespace std;


void keyGen(unsigned long bits, gmp_randclass& rng, mpz_class& n, mpz_class& e, mpz_class& d, mpz_class& p, mpz_class& q, mpz_class& phi) {
    p = genAlea(rng, bits / 2);
    do {
        q = genAlea(rng, bits / 2);
    } while (q == p);
    n = p * q;
    phi = (p - 1) * (q - 1);

    e = 65537; // Choix classique pour e
    while (op_pgcd(e, phi) != 1) 
    {
        e += 2; // Incrémenter e jusqu'à ce qu'il soit premier avec phi
    }

    d = invmod(e, phi);
    if(d == 0) {
        throw runtime_error("Failed to compute modular inverse for d");
    }
}

void enc(mpz_class& c, string m, const mpz_class& e, const mpz_class& n) {
    stringToNum(c, m);
    c = ExpoMod(c, e, n);
}

void dec(string& m, const mpz_class& c, const mpz_class& d, const mpz_class& n) {
    mpz_class decrypted;
    decrypted = mod_exp_window(c, d, n);
    numToString(m, decrypted);
}

void sing(mpz_class& signature, const string& message, const mpz_class& d, const mpz_class& n) {
    mpz_class m_num;
    stringToNum(m_num, message);
    signature = mod_exp_window(m_num, d, n);
}

bool verify(const mpz_class& signature, const string& message, const mpz_class& e, const mpz_class& n) {
    mpz_class m_num;
    stringToNum(m_num, message);
    mpz_class decrypted = mod_exp_window(signature, e, n);
    return decrypted == m_num;
}


