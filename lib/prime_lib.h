#ifndef PRIME_LIB_H
#define PRIME_LIB_H

#include <gmpxx.h>
#include <array>
#include "base.h"

constexpr std::array<unsigned int, 22> SmallPrimes = {
    7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u,
    47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u, 89u, 97u
};

bool trialDiv(const mpz_class& n);
bool miller_rabin(const mpz_class& n, const mpz_class& n_1,
                  const mpz_class& d, unsigned long s,
                  const mpz_class& a);
bool primTest(const mpz_class& n);
mpz_class genAlea(gmp_randclass& rng, unsigned long bits);

#endif  // PRIME_LIB_H
