#include <iostream>
#include <gmpxx.h>

mpz_class modulo(mpz_class a, mpz_class n) {
    // Sécurité : n = 0
    if (n == 0) {
        std::cerr << "Erreur : Modulo par zéro !" << std::endl;
        return 0;
    }

    // Cas simple
    if (a < n) return a;

    mpz_class m = n;
    mpz_class r = a;

    // --- Phase 1 : Ascension ---
    // On double m jusqu'à atteindre le plus grand multiple <= r
    while ((m + m) <= r) {
        m = m + m; // On n'utilise que l'addition
    }

    // --- Phase 2 : Descente ---
    // On réduit r en utilisant les blocs m
    while (m >= n) {
        if (r >= m) {
            r = r - m; // On n'utilise que la soustraction
        }
        // Division par 2 via décalage binaire (Right Shift)
        // C'est l'opération de base équivalente à mpz_tdiv_q_2exp
        m = m/2; // Division par 2 (décalage à droite)
    }

    return r;
}


// -----La fonction quotient -------
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
        mpz_fdiv_q(m, m, two);
        mpz_fdiv_q(p, p, two);
    }

    mpz_clears(m, next_m, r, two, p, next_p, NULL);
}
