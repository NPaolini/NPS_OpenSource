//--------------- P_QUEUE.H ---------------------------------------------
#ifndef P_QUEUE_H_
#define P_QUEUE_H_
//-----------------------------------------------------------------------
#ifndef P_UTIL_H_
  #include "p_util.h"
#endif

typedef genericSet* CntData;
#include "setPack.h"
//-----------------------------------------------------------------------
class P_Queue
{
  public:
    P_Queue(bool autodelete = false);
    virtual ~P_Queue() { Flush(); }

    // verifica se il dato è già presente
//    virtual
    bool Push(CntData d);

    //  aggiunge un dato in testa, ritorna 1 ->ok
    // non verifica la presenza
//    virtual
    bool atTop(CntData);

    // rimuove il nodo corrente
//    virtual
    bool Pop();

    // ritorna il dato corrente
//    virtual
    CntData getCurr();

    void Flush();

  protected:

    bool AutoDelete;

    struct List {
      CntData L;        // puntatore buffer dati
      List *Next;       // link in avanti
      } *First;  // puntatore al primo

    // richiamata se AutoDelete == true, viene usata solo nella Pop().
    // Può essere ridefinita se anziché eliminare il dato debba essere
    // spostato da qualche altra parte
    virtual void freeData(CntData d);

    // funzione di comparazione per evitare duplicati,
    // per default accetta tutto
    virtual bool isEqual(const CntData newData, const CntData listData) { return false; }

    NO_COPY_COSTR_OPER(P_Queue)
};
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
