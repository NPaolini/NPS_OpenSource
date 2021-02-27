//--------------- p_tqueue_simpleCS.h -------------------------------------
#ifndef P_TQUEUE_SIMPLE_CS_H_
#define P_TQUEUE_SIMPLE_CS_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
#include "p_tqueue_simple.h"
#include "PCrt_lck.h"
#include "setPack.h"
//-----------------------------------------------------------------------
// implementata, per efficienza, come vettore circolare
//-----------------------------------------------------------------------
template <typename T, int Sz>
class P_TQueueSimpleCS
{
  public:
    P_TQueueSimpleCS() {}
    ~P_TQueueSimpleCS() {  }

    bool Push(T d);
    T Pop();
    T getCurr();
    int getStored();
    int getFree();
    int copyTo(PVect<T>& set);

    criticalSect& getCS_Queue() { return CSQ; }
  private:
    P_TQueueSimple<T, Sz> Queue;
    criticalSect CSQ;
};
//-----------------------------------------------------------------------
#include "p_tqueue_simpleCS.hpp"
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
