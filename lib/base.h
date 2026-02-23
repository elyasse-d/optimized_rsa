#ifndef BASE_H
#define BASE_H

#include <iostream>
#include <gmpxx.h>
#include <vector>

mpz_class modulo(mpz_class a, mpz_class n);
mpz_class quotient(mpz_class a, mpz_class n);

mpz_class ExpoMod(mpz_class base, mpz_class exp, mpz_class n);
mpz_class mod_exp_window(mpz_class base, mpz_class exp, mpz_class n);
void stringToNum(mpz_class& num, const std::string& str);

#endif  // BASE_H
