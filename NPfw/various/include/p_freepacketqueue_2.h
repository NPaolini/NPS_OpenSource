//----------- P_FreePacketQueue_2.h --------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_packetQueue_2_H_
#define p_packetQueue_2_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_FreeListCS.h"
#include "p_tqueue_simpleCS.h"
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
class P_FreePacketQueue_2
{
  public:
    P_FreePacketQueue_2(P_FreeListCS<T>& freeList, P_TQueueSimpleCS<T*, maxPacket>& queue) : FreeList(freeList), Queue(queue) {}
    ~P_FreePacketQueue_2() {}

    // gestione pacchetti liberi
    T* get();
    void release(T* f);

    // gestione coda
    void push(T* f);
    T* retrieve();
    T* retrieveNoRemove();

    int getStored();
    int getFree();

    // copia (appende) tutta la coda in set, torna il numero di elementi aggiunti
    int copyTo(PVect<T*>& set);

    // se occorresse effettuare più operazioni atomiche sulla coda
    criticalSect& getCS_Queue() { return CSQ; }
  private:
    P_FreeListCS<T>& FreeList;
    P_TQueueSimpleCS<T*, maxPacket>& Queue;
};
//----------------------------------------------------------------------------
#include "P_FreePacketQueue_2.hpp"
//----------------------------------------------------------------------------
#endif
