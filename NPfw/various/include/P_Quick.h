//----------- P_Quick.h ------------------------------------------
#ifndef P_Quick_H_
#define P_Quick_H_
//----------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------
template <class T>
class P_Quick
{
  public:
    P_Quick(long nelem, T* vect) : Vect(vect), nElem(nelem) { }
    virtual ~P_Quick() {}
    void run() { if(nElem > 0) myquicksort(0, nElem - 1); }
  protected:
    T* Vect;
    long nElem;
    // L'oggetto deve fornire l'operatore <
    virtual void myquicksort(long left, long right);
};
//----------------------------------------------------------------
#define MIN_INSER 6
//----------------------------------------------------------------
template <class T>
void P_Quick<T>::myquicksort(long left, long right)
{
  if(right - left < MIN_INSER) { // ordinamento per inserzione
    for(long i = left + 1; i <= right; ++i) {
      T Tmp = Vect[i];
      long j = i;
      for(; j > left; --j)
        if(Tmp < Vect[j - 1])
          Vect[j] = Vect[j - 1];
        else
          break;
      if(i != j)
        Vect[j] = Tmp;
      }
    return;
    }
  long i = (left + right) >> 1;
  T Tmp = Vect[i];
  i = left;
  long j = right;
  do {
    while(Vect[i] < Tmp && i < right)
      ++i;
    while(Tmp < Vect[j] && j > left)
      --j;
    if(i <= j) {
      T t = Vect[i];
      Vect[i] = Vect[j];
      Vect[j] = t;
      ++i;
      --j;
      }
    } while (i <= j);
  if(left < j)
    myquicksort(left, j);
  if(i < right)
    myquicksort(i, right);
}
//----------------------------------------------------------------
#endif
