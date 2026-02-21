
#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <array>
using namespace std;

// Calcule r = a mod n via la méthode de "doublement + soustraction".
// Version optimisée:
// - aligne directement m sur l'ordre de grandeur de a (bit-length),
// - évite la phase de montée itérative (m = 2m, 4m, 8m, ...),
// - conserve uniquement la phase de descente.
void modulo_doublement(mpz_t r, const mpz_t a, const mpz_t n) {
    // Cas invalides/triviaux: n <= 0 ou n == 1 => reste conventionnel 0.
    if (mpz_sgn(n) <= 0) {
        mpz_set_ui(r, 0);
        return;
    }

    if (mpz_cmp_ui(n, 1) == 0) {
        mpz_set_ui(r, 0);
        return;
    }

    // Travail en place sur r (copie de a).
    mpz_set(r, a);

    // Pour les entrées négatives, GMP fournit déjà une réduction robuste.
    if (mpz_sgn(r) < 0) {
        mpz_mod(r, r, n);
        return;
    }

    // Si a < n, le résultat est déjà correct.
    if (mpz_cmp(r, n) < 0) {
        return;
    }

    // m démarre à n puis sera aligné près de r via les bits.
    mpz_t m;
    mpz_init_set(m, n);

    // Nombre de bits de r et n pour calculer le décalage initial.
    const mp_bitcnt_t r_bits = mpz_sizeinbase(r, 2);
    const mp_bitcnt_t n_bits = mpz_sizeinbase(n, 2);

    // m = n << (r_bits - n_bits) pour arriver immédiatement proche de r.
    if (r_bits > n_bits) {
        mpz_mul_2exp(m, m, r_bits - n_bits);
    }

    // Ajustement fin: si m dépasse r, on redescend d'un bit.
    if (mpz_cmp(m, r) > 0) {
        mpz_tdiv_q_2exp(m, m, 1);
    }

    // Descente: à chaque niveau binaire, on soustrait m si possible,
    // puis on divise m par 2 jusqu'à revenir à n.
    while (mpz_cmp(m, n) >= 0) {
        if (mpz_cmp(r, m) >= 0) {
            mpz_sub(r, r, m);
        }
        mpz_tdiv_q_2exp(m, m, 1);
    }

    // Libération des temporaires GMP.
    mpz_clear(m);
}


// Small prime table used as a cheap pre-filter.
// Rejecting candidates here avoids expensive modular exponentiation later.
constexpr array<unsigned int, 22> kSmallPrimes = {
	7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u,
	47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u, 89u, 97u
};

// One Miller-Rabin witness test.
// n - 1 is decomposed as d * 2^s, then we test base a.
// Returns false immediately if a proves compositeness.
bool miller_rabin_round(
	const mpz_class& n,
	const mpz_class& n_minus_one,
	const mpz_class& d,
	unsigned long s,
	const mpz_class& a) {
	mpz_class x;
    //Optimize MPZ_POWM
	mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

	if (x == 1 || x == n_minus_one) {
		return true;
	}
    
	for (unsigned long r = 1; r < s; ++r) {
        //Optimize this MPZ_MUL and MPZ_MOD
		mpz_mul(x.get_mpz_t(), x.get_mpz_t(), x.get_mpz_t());
		mpz_mod(x.get_mpz_t(), x.get_mpz_t(), n.get_mpz_t());
		if (x == n_minus_one) {
			return true;
		}
	}

	return false;
}


// Fast deterministic pre-checks:
// - trivial values,
// - even/divisible by 3 or 5,
// - divisibility by additional small primes.
// This stage is much cheaper than Miller-Rabin rounds.
bool common_composite_checks(const mpz_class& n) {
	if (n < 2) {
		return false;
	}
	if (n == 2 || n == 3 || n == 5) {
		return true;
	}
	if (mpz_even_p(n.get_mpz_t()) != 0) {
		return false;
	}
	if (mpz_divisible_ui_p(n.get_mpz_t(), 3u) != 0 || mpz_divisible_ui_p(n.get_mpz_t(), 5u) != 0) {
		return false;
	}

	for (const unsigned int p : kSmallPrimes) {
		if (n == p) {
			return true;
		}
		if (mpz_divisible_ui_p(n.get_mpz_t(), p) != 0) {
			return false;
		}
	}
	return true;
}

// Reduced primality test (probable prime):
// 1) run cheap composite filters,
// 2) decompose n - 1 = d * 2^s,
// 3) run Miller-Rabin on a fixed small base set.
// Fixed bases reduce operations versus many random rounds.
bool is_probable_prime_reduced(const mpz_class& n) {
	if (!common_composite_checks(n)) {
		return false;
	}

	mpz_class n_minus_one = n - 1;
    //indexing the least significant 1 bit of n - 1 to find s, then d = (n - 1) / 2^s
	const unsigned long s = mpz_scan1(n_minus_one.get_mpz_t(), 0);
	mpz_class d;
	mpz_tdiv_q_2exp(d.get_mpz_t(), n_minus_one.get_mpz_t(), s);

	// Fixed witness set tuned for reduced-cost testing.
	static const unsigned long reduced_bases[] = {2ul, 3ul, 5ul, 7ul, 11ul};
	mpz_class base_value;
	for (unsigned long base : reduced_bases) {
		if (mpz_cmp_ui(n.get_mpz_t(), base) <= 0) {
			continue;
		}
		mpz_set_ui(base_value.get_mpz_t(), base);
		if (!miller_rabin_round(n, n_minus_one, d, s, base_value)) {
			return false;
		}
	}

	return true;
}



