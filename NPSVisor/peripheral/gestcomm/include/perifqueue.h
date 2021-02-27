//----------- Perifqueue.h ------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef PERIFQUEUE_H_
#define PERIFQUEUE_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "p_util.h"
#include "comgperif.h"
//-----------------------------------------------------------------------------
#define USE_TEMPLATE
//-----------------------------------------------------------------------------
#ifdef USE_TEMPLATE
class customData
#else
class customData  : public genericSet
#endif
{
    public:
      customData() : responce(NO), idReq(0)
      {
        U.next = 0;
      }
      // usato come puntatore al successivo nella lista libera
      union {
        DWORD type;
        customData* next;
        } U;

      addrToComm data;

      // codice di ritorno per il supervisore
      WORD responce;

      DWORD idReq;

      void Clear() {
        data.Clear();
        U.next = 0;
        responce = NO;
        idReq = 0;
        }
};
typedef customData* pCustomData;
//-----------------------------------------------------------------------------
#ifdef USE_TEMPLATE
// usa il template, maggiori performance in caso di oggetti non virtuali
// ma codice duplicato in caso di uso dello stesso contenitore per
// tipi diversi
//-----------------------------------------------------------------------------
#include "p_tqueue.h"
#define BASE_Q_CLASS P_TQueue<pCustomData>
//-----------------------------------------------------------------------------
#else
// maggior overhead dovuto all'eredità, ma un solo contenitore per
// più tipi di dato
//-----------------------------------------------------------------------------
#include "p_queue.h"
#define BASE_Q_CLASS P_Queue
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CustomQueue : public BASE_Q_CLASS
{
  public:
    CustomQueue() : BASE_Q_CLASS(true), firstFree(0) { }
    ~CustomQueue();
    // preleva il primo dato libero dalla lista libera o alloca memoria
    // se la lista è vuota
    pCustomData getFree();
    void addToFree(pCustomData data);

    bool pushBits(pCustomData data);

#ifdef USE_TEMPLATE
    pCustomData getCurrData() { return getCurr(); }
#else
    pCustomData getCurrData() {
      return dynamic_cast<pCustomData>(getCurr()); }
#endif
  protected:
    // puntatore alla lista libera
    pCustomData firstFree;

#ifdef USE_TEMPLATE
    typedef pCustomData pDataType;
#else
    typedef CntData pDataType;
#endif
    // anziché liberare memoria lo inserisce nella lista libera
    virtual void freeData(pDataType d);
    // se il dato è già nella coda non viene reinserito
    virtual bool isEqual(const pDataType newData, const pDataType listData);
};
//-----------------------------------------------------------------------------
#endif
