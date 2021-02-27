//--------------- P_TQUEUE.H --------------------------------------------
#ifndef P_TQUEUE_H_
#define P_TQUEUE_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
#include "p_freeList.h"
#include "setPack.h"
//-----------------------------------------------------------------------
template <class T, bool isArray = false>
class P_TQueue
{
  public:
    P_TQueue(bool autodelete = false);
    virtual ~P_TQueue() { Flush(); }

    // verifica la presenza e non lo aggiunge se c'è già
    bool Push(T d);

    // aggiungono sempre
    bool PushAlways(T d);
    //  aggiunge un dato in testa
    bool atTop(T d);

    // rimuove il nodo corrente
    bool Pop();

    // ritorna il dato corrente
    T getCurr();

    void Flush();

    uint getElem() const;

    bool getFirst(LPVOID& curr, T& value);
    // ritorna il dato corrente ed il puntatore al successivo di curr se torna false il value non è valido
    bool getNext(LPVOID& curr, T& value);

  protected:

    bool AutoDelete;

    struct List {
      List *Next;  // link in avanti
      T L;         // dati
      } *First, *Last;  // puntatore al primo e ultimo

    // richiamata se AutoDelete == true, viene usata solo nella Pop().
    // Può essere ridefinita se anziché eliminare il dato debba essere
    // spostato da qualche altra parte
    virtual void freeData(T d) { if(isArray) delete []d; else delete d; }

    // funzione di comparazione per evitare duplicati,
    // per default accetta tutto
    virtual bool isEqual(const T newData, const T listData) { return false; }

    P_freeList FreeList;
    NO_COPY_COSTR_OPER(P_TQueue)
};
//-----------------------------------------------------------------------
#include "p_tqueue.hpp"
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
