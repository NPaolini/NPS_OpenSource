//----------- P_SimpleFreePacket.h --------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_SimpleFreePacket_H_
#define P_SimpleFreePacket_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_freeList.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <typename T, size_t tsize = sizeof(T)>
class P_SimpleFreePacket
{
  public:
    P_SimpleFreePacket() : FreeList(tsize) {}
    ~P_SimpleFreePacket() {}

    T* get();
    void release(T* f);

  private:
    P_freeList FreeList;
};
//----------------------------------------------------------------------------
template <typename T, size_t tsize>
void P_SimpleFreePacket<T, tsize>::release(T* f)
{
  if(!f)
    return;
  FreeList.addToFree(f);
}
//----------------------------------------------------------------------------
template <typename T, size_t tsize>
T* P_SimpleFreePacket<T, tsize>::get()
{
  T* result = (T*)FreeList.getFree();
  return result;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <typename T>
void defFzClear(T* packet, size_t tsize)
{
  ZeroMemory(packet, tsize);
}
//----------------------------------------------------------------------------
template <typename T, void fz_clear(T*, size_t sz) = defFzClear, size_t tsize = sizeof(T) >
class P_SimpleFreePacketClear
{
  public:
    P_SimpleFreePacketClear() : FreeList(tsize) {}
    ~P_SimpleFreePacketClear() {}

    T* get();
    void release(T* f);

  private:
    P_freeList FreeList;
};
//----------------------------------------------------------------------------
template <typename T, void fz_clear(T*, size_t sz), size_t tsize >
void P_SimpleFreePacketClear<T, fz_clear, tsize>::release(T* f)
{
  if(!f)
    return;
  FreeList.addToFree(f);
}
//----------------------------------------------------------------------------
template <typename T, void fz_clear(T*, size_t sz), size_t tsize >
T* P_SimpleFreePacketClear<T, fz_clear, tsize>::get()
{
  T* result = (T*)FreeList.getFree();
  fz_clear(result, tsize);
  return result;
}
//----------------------------------------------------------------------------
#endif
