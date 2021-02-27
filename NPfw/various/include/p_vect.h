//--------------- p_Vect.h ---------------------------------------
#ifndef P_VECT_H_
#define P_VECT_H_
//----------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------
template <class T>
class PVect
{
  public:
    PVect(int dim_incr = 0) : Incr(dim_incr), Vect(0),
          Dim(0), nElem(0) {  }
    ~PVect() { delete []Vect; }

    // se si conosce già il numero degli elementi da inserire è più
    // conveniente allocare in una sola volta tutto il buffer
    bool setDim(uint n_elem);

    // se newIncr == 0 porta l'incremento ad un quarto del numero di elementi
    // solo se superiore all'incremento corrente, se diverso da zero usa quello
    void setIncr(uint newIncr = 0);

    T& operator [](uint pos);
    const T& operator [](uint pos) const;
    const T* operator &() const { return Vect; }
    const T* operator &() { return Vect; }

    uint getDim() const { return Dim; }
    uint getElem() const { return nElem; }
    void reset() { nElem = 0; }

    bool append(const T* buff, uint n_el);
    bool copy(const T* buff, uint from, uint n_el);

    bool append(const PVect<T>& src);
    bool copy(const PVect<T>& src, uint from);

    // T dovrebbe avere un costruttore di default che lo
    // inizializzi a zero, la remove ne crea uno temporaneo in cui
    // copia quello prelevato dall'array se pos non eccede la capacità
    // altrimenti il temporaneo rimane non inizializzato
    T remove(uint pos);

    // per compatibilità è stata mantenuta la vecchia versione,
    // questa torna la posizione all'interno del vettore o -1 se
    // il valore è già presente
    int insertEx(const T& val);

    // permette di specificare la routine di confronto per l'inserimento ordinato
    int insertEx(const T& val, int (*cmp)(const T& v, const T& ins));
    bool find(const T& val, uint& pos, int (*cmp)(const T& v, const T& ins), bool sorted = true) const;


    // aggiungono sempre, anche se già presente
    int insertAlways(const T& val);
    int insertAlways(const T& val, int (*cmp)(const T& v, const T& ins));

    // si può usare questa se si vuole verificare il risultato
    bool remove(uint pos, T& target);

    bool remove(uint pos, uint elem, PVect<T>& target);

    bool insert(const T& val, uint pos);

    // inserimento ordinato, il vettore deve essere già ordinato.
    // L'oggetto deve fornire l'operatore <
    bool insert(const T& val);
    // se ordinato usa la ricerca binaria, altrimenti quella lineare
    bool find(const T& val, uint& pos, bool sorted = true) const;

    // se newElem eccede la dimensione attuale torna false
    bool setElem(uint newElem);
    PVect(const PVect&);
    const PVect& operator=(const PVect&);

    const T* getVect() const { return Vect; }
    const T* getVect() { return Vect; }

    const PVect<T>* getThis() const { return this; }
    PVect<T>* getThis() { return this; }

    // L'oggetto deve fornire l'operatore <
    void sort();
  private:
    uint Dim;    // dimensione del vettore
    uint Incr;   // incremento di default se la richiesta va fuori della dimensione
    uint nElem;  // numero di elementi, equivale alla più alta posizione inserita
    T* Vect;
    bool resize(uint incr);
    uint getIncr();
};
//----------------------------------------------------------------------
#include "p_vect.hpp"
//----------------------------------------------------------------------
// da usarsi solo in caso di vettore di puntatori
//----------------------------------------------------------------------
template <class T>
void flushPV(PVect<T>& v)
{
  int nElem = v.getElem();
  for(int i = 0; i < nElem; ++i) {
    T& tmp = v[i];
    delete tmp;
    }
  v.reset();
}
//----------------------------------------------------------------------
// da usarsi solo in caso di vettore di puntatori ad array( es. PVect<LPCTSTR>)
//----------------------------------------------------------------------
template <class T>
void flushPAV(PVect<T>& v)
{
  int nElem = v.getElem();
  for(int i = 0; i < nElem; ++i) {
    T& tmp = v[i];
    delete []tmp;
    }
  v.reset();
}
//----------------------------------------------------------------------
// per eseguire una funzione sull'insieme degli elementi
//----------------------------------------------------------------------
template<class T, class Pred>
void for_each(const PVect<T>& t, Pred op)
{
  uint nElem = t.getElem();
  for(uint i = 0; i < nElem; ++i)
    op(t[i]);
};
/* ----------------------------------------------------------------------
esempio per class Pred di stampa su PVect<LPCTSTR>
struct print
{
  template< class T >
  void operator()( const T& v )  {
    _tprintf(_T("%s "), v);
    }
};
oppure (specializzata)
struct print
{
  void operator()(const LPCTSTR& v )  {
    _tprintf(_T("%s "), v);
    }
};

richiamata così

  PVect<LPCTSTR> test;
  ...
  for_each(test, print());
-----------------------
oppure con argomento
struct print
{
  print(const Obj& obj) : refObj(obj) {}
  void operator()(const LPCTSTR& v )  {
    _tprintf(_T("%s %s"), v, refObj.foo());
    }
  const Obj& refObj;
};

richiamata così

  PVect<LPCTSTR> test;
  Obj obj;
  ...
  for_each(test, print(obj));

*/
//----------------------------------------------------------------------
// in questa passa alla funzione anche l'indice su cui si trova
//----------------------------------------------------------------------
template<class T, class Pred>
void for_each_ix(const PVect<T>& t, Pred op)
{
  uint nElem = t.getElem();
  for(uint i = 0; i < nElem; ++i)
    op(t[i], i);
};
/*
  esempio
struct print
{
  template< class T >
  void operator()( const T& v, uint ix )  {
    _tprintf(_T("%s[%d] "), v, ix);
    }
};

*/
//----------------------------------------------------------------------
#include "restorePack.h"

#endif
