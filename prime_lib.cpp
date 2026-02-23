#include<iostream>
#include<gmpxx.h>
#include<array>
#include "lib/base.h"

using namespace std;


// forward declaration
bool primTest(const mpz_class& n);

constexpr array<unsigned int, 22> SmallPrimes = {
    7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u,
    47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u, 89u, 97u
};

mpz_class genAlea(gmp_randclass& rng, unsigned long bits) {
    mpz_class alea;
    while (true) {
        alea = rng.get_z_bits(bits);
        mpz_setbit(alea.get_mpz_t(), bits - 1);  // force MSB
        mpz_setbit(alea.get_mpz_t(), 0);          // force odd

        if (primTest(alea)) {
            return alea;
        }
    }
}

// primTest() 

bool miller_rabin(
    const mpz_class& n,
    const mpz_class& n_1,
    const mpz_class& d,
    unsigned long s,
    const mpz_class& a) {
    mpz_class x;

    // mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
    x = mod_exp_window(a, d, n);
    if (x == 1 || x == n_1) return true;
    for (unsigned long r = 1; r < s; ++r) {
        mpz_mul(x.get_mpz_t(), x.get_mpz_t(), x.get_mpz_t()); //alt ?
        x=modulo( x, n);
        if (x == n_1) return true;
    }
    return false;
}



bool trialDiv(const mpz_class& n){
    if(n < 2) return false;
    if(n == 2 || n == 3 || n == 5) return true;
    if(mpz_even_p(n.get_mpz_t()) != 0) return false; 
    if(mpz_divisible_ui_p(n.get_mpz_t(), 3u) != 0 || mpz_divisible_ui_p(n.get_mpz_t(), 5u) != 0) return false;

    for(const unsigned int p : SmallPrimes){
        if(n == p) return true;
        if(mpz_divisible_ui_p(n.get_mpz_t(), p) != 0) return false;
    }
    return true;
}

bool  primTest(const mpz_class& n){
    if(!trialDiv(n)) return false;
    mpz_class n_1 = n - 1;
    mpz_class d,b_value;
    unsigned long s;
    static const unsigned long reduced_bases[] = {2ul, 3ul, 5ul, 7ul, 11ul};

    s = mpz_scan1(n_1.get_mpz_t(), 0);  //why 
    mpz_fdiv_q_2exp(d.get_mpz_t(), n_1.get_mpz_t(), s); //why 
    for (unsigned long base : reduced_bases) {
        if (mpz_cmp_ui(n.get_mpz_t(), base) <= 0) continue;
        mpz_set_ui(b_value.get_mpz_t(), base);  //why
        if (!miller_rabin(n, n_1, d, s, b_value)) return false;
    }    

    return true;
}
