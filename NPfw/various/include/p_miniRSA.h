//--------- p_miniRSA.h ----------------------
//--------------------------------------------
#ifndef p_miniRSA_H_
#define p_miniRSA_H_
//--------------------------------------------
#include "precHeader.h"
//--------------------------------------------
typedef __int64 dh_type;
//--------------------------------------------
class miniRSA
{
  public:
    miniRSA(dh_type p1, dh_type p2) : P1(p1), P2(p2), E(0), e(0), d(0) {}
    void calc();
    dh_type get_N() { return P1 * P2; }
    dh_type get_E() { return E; }
    dh_type get_pub() { return e; }
    dh_type get_priv() { return d; }
  private:
    dh_type P1;
    dh_type P2;
    dh_type E;
    dh_type e;
    dh_type d;
};
//--------------------------------------------
//--------------------------------------------
class Diffie_Hellman
{
  public:
    Diffie_Hellman(dh_type secret, dh_type prime) : Secret(secret), Prime(prime), Base(0) {}
    dh_type getValue(dh_type base);
  private:
    dh_type Secret;
    dh_type Prime;
    dh_type Base;
    dh_type run(dh_type curr);
};
//----------------------------------------------------------------------------
#endif
