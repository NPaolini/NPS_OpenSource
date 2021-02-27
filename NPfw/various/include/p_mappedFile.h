//-------------- P_MAPPEDFILE.H -----------------------------------------------
//-----------------------------------------------------------------------
#ifndef P_MAPPEDFILE_H_
#define P_MAPPEDFILE_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif

#include "setPack.h"
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
enum ACC_MAPPED_FILE { mP_RW, mP_READ_ONLY, mP_CREAT, mP_MEM_ONLY };
//-----------------------------------------------------------------------
// se aperto in sola lettura deve già essere stato aperto
// se creazione non deve essere già aperto.
// se creazione e non viene specificata una dimensione dà errore
//-----------------------------------------------------------------------
class p_MappedFile
{
  public:
    p_MappedFile(LPCTSTR n, ACC_MAPPED_FILE mode = mP_RW); // costruttore, nome e modo
    ~p_MappedFile();

    bool P_open(__int64 desiredSize = 0);

    LPCTSTR get_name();
    LPCTSTR get_namedView();

    LPVOID getAddr();
    bool flush();

    __int64 getDim();

    // pos deve essere un multiplo della granularità di allocazione
    // ricavabile tramite GetSystemInfo()
    bool remapAt(__int64 pos, DWORD len);

    // se si crea un file più grosso per sicurezza ma poi si vuol ridurlo
    // alle sue dimensioni effettive (solo se non aperto in mP_READ_ONLY)
    bool finalResize(__int64 new_dim);

  private:
    LPTSTR Name;        // pathname
    LPTSTR NamedView;
    HANDLE Handle;      // handle del file
    HANDLE HandleMapped; // handle del file mappato
    LPVOID Addr;
    // ci viene poi memorizzato il FILE_MAP_???
    DWORD Mode;      // modalità di apertura
    __int64 Len;

    bool P_close();
    bool resize(__int64 new_dim);
    // nessun costruttore di copia né operatore di assegnazione
    NO_COPY_COSTR_OPER(p_MappedFile)
};
//-----------------------------------------------------------------------
inline LPCTSTR p_MappedFile::get_name() { return Name; }
//-----------------------------------------------------------------------
//inline HANDLE p_MappedFile::get_handle() { return Handle; }
//-----------------------------------------------------------------------
inline LPVOID p_MappedFile::getAddr() { return Addr; }
//-----------------------------------------------------------------------
// classi template wrapper per l'utilizzo come semplici array
// esempio di uso:
/*  p_MappedFile pmf(nome);
    if(!pmf.P_open(4096))
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
class tWrapMappedFile
{

  public:
    tWrapMappedFile(p_MappedFile& pmf) : pMF(pmf) {}

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
    p_MappedFile& pMF;

    NO_COPY_COSTR_OPER(tWrapMappedFile)
};
//----------------------------------------------------------------------
template <class T>
const T& tWrapMappedFile<T>::operator[](DWORD pos) const
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
T& tWrapMappedFile<T>::operator[](DWORD pos)
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
