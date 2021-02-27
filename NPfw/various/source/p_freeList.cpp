//----------- p_freeList.cpp --------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "P_freeList.h"
//-----------------------------------------------------------------------------
P_freeList::~P_freeList()
{
  while(firstFree) {
    LPBYTE tmp = (LPBYTE)firstFree;
    firstFree = (LPVOID)*(__int64*)firstFree;
    delete []tmp;
    }
}
//-----------------------------------------------------------------------------
void P_freeList::addToFree(LPVOID data)
{
  *(__int64*)data = (__int64)firstFree;
  firstFree = data;
}
//-----------------------------------------------------------------------------
LPVOID P_freeList::getFree()
{
  LPVOID t;
  if(firstFree) {
    t = firstFree;
    firstFree = (LPVOID)*(__int64*)firstFree;
    }
  else
    t = (LPVOID) new BYTE[Size];

  return t;
}
