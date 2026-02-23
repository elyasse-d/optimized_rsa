#ifndef RSA_H
#define RSA_H

#include <gmpxx.h>
#include <string>
using namespace std;

void keyGen(unsigned long bits, gmp_randclass& rng,
            mpz_class& n, mpz_class& e, mpz_class& d,
            mpz_class& p, mpz_class& q, mpz_class& phi);

void enc(mpz_class& c, std::string m, const mpz_class& e, const mpz_class& n);
void dec(string& m, const mpz_class& c, const mpz_class& d, const mpz_class& n);

void sing(mpz_class& signature, const string& message, const mpz_class& d, const mpz_class& n);
bool verify(const mpz_class& signature, const string& message, const mpz_class& e, const mpz_class& n);

#endif  // RSA_H
