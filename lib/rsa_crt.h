#ifndef RSA_CRT_H
#define RSA_CRT_H

#include <gmpxx.h>
#include <string>

void keyGen_crt(unsigned long bits, gmp_randclass& rng,
                mpz_class& n, mpz_class& e, mpz_class& d,
                mpz_class& p, mpz_class& q, mpz_class& phi,
                mpz_class& dp, mpz_class& dq, mpz_class& qinv);

void enc_crt(mpz_class& c, std::string m, const mpz_class& e, const mpz_class& n);

void dec_crt(std::string& m, const mpz_class& c,
             const mpz_class& p, const mpz_class& q,
             const mpz_class& dp, const mpz_class& dq,
             const mpz_class& qinv);

void sing_crt(mpz_class& signature, const std::string& message,
              const mpz_class& p, const mpz_class& q,
              const mpz_class& dp, const mpz_class& dq,
              const mpz_class& qinv);

bool verify_crt(const mpz_class& signature, const std::string& message,
                const mpz_class& e, const mpz_class& n);

#endif  // RSA_CRT_H
