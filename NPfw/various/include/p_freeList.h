//----------- p_freeList.h ----------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef P_freeList_H_
#define P_freeList_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "p_util.h"
//-----------------------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------------------
#define MIN_SIZE_FREELIST sizeof(__int64)
//-----------------------------------------------------------------------------
class P_freeList
{
  public:
    P_freeList(size_t size) : firstFree(0), Size(max(size, MIN_SIZE_FREELIST)) { }
    ~P_freeList();
    // preleva il primo dato libero dalla lista libera o alloca memoria
    // se la lista è vuota
    LPVOID getFree();
    void addToFree(LPVOID data);

  private:
    // puntatore alla lista libera
    LPVOID firstFree;
    size_t Size;

    NO_COPY_COSTR_OPER(P_freeList)
};
//-----------------------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------------------------
#endif
