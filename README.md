# optimized_rsa
L’objectif de ce projet est d’implémenter les différentes fonctionnalités du RSA avec la contrainte de ne pas disposer de fonctions mathématiques évoluées, et de se limiter aux quatre opérations de base sur les grands entiers. Cette limitation permet de se rapprocher d’un contexte de développement du RSA en environnement contraint comme par exemple la programmation d’un cryptoprocesseur pour carte à puce.


# 📁 Structure du Projet
Le projet RSA est organisé comme suit :
```
/lib
   ->/base.h"
   ->/prime_lib.h"
   ->/op_mod.h"
   ->/rsa.h"
   ->/rsa_crt.h
base.cpp
prime_lib.cpp
op_mod.cpp
rsa.cpp
rsa_crt.cpp
```  

```
https://cetinkayakoc.net/docs/r01.pdf
https://arxiv.org/pdf/2511.03341 just pour Le Rapport
```

```
g++ -O2 -std=c++17 -I. -o rsa_main rsa_main.cpp -lgmpxx -lgmp 2>&1
```
