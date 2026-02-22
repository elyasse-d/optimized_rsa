#include <iostream>
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

int main() {
    mpz_t a, n, res;
    mpz_inits(a, n, res, NULL);

    // Test avec de grands nombres
    mpz_set_str(a, "1000000000000000000000000000000", 10);
    mpz_set_str(n, "987654321", 10);

    // Appel de la fonction
    modulo(res, a, n);

    gmp_printf("Dividende a : %Zd\n", a);
    gmp_printf("Diviseur n  : %Zd\n", n);
    gmp_printf("Reste (a mod n) : %Zd\n", res);

    mpz_clears(a, n, res, NULL);
    return 0;
}