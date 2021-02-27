//------------------ P_HEAP.H -------------------------------
//-----------------------------------------------------------
#ifndef P_HEAP_H_
#define P_HEAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
// l'elemento T è un oggetto
template <class T>
class P_Heap
{
  public:
    P_Heap(uint n = 0, uint N_Add = 10);
    virtual ~P_Heap();

  // crea da un vettore allocato, distrugge i dati precedenti, ridimensiona
    void Make(T *v, uint n);

   // estrae e ritorna la testa ed inserisce il nuovo valore,
   // non verifica se ci sono elementi
    T Swap(T new_elem);


     // estrae la testa, non verifica che sia valida
     // (se ci sono elementi)
    T Pop();

    // tornano false se errore
    bool Push(T new_elem);
    bool Pop(T& target);

    const T &Get() { return Vect[0]; } // ritorna la testa, senza estrarla

  // ritorna numero di elementi, da usare con pop e swap
    uint getNumElem() { return N_Elem; }

  protected:
    T *Vect;

    virtual bool Great(int i, int k) { return Vect[i] > Vect[k]; }

  private:
    uint N_Elem;   // numero di elementi nel vettore

  // numero massimo di elementi, se si inserisce un nuovo valore
  // e N_Elem è uguale a Max, prima Vect viene espanso e poi si inserisce
    uint Max;

    // numero di elementi da incrementare ogni volta
    uint N_Add;

    // ridimensiona in caso di crescita del vettore
    void Redim();

  // sposta l'elemento che si trova in testa nella sua giusta posizione
    void heapDown();

    void allocBuffer(uint n);

    NO_COPY_COSTR_OPER(P_Heap)
};
//-----------------------------------------------------------
// l'elemento T è un puntatore all'oggetto
template <class T>
class P_Heap_p : public P_Heap<T>
{
  public:
    P_Heap_p(uint n = 0, uint n_add = 10) : P_Heap<T>(n, n_add) { }

  protected:
    inline bool Great(int i, int k) { return *Vect[i] > *Vect[k]; }

  private:
    NO_COPY_COSTR_OPER(P_Heap_p)
};
//-----------------------------------------------------------
#include "p_heap.hpp" // codice funzioni template
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
