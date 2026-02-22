// ============================================================
// rsa_main.cpp — Menu only. All logic lives in methodes.h
// ============================================================

#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <string>

#include "methodes.h"

using namespace std;

int main() {
    gmp_randclass rng(gmp_randinit_default);
    rng.seed(chrono::steady_clock::now().time_since_epoch().count());

    rsa_key key;
    bool has_key = false;
    int choice = 0;

    while (true) {
        cout << "\n========================================" << endl;
        cout << "           RSA Menu" << endl;
        cout << "========================================" << endl;
        cout << "  1. KeyGen" << endl;
        cout << "  2. Encrypt" << endl;
        cout << "  3. Decrypt" << endl;
        cout << "  4. Sign" << endl;
        cout << "  5. Verify" << endl;
        cout << "  0. Quit" << endl;
        cout << "========================================" << endl;
        cout << "  Choice: ";
        cin >> choice;

        switch (choice) {

        case 1: {
            unsigned long bits;
            cout << "  Enter key size in bits (e.g. 1024, 2048): ";
            cin >> bits;
            cout << "\n── KeyGen (" << bits << " bits) ──" << endl;
            key = rsa_keygen(bits, rng);
            has_key = true;
            cout << "  n   = " << key.n.get_str(16) << endl;
            cout << "  e   = " << key.e << endl;
            cout << "  d   = " << key.d.get_str(16) << endl;
            cout << "  p   = " << key.p.get_str(16) << endl;
            cout << "  q   = " << key.q.get_str(16) << endl;
            cout << "  phi = " << key.phi.get_str(16) << endl;
            break;
        }

        case 2: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Encrypt (Square-and-Multiply + Montgomery) ──" << endl;
            string input;
            cout << "  Enter message m (integer): ";
            cin >> input;
            mpz_class m(input);
            if (m >= key.n) { cout << "  [!] m must be < n." << endl; break; }
            mpz_class c = rsa_encrypt(m, key.e, key.n);
            cout << "  c = " << c.get_str(16) << endl;
            break;
        }

        case 3: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Decrypt (PSA + Montgomery) ──" << endl;
            string input;
            cout << "  Enter ciphertext c (hex): ";
            cin >> input;
            mpz_class c(input, 16);
            if (c >= key.n) { cout << "  [!] c must be < n." << endl; break; }
            mpz_class m = rsa_decrypt(c, key.d, key.n);
            cout << "  m = " << m << endl;
            break;
        }

        case 4: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Sign (PSA + Montgomery) ──" << endl;
            string input;
            cout << "  Enter message m (integer): ";
            cin >> input;
            mpz_class m(input);
            if (m >= key.n) { cout << "  [!] m must be < n." << endl; break; }
            mpz_class s = rsa_sign(m, key.d, key.n);
            cout << "  signature s = " << s.get_str(16) << endl;
            break;
        }

        case 5: {
            if (!has_key) { cout << "  [!] Generate a key first (option 1)." << endl; break; }
            cout << "\n── Verify (Square-and-Multiply + Montgomery) ──" << endl;
            string s_input, m_input;
            cout << "  Enter signature s (hex): ";
            cin >> s_input;
            cout << "  Enter original message m (integer): ";
            cin >> m_input;
            mpz_class s(s_input, 16);
            mpz_class m(m_input);
            bool valid = rsa_verify(m, s, key.e, key.n);
            cout << "  s^e mod n = " << rsa_encrypt(s, key.e, key.n) << endl;
            cout << "  valid = " << (valid ? "YES" : "NO") << endl;
            break;
        }

        case 0:
            cout << "  Bye." << endl;
            return 0;

        default:
            cout << "  [!] Invalid choice." << endl;
            break;
        }
    }

    return 0;
}