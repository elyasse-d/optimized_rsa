#include <iostream>
#include <gmpxx.h>
#include <gmp.h>

// La fonction renommée "modulo" comme vous le souhaitiez
void modulo(mpz_t r, const mpz_t a, const mpz_t n) {
    if (mpz_cmp(a, n) < 0) {
        mpz_set(r, a);
        return;
    }
    mpz_t m, next_m, two;
    mpz_init_set(m, n);
    mpz_init(next_m);
    mpz_init_set_ui(two, 2);
    mpz_set(r, a);

    // Phase d'ascension
    while (true) {
        mpz_add(next_m, m, m);
        if (mpz_cmp(next_m, r) <= 0) {
            mpz_set(m, next_m);
        } else {
            break;
        }
    }

    // Phase de descente
    while (mpz_cmp(m, n) >= 0) {
        if (mpz_cmp(r, m) >= 0) {
            mpz_sub(r, r, m);
        }
        mpz_tdiv_q(m, m, two);
    }

    mpz_clears(m, next_m, two, NULL);
}


void quotient(mpz_t q, const mpz_t a, const mpz_t n) {
    // Phase 1 : Initialisation
    if (mpz_cmp(a, n) < 0) {
        mpz_set_ui(q, 0); // Si a < n, le quotient entier est 0
        return;
    }

    mpz_t m, next_m, r, two, p, next_p;
    mpz_init_set(m, n);           // Le bloc multiple de n (m = n * 2^i)
    mpz_init(next_m);
    mpz_init_set_ui(two, 2);
    mpz_init_set_ui(p, 1);        // La puissance de 2 actuelle (p = 2^i)
    mpz_init(next_p);
    mpz_init_set(r, a);           // r est le reste temporaire qu'on réduit
    mpz_set_ui(q, 0);             // q est la somme des p (le quotient final)

    // Phase 2 : Ascension
    // On cherche le plus grand m <= a, et on garde la puissance p correspondante
    while (true) {
        mpz_add(next_m, m, m);
        mpz_add(next_p, p, p);
        if (mpz_cmp(next_m, r) <= 0) {
            mpz_set(m, next_m);
            mpz_set(p, next_p);
        } else {
            break;
        }
    }

    // Phase 3 : Descente
    // Chaque fois qu'on peut soustraire le bloc m, on ajoute p au quotient
    while (mpz_cmp(m, n) >= 0) {
        if (mpz_cmp(r, m) >= 0) {
            mpz_sub(r, r, m);      // On réduit le reste
            mpz_add(q, q, p);      // ON AJOUTE LA PUISSANCE AU QUOTIENT
        }
        // On divise par 2 pour passer à l'étape inférieure
        mpz_tdiv_q(m, m, two);
        mpz_tdiv_q(p, p, two);
    }

    mpz_clears(m, next_m, r, two, p, next_p, NULL);
}

// Exponentiation Modulaire : Square and Multiply 
// Calcule (base^exp) mod n 
mpz_class ExpoMod(mpz_class base, mpz_class exp, mpz_class n) {
    mpz_class result = 1;
    
    // Étape 1 : Réduire la base initiale avec la fonction modulo
    base = modulo(base, n); 

    // Étape 2 : Parcourir les bits de l'exposant
    while (exp > 0) {
        // Si le bit de poids faible est à 1 (c'est-à-dire si exp est impair)
        if ((exp & 1) == 1) { 
            // On multiplie le résultat par la base courante, puis on réduit
            result = modulo(result * base, n);
        }
        
        // On décale l'exposant d'un bit vers la droite (équivaut à diviser par 2)
        exp = exp >> 1; 
        
        // On élève la base au carré pour le prochain bit, puis on réduit
        base = modulo(base * base, n); 
    }
    
    return result;
}

