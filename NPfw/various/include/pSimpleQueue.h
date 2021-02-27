//----------- pSimpleQueue.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef pSimpleQueue_H_
#define pSimpleQueue_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "PCrt_lck.h"
//----------------------------------------------------------------------------
template <class T>
class pSimpleQueue
{
  public:
    pSimpleQueue() : First(0), Last(0) {}
    ~pSimpleQueue() { while(Pop()); }

    void Push(T* d);
    T* Pop();

    bool hasData() { criticalLock CL(CS); return toBool(First); }

  private:
    criticalSect CS;
    struct List {
      List *Next;
      T* Data;

      List(T* d) : Next(0), Data(d) {}
      } *First, *Last;

    NO_COPY_COSTR_OPER(pSimpleQueue)
};
//----------------------------------------------------------------------------
template <class T>
void pSimpleQueue<T>::Push(T* d)
{
  criticalLock CL(CS);
  if(!Last)
    Last = First = new List(d);
  else {
    Last->Next = new List(d);
    Last = Last->Next;
    }
}
//----------------------------------------------------------------------------
template <class T>
T* pSimpleQueue<T>::Pop()
{
  criticalLock CL(CS);
  if(!First)
    return 0;
  List* tl = First;
  First = First->Next;

  T* t = tl->Data;
  delete tl;

  if(!First)
    Last = 0;
  return t;
}
//----------------------------------------------------------------------------
#endif
