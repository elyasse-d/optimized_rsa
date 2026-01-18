#include <iostream>
#include <gmpxx.h>
#include <chrono>
using namespace std;

struct  rsa_st
{
    mpz_class n; 
    mpz_class e; 
    mpz_class d; 
    mpz_class r;
    mpz_class t;
    mpz_class pp;
    mpz_class m;

};

mpz_class multpGem_optimized(const mpz_class& m, const mpz_class& e, const mpz_class& n) {
    mpz_t r0, r1, temp;
    mpz_inits(r0, r1, temp, NULL);
    
    mpz_set_ui(r0, 1);
    mpz_set(r1, m.get_mpz_t());

    
    for (int i = mpz_sizeinbase(e.get_mpz_t(), 2) - 1; i >= 0; i--) {
        if (mpz_tstbit(e.get_mpz_t(), i)) {
            mpz_mul(r1, r0, r1);
            mpz_mod(r1, r1, n.get_mpz_t());
            
            mpz_mul(r0, r0, r0);
            mpz_mod(r0, r0, n.get_mpz_t());
        } else {
            mpz_mul(r0, r0, r1);
            mpz_mod(r0, r0, n.get_mpz_t());
            mpz_mul(r1, r1, r1);
            mpz_mod(r1, r1, n.get_mpz_t());
        }
    }
    mpz_class result(r0);
    mpz_clears(r0, r1, temp, NULL);
    return result;
}
mpz_class SquareAndMultiply(mpz_class base,mpz_class exp,mpz_class mod) {
    mpz_t result;
    mpz_inits(result, NULL);
    mpz_set_ui(result, 1);
    // mpz_class result = 1;    
    size_t bit_length = mpz_sizeinbase(exp.get_mpz_t(), 2);
    for (long i = bit_length - 1; i >= 0; i--) {
        // result = (result * result) % mod;
        mpz_mul(result, result, base.get_mpz_t());
        mpz_mod(result, result, mod.get_mpz_t());

        if (mpz_tstbit(exp.get_mpz_t(), i)) {
            // result = (result * base) % mod;
            mpz_mul(result, result, base.get_mpz_t());
            mpz_mod(result, result, mod.get_mpz_t());
        }
    }
    mpz_class result_class(result);
    mpz_clears(result, NULL);
    return result_class;
    
}


int main(){


    mpz_class base("123456789012345678901234567890");
    mpz_class exp("65537");
    mpz_class mod("987654321098765432109876543210");

    // 2. Measure multpGem
    

    // 3. Measure SquareAndMultiply
    auto start2 = std::chrono::steady_clock::now();
    mpz_class res2 = SquareAndMultiply(base, exp, mod);
    auto end2 = std::chrono::steady_clock::now();
    
    auto diff2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);


    auto start1 = std::chrono::steady_clock::now();
    mpz_class res1 = multpGem_optimized(base, exp, mod);
    auto end1 = std::chrono::steady_clock::now();
    
    auto diff1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

    // 4. Output results
    std::cout << "multpGem took: " << diff1.count() << " microseconds" << std::endl;
    std::cout << "SquareAndMultiply took: " << diff2.count() << " microseconds" << std::endl;
    return 0;
}