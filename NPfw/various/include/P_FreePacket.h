//----------- P_FreePacket.h --------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_FreePacket_H_
#define P_FreePacket_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_freeList.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <typename T>
class P_FreePacket
{
  private:
    struct packet
    {
      packet* next;
      T* pack;
    };

  public:
    P_FreePacket() : FreeList(sizeof(packet)), First(0) {}
    ~P_FreePacket();

    T* get();
    void release(T* f);

  private:
    packet* First;
    P_freeList FreeList;

    NO_COPY_COSTR_OPER(P_FreePacket)
};
//----------------------------------------------------------------------------
template <typename T>
P_FreePacket<T>::~P_FreePacket()
{
  while(First) {
    packet* pk = First;
    First = pk->next;
    delete pk->pack;
    }
}
//----------------------------------------------------------------------------
template <typename T>
void P_FreePacket<T>::release(T* f)
{
  if(!f)
    return;
  packet* pk = (packet*)FreeList.getFree();
  pk->next = First;
  First = pk;
  First->pack = f;
}
//----------------------------------------------------------------------------
template <typename T>
T* P_FreePacket<T>::get()
{
  if(First) {
    packet* pk = First;
    First = pk->next;
    T* result = pk->pack;
    FreeList.addToFree(pack);
    return result;
    }
  T* result = new T();
  return result;
}
//----------------------------------------------------------------------------
#endif
