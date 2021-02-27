//------------- p_RSA.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "p_RSA.h"
//--------------------------------------------
static void Euclide(dh_type result, dh_type a, dh_type b)
{
#if 1
  mpz_gcd(result, a, b);
#else  // per diletto, fatto a mano
  dh_type r;
  mpz_init(r);
  dh_type a2;
  mpz_init(a2);
  mpz_set(a2, a);
  dh_type b2;
  mpz_init(b2);
  mpz_set(b2, b);

  while(mpz_cmp_ui(b2, 0) != 0) {
    mpz_tdiv_r(r, a2, b2);
    mpz_set(a2, b2);
    mpz_set(b2, r);
    }
  mpz_set(result, a2);
  mpz_clear(r);
  mpz_clear(a2);
  mpz_clear(b2);
#endif
}
//--------------------------------------------
static void coprimo(dh_type result, dh_type E, int sum)
{
  dh_type e;
  dh_type add;
  dh_type r;
  bool toggle = true;

  mpz_init(e);
  mpz_init(add);
  mpz_set_ui(add, 1);
  mpz_init(r);

  mpz_tdiv_q_ui(e, E, 3);
  mpz_add_ui(e, e, sum);
  while(mpz_cmp_si(e, 0) > 0 && mpz_cmp(e, E) < 0) {
  Euclide(r, E, e);
    if (mpz_cmp_si(r, 1) == 0) {
      mpz_set(result, e);
      break;
      }
    if(toggle) {
      mpz_add(e, e, add);
      toggle = false;
      }
    else {
      mpz_sub(e, e, add);
      toggle = true;
      }
    mpz_add_ui(add, add, 1);
    }
  mpz_clear(e);
  mpz_clear(add);
  mpz_clear(r);
}
//--------------------------------------------
static void calc_d_euclide(dh_type result, dh_type r1c1, dh_type r2c1, dh_type r1c2, dh_type r2c2)
{
  dh_type r2c3;
  dh_type r3c2;
  dh_type r3c1;
  dh_type tmp;
  mpz_init(r2c3);
  mpz_init(r3c2);
  mpz_init(r3c1);
  mpz_init(tmp);

  mpz_tdiv_q(r2c3, r1c1, r2c1);

  mpz_set(r3c2, r1c2);
  mpz_submul(r3c2, r2c2, r2c3);

  mpz_tdiv_r(r3c1, r1c1, r2c1);
  if (mpz_cmp_si(r3c1, 1) == 0)
      mpz_set(result, r3c2);
  else
    calc_d_euclide(result, r2c1, r3c1, r2c2, r3c2);
  mpz_clear(r2c3);
  mpz_clear(r3c2);
  mpz_clear(r3c1);
  mpz_clear(tmp);
}
//--------------------------------------------
RSA::RSA(dh_type p1, dh_type p2)
{
  mpz_init(P1);
  mpz_init(P2);
  mpz_set(P1, p1);
  mpz_set(P2, p2);
  init();
}
//--------------------------------------------
RSA::RSA(char* p1, char* p2)
{
  mpz_init_set_str(P1, p1, 10);
  mpz_init_set_str(P2, p2, 10);
  init();
}
//--------------------------------------------
RSA::~RSA()
{
  mpz_clear(P1);
  mpz_clear(P2);
  mpz_clear(N);
  mpz_clear(E);
  mpz_clear(e);
  mpz_clear(d);
}
//--------------------------------------------
void RSA::init()
{
  mpz_init(N);
  mpz_init(E);
  mpz_init(e);
  mpz_init(d);

  mpz_mul(N, P1, P2);

  mpz_sub_ui(e, P1, 1);
  mpz_sub_ui(d, P2, 1);
  mpz_mul(E, e, d);
}
//--------------------------------------------
void RSA::get_N(dh_type n)
{
  mpz_set(n, N);
}
//--------------------------------------------
void RSA::get_E(dh_type e)
{
  mpz_set(e, E);
}
//--------------------------------------------
void RSA::get_pub(dh_type pub)
{
  mpz_set(pub, e);
}
//--------------------------------------------
void RSA::get_priv(dh_type priv)
{
  mpz_set(priv, d);
}
//--------------------------------------------
void RSA::calc()
{
  int add = 10;
  dh_type t1;
  dh_type t2;
  mpz_init(t1);
  mpz_init(t2);
  mpz_set_ui(t1, 0);
  mpz_set_ui(t2, 1);
  do {
    coprimo(e, E, add);
    calc_d_euclide(d, E, e, t1, t2);
    if (mpz_cmp_ui(d, 0) < 0)
      mpz_add(d, d, E);
    if (mpz_cmp(d, e) != 0)
      break;
    add += 10;
  } while (true);
  mpz_clear(t1);
  mpz_clear(t2);
}
//--------------------------------------------
//--------------------------------------------
Diffie_Hellman::Diffie_Hellman(dh_type secret, dh_type prime)
{
  mpz_init(Secret);
  mpz_init(Prime);
  mpz_init(Base);
  mpz_set(Secret, secret);
  mpz_set(Prime, prime);
}
//--------------------------------------------
Diffie_Hellman::~Diffie_Hellman()
{
  mpz_clear(Secret);
  mpz_clear(Prime);
  mpz_clear(Base);
}
//--------------------------------------------
void Diffie_Hellman::getValue(dh_type result, dh_type base)
{
  mpz_set(Base, base);
  if(mpz_cmp_ui(Base, 1) > 0)
    run(result, Secret);
  else
    mpz_set(result, Base);
}
//--------------------------------------------
void Diffie_Hellman::run(dh_type result, dh_type curr)
{
#if 1
  mpz_powm(result, Base, curr, Prime);
#else // per diletto, fatto a mano
  if(mpz_cmp_ui(curr, 0) == 0)
    mpz_set_ui(result, 0);
  else if(mpz_cmp_ui(curr, 1) == 0)
    mpz_set(result, Base);
  else {
    dh_type value;
    mpz_init(value);
    dh_type curr2;
    mpz_init(curr2);
    mpz_tdiv_q_ui(curr2, curr, 2);
    run(value, curr2);
    mpz_powm_ui(value, value, 2, Prime);
    if(mpz_odd_p(curr)) {
      mpz_mul(result, value, Base);
      mpz_tdiv_r(result, result, Prime);
      }
    else
      mpz_set(result, value);
    mpz_clear(value);
    mpz_clear(curr2);
    }
#endif
}
//----------------------------------------------------------------------------
