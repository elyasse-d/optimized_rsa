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

// Exponentiation Modulaire : Square and Multiply 
// Calcule (base^exp) mod n 
mpz_class ExpoMod(mpz_class base, mpz_class exp, mpz_class n) {
    mpz_class result = 1;
    
    // Étape 1 : Réduire la base initiale avec la fonction modulo
    base = modulo(base, n); 

    // Étape 2 : Parcourir les bits de l'exposant
    while (exp > 0) {
        if ((exp & 1) == 1) { // Si le bit de poids faible est à 1 (c'est-à-dire si exp est impair)
            result = modulo(result * base, n); // On multiplie le résultat par la base courante, puis on réduit
        }     
        // On décale l'exposant d'un bit vers la droite (équivaut à diviser par 2)
        exp = exp >> 1;         
        // On élève la base au carré pour le prochain bit, puis on réduit
        base = modulo(base * base, n); 
    }
    
    return result;
}

// Exponentiation Modulaire : Sliding Window
// Calcule (base^exp) mod n de manière très optimisée
mpz_class mod_exp_window(mpz_class base, mpz_class exp, mpz_class n) {
    if (exp == 0) return 1;

    // Paramètre : Taille maximale de la fenêtre (4 bits est optimal pour RSA)
    int w = 4; 
    
    // Le tableau stockera 2^(w-1) = 8 valeurs (uniquement les puissances impaires)
    int num_precomp = 1 << (w - 1); 
    std::vector<mpz_class> g(num_precomp);

    //  1. PHASE DE PRÉCALCUL
    // On précalcule base^1, base^3, base^5, ..., base^15
    g[0] = modulo(base, n);                   // base^1
    mpz_class base2 = modulo(base * base, n); // base^2 (sert à sauter de 2 en 2)
    
    for (int j = 1; j < num_precomp; j++) {
        g[j] = modulo(g[j - 1] * base2, n);
    }

    //  2. CALCUL DU NOMBRE DE BITS DE L'EXPOSANT
    // Totalement "artisanal" pour respecter vos contraintes (sans fonction GMP)
    int total_bits = 0;
    mpz_class temp_exp = exp;
    while (temp_exp > 0) {
        total_bits++;
        temp_exp = temp_exp >> 1;
    }

    //  3. PHASE D'ÉVALUATION (Lecture de gauche à droite)
    mpz_class result = 1;
    int i = total_bits - 1;

    while (i >= 0) {
        // On lit le bit à la position i
        int bit_i = ((exp >> i) & 1) == 1 ? 1 : 0;

        if (bit_i == 0) {
            // Si le bit est 0, on fait juste une élévation au carré
            result = modulo(result * result, n);
            i--;
        } else {
            // Si le bit est 1, on cherche à grouper les bits dans une "fenêtre"
            // l est l'index de fin de la fenêtre (au maximum w bits plus loin)
            int l = std::max(0, i - w + 1);
            
            // Une fenêtre glissante DOIT se terminer par un bit à 1 (être impair)
            while ( (((exp >> l) & 1) == 1 ? 1 : 0) == 0 ) {
                l++; 
            }

            // On extrait la valeur entière contenue dans cette fenêtre binaire
            int window_value = 0;
            for (int j = i; j >= l; j--) {
                int bit_j = ((exp >> j) & 1) == 1 ? 1 : 0;
                window_value = (window_value << 1) + bit_j;
            }

            // On applique autant de carrés que la taille de la fenêtre qu'on vient de lire
            for (int j = 0; j < (i - l + 1); j++) {
                result = modulo(result * result, n);
            }

            // On multiplie par la valeur précalculée correspondante !
            // (Comme on n'a stocké que les impairs, l'indice est window_value / 2)
            result = modulo(result * g[window_value / 2], n);

            // On recule notre pointeur de lecture pour la suite
            i = l - 1;
        }
    }

    return result;
}
