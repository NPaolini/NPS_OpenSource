//------------- p_RSA.h ----------------------
//--------------------------------------------
#ifndef p_RSA_H_
#define p_RSA_H_
//--------------------------------------------
#include "precHeader.h"
#include "mpir.h"
//--------------------------------------------
typedef mpz_t dh_type;
//--------------------------------------------
class RSA
{
  public:
    RSA(dh_type p1, dh_type p2);
  RSA(char* p1, char* p2);
    ~RSA();
    void calc();
    void get_N(dh_type n);
    void get_E(dh_type e);
    void get_pub(dh_type pub);
    void get_priv(dh_type priv);
  private:
    void init();
    dh_type P1;
    dh_type P2;
    dh_type N;
    dh_type E;
    dh_type e;
    dh_type d;
};
//--------------------------------------------
//--------------------------------------------
class Diffie_Hellman
{
  public:
    Diffie_Hellman(dh_type secret, dh_type prime);
    ~Diffie_Hellman();
    void getValue(dh_type result, dh_type base);
  private:
    dh_type Secret;
    dh_type Prime;
    dh_type Base;
    void run(dh_type result, dh_type curr);
};
//----------------------------------------------------------------------------
#endif
