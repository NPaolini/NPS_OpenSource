//-------------- simple_helper.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef SIMPLE_HELPER_H_
#define SIMPLE_HELPER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <shlobj.h>
#include "setPack.h"
//----------------------------------------------------------------------------
// Semplice auto_ptr per COM
//----------------------------------------------------------------------------
template <class T>
class simple_com_ptr
{
  public:
    simple_com_ptr() : P(0) {}

    // per evitare crash in caso di creazione oggetto ma di non
    // inizializzazione COM occorre verificare la validità di P
    // prima di rilasciare l'oggetto COM
    ~simple_com_ptr() { if(P) P->Release(); }

    T* operator->()  const { return P;  }
    operator void**() { return (void**)&P; }
    operator T**() { return (T**)&P; }
  private:
    T* P;
    // non definiti per evitare assegnazioni non desiderate
    const simple_com_ptr& operator =(const simple_com_ptr&);
    simple_com_ptr(const simple_com_ptr&);
};
//----------------------------------------------------------------------------
// Semplice wrapper per incapsulare la gestione BSTR
//----------------------------------------------------------------------------
class simple_BSTR
{
  public:
    simple_BSTR(LPCWSTR name);
    simple_BSTR(LPCSTR name);
    ~simple_BSTR();
    operator const BSTR() const { return P; }
  private:
    BSTR P;
    // non definiti per evitare assegnazioni non desiderate
    const simple_BSTR& operator =(const simple_BSTR&);
    simple_BSTR(const simple_BSTR&);
};
//----------------------------------------------------------------------------
inline
simple_BSTR::simple_BSTR(LPCWSTR name) : P(SysAllocString(name))
{ }
//----------------------------------------------------------------------------
inline
simple_BSTR::~simple_BSTR()
{
  SysFreeString(P);
}
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif