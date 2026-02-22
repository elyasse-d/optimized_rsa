#ifndef FONCTION_H
#define FONCTION_H

#include <gmpxx.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

struct MontgomeryContext {
	mpz_class n;
	mpz_class r;
	mpz_class r_mask;
	mpz_class n_prime;
	unsigned long k;
};

inline MontgomeryContext make_montgomery_context(const mpz_class& modulus) {
	if (modulus <= 0) {
		throw invalid_argument("modulus must be > 0");
	}
	if (!mpz_odd_p(modulus.get_mpz_t())) {
		throw invalid_argument("modulus must be odd for Montgomery arithmetic");
	}

	MontgomeryContext ctx;
	ctx.n = modulus;
	ctx.k = mpz_sizeinbase(modulus.get_mpz_t(), 2);

	mpz_ui_pow_ui(ctx.r.get_mpz_t(), 2u, ctx.k);
	if (ctx.r <= ctx.n) {
		++ctx.k;
		mpz_mul_2exp(ctx.r.get_mpz_t(), ctx.r.get_mpz_t(), 1u);
	}

	ctx.r_mask = ctx.r - 1;

	mpz_class n_inv;
	if (mpz_invert(n_inv.get_mpz_t(), ctx.n.get_mpz_t(), ctx.r.get_mpz_t()) == 0) {
		throw runtime_error("failed to invert n modulo R");
	}
	ctx.n_prime = (ctx.r - n_inv) & ctx.r_mask;

	return ctx;
}

inline mpz_class mont_mul(const mpz_class& a, const mpz_class& b, const MontgomeryContext& ctx) {
	const mpz_class t = a * b;
	const mpz_class m = ((t & ctx.r_mask) * ctx.n_prime) & ctx.r_mask;

	mpz_class u = t + m * ctx.n;
	mpz_fdiv_q_2exp(u.get_mpz_t(), u.get_mpz_t(), ctx.k);

	if (u >= ctx.n) {
		u -= ctx.n;
	}
	return u;
}

inline mpz_class to_montgomery(const mpz_class& x, const MontgomeryContext& ctx) {
	mpz_class reduced = x % ctx.n;
	if (reduced < 0) {
		reduced += ctx.n;
	}
	return (reduced * ctx.r) % ctx.n;
}

inline mpz_class from_montgomery(const mpz_class& x, const MontgomeryContext& ctx) {
	return mont_mul(x, 1, ctx);
}

inline vector<unsigned long> exponent_set_bits(const mpz_class& exponent) {
	if (exponent < 0) {
		throw invalid_argument("exponent must be >= 0");
	}

	vector<unsigned long> bits;
	const unsigned long bit_len = mpz_sizeinbase(exponent.get_mpz_t(), 2);
	for (unsigned long i = 0; i < bit_len; ++i) {
		if (mpz_tstbit(exponent.get_mpz_t(), i) != 0) {
			bits.push_back(i);
		}
	}
	return bits;
}

inline vector<vector<unsigned long>> split_indices(
	const vector<unsigned long>& indices,
	size_t split_size) {
	if (split_size == 0) {
		throw invalid_argument("split_size must be > 0");
	}

	vector<vector<unsigned long>> groups;
	groups.reserve((indices.size() + split_size - 1) / split_size);

	for (size_t i = 0; i < indices.size(); i += split_size) {
		const size_t end = min(i + split_size, indices.size());
		groups.emplace_back(indices.begin() + static_cast<long>(i), indices.begin() + static_cast<long>(end));
	}

	return groups;
}

inline mpz_class psa_montgomery(
	const mpz_class& message,
	const mpz_class& public_exponent,
	const mpz_class& modulus,
	size_t split_size = 8,
	uint64_t permutation_seed = 0xC0FFEEULL) {
	MontgomeryContext ctx = make_montgomery_context(modulus);
	const mpz_class m_mont = to_montgomery(message, ctx);
	const mpz_class one_mont = to_montgomery(1, ctx);

	vector<unsigned long> bit_positions = exponent_set_bits(public_exponent);
	mt19937_64 rng(permutation_seed);
	shuffle(bit_positions.begin(), bit_positions.end(), rng);

	const auto groups = split_indices(bit_positions, split_size);
	const unsigned long max_bit = bit_positions.empty() ? 0UL : *max_element(bit_positions.begin(), bit_positions.end());

	vector<mpz_class> powers(max_bit + 1, one_mont);
	if (max_bit > 0 || public_exponent == 1) {
		powers[0] = m_mont;
		for (unsigned long i = 1; i <= max_bit; ++i) {
			powers[i] = mont_mul(powers[i - 1], powers[i - 1], ctx);
		}
	}

	mpz_class acc = one_mont;
	for (const auto& group : groups) {
		mpz_class local = one_mont;
		for (const auto bit_index : group) {
			local = mont_mul(local, powers[bit_index], ctx);
		}
		acc = mont_mul(acc, local, ctx);
	}

	return from_montgomery(acc, ctx);
}

#endif  // FONCTION_H