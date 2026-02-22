#include <iostream>
#include <gmpxx.h>

// a mod m en utilisant uniquement les 4 opérations de base.
mpz_class modulo(const mpz_class& a, const mpz_class& m) {
    mpz_class q = a / m;     // quotient
    mpz_class r = a - q * m; // reste

    if (r < 0) {
        r += m;
    }

    return r;
}

mpz_class modexp(mpz_class base, mpz_class exp, const mpz_class& modn) {
    mpz_class result = 1;

    base = modulo(base, modn);

    while (exp > 0) {
        if (mod(exp, 2) == 1) {
            result = modulo(result * base, modn);
        }

        exp = exp / 2;
        base = modulo(base * base, modn);
    }

    return result;
}

mpz_class mulmod(const mpz_class& A, const mpz_class& B, const mpz_class& n) {
    mpz_class a = modulo(A, n);
    mpz_class b = modulo(B, n);
    mpz_class prod = a * b;
    return mod(prod, n);
}

// Algorithme d'Euclide étendu : retourne le pgcd de A et B ainsi que les coefficients de Bezout x et y tels que  A*x + B*y = pgcd(A, B).
std::tuple<mpz_class, mpz_class, mpz_class> extended_gcd(mpz_class A, mpz_class B) {
    mpz_class x0 = 1, y0 = 0;
    mpz_class x1 = 0, y1 = 1;

    while (B != 0) {
        mpz_class q = A / B;
        mpz_class r = A - q * B;

        A = B;
        B = r;

        mpz_class x_temp = x0 - q * x1;
        x0 = x1;
        x1 = x_temp;

        mpz_class y_temp = y0 - q * y1;
        y0 = y1;
        y1 = y_temp;
    }

    return {A, x0, y0}; // pgcd, x, y
}

// Inverse modulaire en utilisant l'algorithme d'Euclide étendu
mpz_class invmod(const mpz_class& A, const mpz_class& n) {
    auto [gcd, x, y] = extended_gcd(modulo(A,n), n);
    if (gcd != 1) {
        throw std::runtime_error("Inverse modulaire inexistant");
    }
    return modulo(x, n); // on normalise avec mod pour avoir un positif
}

int main() {
    mpz_class base("12345678901234567890");
    mpz_class exponent("12345");
    mpz_class modulus("987654321");
    mpz_class A = 17;
    mpz_class n = 3120;

    mpz_class result = modexp(base, exponent, modulus);
    std::cout << "Inverse modulaire: " << invmod(A, n) << std::endl;

    std::cout << "Resultat: " << result << std::endl;

    return 0;
}
