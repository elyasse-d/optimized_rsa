#ifndef BASE_H
#define BASE_H

#include <iostream>
#include <gmpxx.h>
#include <vector>

// raw mpz_t operations
void modulo(mpz_t r, const mpz_t a, const mpz_t n);
void quotient(mpz_t q, const mpz_t a, const mpz_t n);

// mpz_class wrapper for modulo
inline mpz_class modulo(const mpz_class& a, const mpz_class& n) {
    mpz_class r;
    modulo(r.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
    return r;
}

// exponentiation
mpz_class ExpoMod(mpz_class base, mpz_class exp, mpz_class n);
mpz_class mod_exp_window(mpz_class base, mpz_class exp, mpz_class n);

#endif  // BASE_H
