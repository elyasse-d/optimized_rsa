#include<iostream>
#include<gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"
#include "lib/op_mod.h"
#include "lib/rsa.h"
#include "lib/rsa_crt.h"



using namespace std;   

void rsa(){
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

    mpz_class signature;
    sing(signature, message, d, n);
    cout << "Signature: " << signature.get_str(16) << endl;

    bool verified = verify(signature, message, e, n);
    cout << "Signature verified: " << verified << endl;
}
void rsa_crt(){
    {
        gmp_randclass rng(gmp_randinit_default);
        rng.seed(time(nullptr) + 1);

        mpz_class n, e, d, p, q, phi, dp, dq, qinv;
        keyGen_crt(1024, rng, n, e, d, p, q, phi, dp, dq, qinv);

        cout << "Public key (n, e): (" << n.get_str(16) << ", " << e.get_str(16) << ")" << endl;
        cout << "Private key (d): " << d.get_str(16) << endl;
        cout << "CRT params: dp=" << dp.get_str(16).substr(0, 16) << "... dq=" << dq.get_str(16).substr(0, 16) << "..." << endl;

        string message = "Hello RSA CRT!";
        mpz_class ciphertext;
        enc_crt(ciphertext, message, e, n);
        cout << "Ciphertext: " << ciphertext.get_str(16) << endl;

        string decrypted;
        dec_crt(decrypted, ciphertext, p, q, dp, dq, qinv);
        cout << "Decrypted message: " << decrypted << endl;

        mpz_class signature;
        sing_crt(signature, message, p, q, dp, dq, qinv);
        cout << "Signature: " << signature.get_str(16) << endl;

        bool verified = verify_crt(signature, message, e, n);
        cout << "Signature verified: " << verified << endl;
    }
}


int main() {
    cout << "===Implementation de RSA mode Standard ===" << endl;
    rsa();
    cout << "\n===Implementation de RSA mode CRT ===" << endl;
    rsa_crt();
    
    return 0;
}