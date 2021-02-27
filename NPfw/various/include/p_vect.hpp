//----------------------- p_vect.hpp -----------------------------------
//----------------------------------------------------------------------
#include "P_Quick.h"
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::setDim(uint newdim)
{
  // necessario perché i confronti verificano se c'è posto per l'eventuale
  // movimento
  ++newdim;
  if(newdim <= Dim)
    return true;
  return resize(newdim - Dim);
}
//----------------------------------------------------------------------
template <class T>
uint PVect<T>::getIncr()
{
  if(Incr)
    return Incr;
  uint incr = Dim / 2;
  return max(incr, 10);
}
//----------------------------------------------------------------------
template <class T>
T& PVect<T>::operator[](uint pos)
{
  if(pos >= Dim) {
    uint n = pos - Dim;
    uint incr = getIncr();
    n /= incr;
    ++n;
    n *= incr;
    if(!resize(n))
      return Vect[Dim-1];
    }
  if(nElem <= pos)
    nElem = pos + 1;
  return Vect[pos];
}
//----------------------------------------------------------------------
template <class T>
const T& PVect<T>::operator[](uint pos) const
{
  return Vect[pos];
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::append(const PVect<T>& src)
{
  return append(src.getVect(), src.getElem());
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::copy(const PVect<T>& src, uint from)
{
  return copy(src.getVect(), from, src.getElem());
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::append(const T* buff, uint n_el)
{
  if(nElem + n_el + 1 >= Dim) {
    uint incr = getIncr();
    uint incr2 = n_el;
    if(incr2 < incr)
      incr2 = incr;
    if(!resize(incr2))
      return false;
    }
  for(uint i = 0; i < n_el; ++i, ++nElem)
    Vect[nElem] = buff[i];
  return true;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::copy(const T* buff, uint from, uint n_el)
{
  if(from > nElem)
    return false;
  if(from == nElem)
    return append(buff, n_el);

  if(nElem + n_el - from + 1 >= Dim) {
    uint incr = getIncr();
    uint incr2 = n_el - from;
    if(incr2 < incr)
      incr2 = incr;
    if(!resize(incr2))
      return false;
    }
  for(uint i = 0; i < n_el; ++i, ++from)
    Vect[from] = buff[i];
  if(nElem < from)
    nElem = from;

  return true;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::resize(uint incr)
{
  T *tmp = new T[Dim + incr];
  if(tmp) {
    if(Vect) {
      for(uint i = 0; i < nElem; ++i)
        tmp[i] = Vect[i];
      delete [] Vect;
      }
    Vect = tmp;
    Dim += incr;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------
template <class T>
T PVect<T>::remove(uint pos)
{
  // usa l'ultimo elemento non usato per memorizzare il valore da tornare
  if(nElem == Dim)
    resize(2);
  if(pos < nElem) {
    Vect[Dim - 1] = Vect[pos];
    --nElem;
    for(uint i = pos; i < nElem; ++i)
      Vect[i] = Vect[i + 1];
    }
  return Vect[Dim - 1];
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::remove(uint pos, T& target)
{
  if(pos < nElem) {
    target = Vect[pos];
    --nElem;
    for(uint i = pos; i < nElem; ++i)
      Vect[i] = Vect[i + 1];
    return true;
    }
  return false;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::remove(uint pos, uint elem, PVect<T>& target)
{
  if(pos >= nElem)
    return false;
  uint ixT = target.getElem();
  if(elem + pos >= nElem)
    elem = nElem - pos - 1;

  // trasferisce
  uint last = elem + pos;
  for(uint i = pos; i < last; ++i, ++ixT)
    target[ixT] = Vect[i];

  // posizione ai rimanenti da spostare
  ixT = last;

  // elementi rimasti a destra del buco
  uint right = nElem - ixT + pos;
  for(uint i = pos; i < right; ++i, ++ixT)
    Vect[i] = Vect[ixT];

  nElem -= elem;
  return true;
}
//----------------------------------------------------------------------
template <class T>
int PVect<T>::insertAlways(const T& val)
{
  if(!nElem) {
    (*this)[0] = val;
    return 0;
    }
  if(nElem + 1 >= Dim)
    if(!resize(getIncr()))
      return -1;
  ++nElem;
  uint pos;
  for(pos = nElem - 1; pos > 0; --pos) {
    if(Vect[pos - 1] < val)
      break;
    Vect[pos] = Vect[pos - 1];
    }
  Vect[pos] = val;
  return pos;
}
//----------------------------------------------------------------------
template <class T>
int PVect<T>::insertEx(const T& val)
{
  if(!nElem) {
    (*this)[0] = val;
    return 0;
    }
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    const T& d = Vect[mid];
    if(val < d)
      upp = mid - 1;
    else if(d < val)
      low = ++mid;
    else
      // già esiste
      return -1;
    }
  if(insert(val, mid))
    return mid;
  return -1;
}
//----------------------------------------------------------------------
template <class T>
int PVect<T>::insertAlways(const T& val, int (*cmp)(const T& v, const T& ins))
{
  if(!nElem) {
    (*this)[0] = val;
    return 0;
    }
  if(nElem + 1 >= Dim)
    if(!resize(getIncr()))
      return -1;
  ++nElem;
  uint pos = 0;
  for(pos = nElem - 1; pos > 0; --pos) {
    Vect[pos] = Vect[pos - 1];
    int res = cmp(val, Vect[pos]);
    if(res >= 0)
      break;
    }
  (*this)[pos] = val;
  return pos;
}
//----------------------------------------------------------------------
template <class T>
int PVect<T>::insertEx(const T& val, int (*cmp)(const T& v, const T& ins))
{
  if(!nElem) {
    (*this)[0] = val;
    return 0;
    }
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    const T& d = Vect[mid];
    int res = cmp(val, d);
    if(res < 0)
      upp = mid - 1;
    else if(res > 0)
      low = ++mid;
    else
      // già esiste
      return -1;
    }
  if(insert(val, mid))
    return mid;
  return -1;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::insert(const T& val, uint pos)
{
  if(nElem + 1 >= Dim)
    if(!resize(getIncr()))
      return false;
  ++nElem;
  for(uint i = nElem - 1; i > pos; --i)
    Vect[i] = Vect[i - 1];

  (*this)[pos] = val;
  return true;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::insert(const T& val)
{
  return insertEx(val) >= 0;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::find(const T& val, uint& pos, bool sorted) const
{
  if(!nElem)
    return false;
  if(sorted) {
    int upp = nElem - 1;
    int mid = 0;
    int low = 0;
    while(low <= upp) {
      mid = (upp + low) / 2;
      const T& d = Vect[mid];
      if(val < d)
        upp = mid - 1;
      else if(d < val)
        low = ++mid;
      else {
        pos = mid;
        return true;
        }
      }
    pos = mid;
    return false;
    }
  for(uint i = 0; i < nElem; ++i) {
    if(val <  Vect[i] || Vect[i] < val)
      continue;
    pos = i;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::find(const T& val, uint& pos, int (*cmp)(const T& v, const T& ins), bool sorted) const
{
  if(!nElem)
    return false;
  if(sorted) {
    int upp = nElem - 1;
    int mid = 0;
    int low = 0;
    while(low <= upp) {
      mid = (upp + low) / 2;
      const T& d = Vect[mid];
      int res = cmp(val, d);
      if(res < 0)
        upp = mid - 1;
      else if(res > 0)
        low = ++mid;
      else {
        pos = mid;
        return true;
        }
      }
    pos = mid;
    return false;
    }
  for(uint i = 0; i < nElem; ++i) {
    if(cmp(val, Vect[i]))
      continue;
    pos = i;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------
template <class T>
void PVect<T>::sort()
{
  P_Quick<T> q(nElem, Vect);
  q.run();
}
//----------------------------------------------------------------------
template <class T>
bool PVect<T>::setElem(uint newElem)
{
  if(newElem < Dim) {
    nElem = newElem;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
template <class T>
const PVect<T>& PVect<T>::operator=(const PVect<T>& v)
{
#define COMPARE_VECT
#ifdef COMPARE_VECT
  // use oveloading of &
  if(&v != &(*this)) {
#else
  // compare this
  if(v.getThis() != this) {
#endif
    int elem = v.getElem();
    Incr = v.Incr;
    setDim(elem);
    for(int i = 0; i < elem; ++i)
      Vect[i] = v.Vect[i];
    nElem = v.nElem;
    }
  return *this;
}
//----------------------------------------------------------------------
template <class T>
PVect<T>::PVect(const PVect& v) : Dim(0), Vect(0), nElem(0), Incr(v.Incr)
{
  int elem = v.getElem();
  setDim(elem);
  for(int i = 0; i < elem; ++i)
    Vect[i] = v.Vect[i];
  nElem = elem;
}
//----------------------------------------------------------------------
template <class T>
void  PVect<T>::setIncr(uint newIncr)
{
  if(!newIncr && Incr) {
    newIncr = nElem / 4;
    if(newIncr <= Incr)
      return;
    }
  Incr = newIncr;
}
//----------------------------------------------------------------------
