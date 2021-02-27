//--------------- p_tqueue_simple.h -------------------------------------
#ifndef P_TQUEUE_SIMPLE_H_
#define P_TQUEUE_SIMPLE_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
#include "setPack.h"
//-----------------------------------------------------------------------
// implementata, per efficienza, come vettore circolare
//-----------------------------------------------------------------------
template <typename T, int Sz>
class P_TQueueSimple
{
  public:
    P_TQueueSimple();
    ~P_TQueueSimple() {  }

    bool Push(T d);

    T Pop();

    // ritorna il dato corrente senza rimuoverlo
    T getCurr();

    int getStored();
    int getFree();

    // copia (appende) tutta la coda in set, torna il numero di elementi aggiunti
    int copyTo(PVect<T>& set);
  protected:

    T Set[Sz];
    int Head;
    int Tail;
};
//-----------------------------------------------------------------------
#include "p_tqueue_simple.hpp"
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
