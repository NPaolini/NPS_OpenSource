//----------- P_FreePacketQueue.h --------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_packetQueue_H_
#define p_packetQueue_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_freeList.h"
#include "p_tqueue_simple.h"
#include "PCrt_lck.h"
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
class P_FreePacketQueue
{
  public:
    P_FreePacketQueue() : FreeList(sizeof(T)) {}
    ~P_FreePacketQueue();

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
    P_freeList FreeList;
    P_TQueueSimple<T*, maxPacket> Queue;
    criticalSect CSF;
    criticalSect CSQ;
};
//----------------------------------------------------------------------------
#include "P_FreePacketQueue.hpp"
//----------------------------------------------------------------------------
#endif
