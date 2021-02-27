//------------- P_LIST.H --------------------------------------
#ifndef P_LIST_H_
#define P_LIST_H_
//-------------------------------------------------------------
#include "precHeader.h"
#include <string.h>
//-------------------------------------------------------------
#ifndef P_UTIL_H_
  #include "p_util.h"
#endif
#ifndef PDEF_H_
  #include "pDef.h"
#endif
#include "setPack.h"
//-------------------------------------------------------------
typedef genericSet* ListData;
//-------------------------------------------------------------
class IMPORT_EXPORT P_List_Gen
{
  public:
    P_List_Gen(bool autodelete = false);
    virtual ~P_List_Gen();

    virtual bool Add(ListData);  //  aggiunge un dato in testa
    virtual bool Push(ListData d);
    virtual bool atEnd(ListData);//  aggiunge un dato in coda

    virtual bool insert(ListData, bool before); // inserisce nel punto corrente

    virtual ListData Rem();   // rimuove il nodo corrente, ritorna il dato
    virtual ListData Pop();

    virtual ListData getFirst(); // ritorna il primo dato se la lista non è vuota
    virtual ListData getCurr();  // ritorna il dato corrente

    virtual bool Search(ListData);

    bool levelUp();         // sposta il nodo corrente di un livello
    bool levelDown();

    // vuota la lista
    void Flush();

    bool setFirst();   // setta il corrente al primo
    bool setLast();    // setta il corrente all'ultimo
    bool setNext();    // al seguente
    bool setPrev();    // al precedente


    struct List {
      ListData L;        // puntatore buffer dati
      List *Next;       // link in avanti
      List *Prev;       // e indietro
      };

    typedef P_List_Gen::List* pList;

    virtual bool insert(ListData toAdd, bool before, pList nearTo);

    // per non usare lo stato interno e quindi poter essere usata in più punti contemporaneamente
    pList getFirstL();
    pList getLastL();
    pList getNext(pList curr);
    pList getPrev(pList curr);

    // da usare con cautela, specialmente la seconda
    pList getCurrL();
    void setCurrL(pList curr);

  protected:
    List *First, *Curr;  // puntatori al primo e al corrente

    bool AutoDelete;

    // richiamata se AutoDelete == true
    // se viene ridefinita occorre richiamare il metodo Flush()
    // nel dtor della classe derivata
    virtual void freeData(ListData d);

    virtual int Cmp(ListData a, ListData b);

    NO_COPY_COSTR_OPER(P_List_Gen)
};
//-------------------------------------------------------------
inline P_List_Gen::P_List_Gen(bool autodelete) :
  AutoDelete(autodelete), First(0), Curr(0)  {  }
//-------------------------------------------------------------
inline P_List_Gen::~P_List_Gen() { Flush(); }
//-------------------------------------------------------------
inline bool P_List_Gen::Push(ListData d) { return Add(d); }
//-------------------------------------------------------------
inline ListData P_List_Gen::Pop() { return Rem(); }
//-------------------------------------------------------------
inline ListData P_List_Gen::getFirst()
{
  return First ? First->L : 0;
}
//-------------------------------------------------------------
inline ListData P_List_Gen::getCurr()
{
  return Curr ? Curr->L : 0;
}
//-------------------------------------------------------------
inline void P_List_Gen::freeData(ListData d) { delete d; }
//-------------------------------------------------------------
inline int P_List_Gen::Cmp(ListData a, ListData b)
{
  return _tcsicmp((LPCTSTR)a, (LPCTSTR)b);
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
class IMPORT_EXPORT P_List_Stack : public P_List_Gen // LIFO
{
  public:
    P_List_Stack(bool autodelete = false);
    virtual ListData Pop();
};
//-------------------------------------------------------------
inline P_List_Stack::P_List_Stack(bool autodelete) :
    P_List_Gen(autodelete) { }
//-------------------------------------------------------------
inline ListData P_List_Stack::Pop()
{
  setFirst();
  return Rem();
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
class IMPORT_EXPORT P_List_Queue : public P_List_Gen // FIFO
{
  public:
    P_List_Queue(bool autodelete = false);
    virtual bool Push(ListData f);
    virtual ListData Pop();
  private:
    // non deve essere richiamata direttamente
    virtual bool Add(ListData);
};
//-------------------------------------------------------------
inline P_List_Queue::P_List_Queue(bool autodelete) :
    P_List_Gen(autodelete) { }
//-------------------------------------------------------------
inline bool P_List_Queue::Push(ListData f) { return Add(f); }
//-------------------------------------------------------------
inline ListData P_List_Queue::Pop()  { setFirst(); return Rem(); }
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
class IMPORT_EXPORT P_List_Ord :  public P_List_Gen // Ordinata
{
  public:
    P_List_Ord(bool autodelete = false);
    virtual bool Push(ListData f);
    virtual ListData Pop();

    virtual bool Search(ListData);

  private:
    int Where;
    bool Add(ListData);
    void search4add(ListData);  // ricerca per inserimento dato, distrugge Curr
};
//-------------------------------------------------------------
inline P_List_Ord::P_List_Ord(bool autodelete) : P_List_Gen(autodelete) { }
//-------------------------------------------------------------
inline bool P_List_Ord::Push(ListData f) { return Add(f); }
//-------------------------------------------------------------
inline ListData P_List_Ord::Pop()   {  return Rem(); }
//-------------------------------------------------------------
#include "restorePack.h"
//-------------------------------------------------------------
#endif
