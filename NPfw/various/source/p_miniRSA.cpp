//--------- p_miniRSA.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "p_miniRSA.h"
//--------------------------------------------
static dh_type Euclide(dh_type a, dh_type b)
{
  dh_type r;
  while (b != 0) {
    r = a % b;
    a = b;
    b = r;
    }
  return a; // quando b è (o è diventato) 0, il risultato è a
}
//--------------------------------------------
static dh_type coprimo(dh_type E, dh_type sum = 0)
{
  dh_type e = E / 3 + sum;
  dh_type add = 1;
  bool toggle = true;
  while(0 < e && e < E) {
    dh_type r = Euclide(E, e);
    if (1 == r)
      return e;
    if(toggle) {
      e += add;
      toggle = false;
      }
    else {
      e -= add;
      toggle = true;
      }
    ++add;
    }
  return e;
}
//--------------------------------------------
static dh_type calc_d_euclide(dh_type r1c1, dh_type r2c1, dh_type r1c2, dh_type r2c2)
{
  dh_type r2c3 = r1c1 / r2c1;
  dh_type r3c2 = r1c2 - r2c2 * r2c3;
  dh_type r3c1 = r1c1 % r2c1;
  if(1 == r3c1)
    return r3c2;
  return calc_d_euclide(r2c1, r3c1, r2c2, r3c2);
}
//--------------------------------------------
//--------------------------------------------
void miniRSA::calc()
{
  E = (P1 - 1) * (P2 - 1);
  dh_type add = 10;
  do {
    e = coprimo(E, add);
    d = calc_d_euclide(E, e, 0, 1);
    if (d < 0)
      d += E;
    if (d != e)
      break;
    add += 10;
  } while (true);
}
//--------------------------------------------
//--------------------------------------------
dh_type Diffie_Hellman::getValue(dh_type base)
{
  Base = base;
  if(Base > 1)
    return run(Secret);
  return Base;
}
//--------------------------------------------
dh_type Diffie_Hellman::run(dh_type curr)
{
  if(!curr)
    return 0;
  if(1 == curr)
    return Base;
  dh_type value = run(curr / 2);
  value *= value;
  value %= Prime;
  if (curr % 2)
    return (value * Base) % Prime;
  return value;
}
//----------------------------------------------------------------------------
