#include<iostream>
#include<gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"

using namespace std;


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
    c = ExpoMod(c, e, n);
}

void dec(string& m, const mpz_class& c, const mpz_class& d, const mpz_class& n) {
    mpz_class decrypted;
    decrypted = mod_exp_window(c, d, n);
    numToString(m, decrypted);
}

void sing();  
bool verify();


int main() {
    gmp_randclass rng(gmp_randinit_default);
    rng.seed(time(nullptr));

    mpz_class n, e, d, p, q, phi;
    keyGen(1024, rng, n, e, d, p, q, phi);

    cout << "Public key (n, e): (" << n.get_str(16) << ", " << e.get_str(16) << ")" << endl;
    cout << "Private key (d): " << d.get_str(16) << endl;

    string message = "Hello RSA!";
    mpz_class ciphertext;
    enc(ciphertext, message, e, n);
    cout << "Ciphertext: " << ciphertext.get_str(16) << endl;

    string decrypted;
    dec(decrypted, ciphertext, d, n);
    cout << "Decrypted message: " << decrypted<< endl;

    return 0;
}