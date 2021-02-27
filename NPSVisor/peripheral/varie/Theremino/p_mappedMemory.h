//-------------- p_MappedMemory.h -----------------------------------------------
//-----------------------------------------------------------------------
#ifndef p_MappedMemory_h_
#define p_MappedMemory_h_
//-----------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif

#include "setPack.h"
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
class p_MappedMemory
{
  public:
    p_MappedMemory();
    ~p_MappedMemory();

    bool P_open(LPCTSTR n, __int64 desiredSize = 2048);

    LPVOID getAddr();

    __int64 getDim();

    // pos deve essere un multiplo della granularità di allocazione
    // ricavabile tramite GetSystemInfo()
    bool remapAt(__int64 pos, DWORD len);

  private:
    HANDLE HandleMapped; // handle del file mappato
    LPVOID Addr;
    __int64 Len;

    bool P_close();
    // nessun costruttore di copia né operatore di assegnazione
    NO_COPY_COSTR_OPER(p_MappedMemory)
};
//-----------------------------------------------------------------------
inline LPVOID p_MappedMemory::getAddr() { return Addr; }
//-----------------------------------------------------------------------
// classi template wrapper per l'utilizzo come semplici array
// esempio di uso:
/*  p_MappedMemory pmf;
    if(!pmf.P_open(nome, 4096))
      return false;

    tWrapMappedFile<DWORD> wDWmf(pmf);
#ifdef USE_INDEXED_POINTER
    for(int i = 0; i < 10; ++i)
      wDWmf[i] = i + 1;
#else
    for(int i = 0, j = 0; i < 10; ++i, j += sizeof(DWORD))
      wDWmf[j] = i + 1;
#endif
    ecc.
*/
//-----------------------------------------------------------------------
template <class T>
class tWrapMappedMemory
{

  public:
    tWrapMappedMemory(p_MappedMemory& pmf) : pMF(pmf) {}

#define USE_INDEXED_POINTER
#ifdef USE_INDEXED_POINTER
    // pos deve essere già allineato al tipo parametrizzato
    // in modo che pos + 1 punti al successivo valore
#else
    // pos è la posizione in byte, la posizione effettiva viene
    // calcolata al momento dell'acquisizione, es. in DWORD la posizione
    // successiva è pos + 4
#endif
    T& operator [](DWORD pos);
    const T& operator [](DWORD pos) const;

  private:
    p_MappedMemory& pMF;

    NO_COPY_COSTR_OPER(tWrapMappedMemory)
};
//----------------------------------------------------------------------
template <class T>
const T& tWrapMappedMemory<T>::operator[](DWORD pos) const
{
#ifdef USE_INDEXED_POINTER
  return ((T*)pMF.getAddr())[pos];
#else
  LPBYTE pb = (LPBYTE)pMF.getAddr();
  pb += pos;
  return *((T*)pb);
#endif
}
//----------------------------------------------------------------------
template <class T>
T& tWrapMappedMemory<T>::operator[](DWORD pos)
{
#ifdef USE_INDEXED_POINTER
  return ((T*)pMF.getAddr())[pos];
#else
  LPBYTE pb = (LPBYTE)pMF.getAddr();
  pb += pos;
  return *((T*)pb);
#endif
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
