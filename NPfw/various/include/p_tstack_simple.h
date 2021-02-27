//--------------- p_tstack_simple.h -------------------------------------
#ifndef P_TSTACK_SIMPLE_H_
#define P_TSTACK_SIMPLE_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
#include "setPack.h"
//-----------------------------------------------------------------------
template <typename T, int Sz>
class P_TStackSimple
{
  public:
    P_TStackSimple();
    ~P_TStackSimple() {  }

    bool Push(T d);

    T Pop();

    // ritorna il dato corrente senza rimuoverlo
    T getCurr();

  protected:

    T Set[Sz];
    int Added;
};
//-----------------------------------------------------------------------
#include "p_tstack_simple.hpp"
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
