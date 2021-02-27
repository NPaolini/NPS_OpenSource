//----------- P_FreeListCS.h --------------------------------------------
//-------------------------------------------------------------------------
#ifndef P_FreePacketCS_H_
#define P_FreePacketCS_H_
//-------------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------------
#include "p_freeList.h"
#include "PCrt_lck.h"
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
template <typename T>
class P_FreeListCS
{
  public:
    P_FreeListCS() : FreeList(sizeof(T)) {}
    ~P_FreeListCS() {}

    T* getFree();
    void addToFree(T* data);

  private:
    P_freeList FreeList;
    criticalSect CSF;

    NO_COPY_COSTR_OPER(P_FreeListCS)
};
//-------------------------------------------------------------------------
template <typename T>
void P_FreeListCS<T>::addToFree(T* f)
{
  if(!f)
    return;
  criticalLock CL(CSF);
  FreeList.addToFree(f);
}
//-------------------------------------------------------------------------
template <typename T>
T* P_FreeListCS<T>::getFree()
{
  criticalLock CL(CSF);
  T* f = (T*)FreeList.getFree();
  ZeroMemory(f, sizeof(*f));
  return f;
}
//-------------------------------------------------------------------------
#endif
