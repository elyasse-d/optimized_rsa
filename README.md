# optimized_rsa

```
https://cetinkayakoc.net/docs/r01.pdf
https://arxiv.org/pdf/2511.03341 just pour Le Rapport
```

```
g++ -O2 -std=c++17 -I. -o rsa_main rsa_main.cpp -lgmpxx -lgmp 2>&1
```
KeyGen() Prime GEneration , NIST SP800-90-A rev 1 , CFTP12 , Fouque Tibouchi \{AKS , ECC too expensive}
          Trial Division + Sieve (Deterministic) || Miller Rabin  {Carte a puce}
Enc()       L-To-R square and mult , Montgomery ladder, Barret, Window 
Dec()      
Sing()
Verify()
+ CRT MODE

```




```  
    base.h
    ==============
    modulo() //modulo_doublement =a[n]
    inversMod()
    op_pgcd()
    ExpoMod()  m^e[n]
    methodes.h
    genAlea()
    primTest() 
    stringToNum()
    numToString()

    base_ctr.h
    =============


    main()
    ===============
    keyGen()
    enc()
    dec()
    sing()
    verify()






    GenKey()
        -genPrime() 
        -TestPrime()=>p,q
        e= given
        -ecludEtend(phi(n),e)=d
    =============================
    enc(m,e,N)
            -m => nombre         COnversion(String)=> number 
            mod(m,e,n)           COnversion(Number)=> String
            converter(c)=> string 
    =============================
    Dec(c,d,N)
            mod(c,d,e)=m
            m  ==> string
    Sing(m,d,N)
            converter(m)=> nomber
            mod(m,d,n)
            converter(m)=> string 
    Verify(c,e,N,m)
            converter(m)=> nomber
            mod(m,d,n)
            converter(m)=> string 
            Compare : equal reutrn true/false   Compare()




```
test