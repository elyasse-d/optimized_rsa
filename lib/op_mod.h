#ifndef OP_MOD_H
#define OP_MOD_H

#include <gmpxx.h>
#include <tuple>
#include "base.h"

// Multiplication modulaire
mpz_class mulmod(const mpz_class& A, const mpz_class& B, const mpz_class& n);

// Euclide étendu : retourne (pgcd, x, y) tels que A*x + B*y = pgcd(A, B)
std::tuple<mpz_class, mpz_class, mpz_class> extended_gcd(mpz_class A, mpz_class B);

// Inverse modulaire via Euclide étendu
mpz_class invmod(const mpz_class& A, const mpz_class& n);

// PGCD via Euclide étendu
inline mpz_class op_pgcd(const mpz_class& A, const mpz_class& B) {
    auto [g, x, y] = extended_gcd(A, B);
    return g;
}

#endif  // OP_MOD_H
