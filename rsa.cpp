#include<iostream>
#include<gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"




void keyGen(unsigned long bits, gmp_randclass& rng, mpz_class& n, mpz_class& e, mpz_class& d, mpz_class& p, mpz_class& q, mpz_class& phi) {
    p = genAlea(rng, bits / 2);
    q = genAlea(rng, bits / 2);
    n = p * q;
    phi = (p - 1) * (q - 1);

    e = 65537; // Choix classique pour e
    while (op_pgcd(e, phi) != 1) 
    {
        e += 2; // Incrémenter e jusqu'à ce qu'il soit premier avec phi
    }

    if (inversMod(d, e, phi) == 0) {
        throw runtime_error("Failed to compute modular inverse for d");
    }
}

void enc(mpz_class& c, string m, const mpz_class& e, const mpz_class& n) {
    stringToNum(c, m);
    c = mod_exp_window(c, e, n);
}

void dec();
void sing();  
bool verify();