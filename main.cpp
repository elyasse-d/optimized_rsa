#include<iostream>
#include<gmpxx.h>
#include "lib/base.h"
#include "lib/prime_lib.h"
#include "lib/op_mod.h"
#include "lib/rsa.h"
#include "lib/rsa_crt.h"


using namespace std;   

int main() {
    gmp_randclass rng(gmp_randinit_default);
    rng.seed(time(nullptr));
    mpz_class n, e, d, p, q, phi ,dp, dq, qinv;
    string message = "Master Cryptis!",decrypted;
    mpz_class ciphertext,signature;
    bool verified;


    cout << "\033[2J\033[1;1H";
    cout << "\n====================================================" << endl;
    cout << "          RSA à jeu réduit d’instruction            " << endl;
    cout << "          Master Cryptis                            " << endl;
    cout << "====================================================" << endl;
    cout << "Enter 0 to continue...." << endl;
    int temp;
    cin >> temp;
    cout << "\033[2J\033[1;1H";
    int choice=99;
    while(choice != 0){
        cout << "====================================================" << endl;
        cout << "Choisir une Option:" << endl;
        cout << "Menu:" << endl;
        cout << "1-Génération de Clés RSA " << endl;
        cout << "2-Chiffrement RSA standard " << endl;
        cout << "3-Déchiffrement RSA standard " << endl;
        cout << "4-Signature RSA standard " << endl;
        cout << "5-Vérification RSA standard " << endl;
        cout << "6-Déchiffrement RSA CRT " << endl;
        cout << "7-Signature RSA CRT " << endl;
        cout << "8-Vérification RSA CRT " << endl;
        cout << "0- Exit  "  << endl;
        cout << "====================================================" << endl;
        cin >> choice;
        switch (choice)
        {
        case 1:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Génération de Clés RSA                     " << endl;
            cout << "====================================================" << endl;
            keyGen_crt(1024, rng, n, e, d, p, q, phi, dp, dq, qinv);
            cout << "La clef publique (n, e):" << endl;
            cout << "n :"<< n.get_str(16) << endl;
            cout << "e :"<< e.get_str(16)<< endl;
            cout << "La clef privée (d): " << d.get_str(16) << endl;
            cout << "\n====================================================" << endl;
            cout << "          Génération de Clés RSA CRT                   " << endl;
            cout << "====================================================" << endl;
            cout << "Paramètres CRT: " << endl;
            cout << "(dp)=" << dp.get_str(16) << endl;
            cout << "(dq)=" << dq.get_str(16) << endl;
            cout << "(qinv)=" << qinv.get_str(16) << endl;
            break;
        case 2:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Chiffrement RSA standard                   " << endl;
            cout << "====================================================" << endl;
            enc(ciphertext, message, e, n);
            cout << "Ciphertext: " << ciphertext.get_str(16) << endl;
            break;
        case 3:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Déchiffrement RSA standard                   " << endl;
            cout << "====================================================" << endl;
            dec(decrypted, ciphertext, d, n);
            cout << "Message déchiffré: " << decrypted<< endl;
            break;
        case 4:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Signature RSA standard                   " << endl;     
            cout << "====================================================" << endl;
            sing(signature, message, d, n);
            cout << "Signature: " << signature.get_str(16) << endl;
            break;
        case 5:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Vérification RSA standard                   " << endl;
            cout << "====================================================" << endl;
            verified = verify(signature, message, e, n);
            cout << "Signature verifie: " << verified << endl;
            break;
        case 6:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Déchiffrement RSA CRT                   " << endl;
            cout << "====================================================" << endl;
            dec_crt(decrypted, ciphertext, p, q, dp, dq, qinv);
            cout << "Message déchiffré: " << decrypted << endl;
            break;
        case 7:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Signature RSA CRT                   " << endl;
            cout << "====================================================" << endl;
            sing_crt(signature, message, p, q, dp, dq, qinv);
            cout << "Signature: " << signature.get_str(16) << endl;
            break;
        case 8:
            cout << "\033[2J\033[1;1H";
            cout << "\n====================================================" << endl;
            cout << "          Vérification RSA CRT                   " << endl;
            cout << "====================================================" << endl;
            verified = verify(signature, message, e, n);
            cout << "Signature verifie: " << verified << endl;
            break;
        case 0:
            cout << "Exiting..." << endl;
            break;
        default:
            cout << "Choix invalide. Veuillez réessayer" << endl;
            break;
        }
    }


    return 0;
}