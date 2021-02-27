//----- p_util.h -------------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_UTIL_H
#define P_UTIL_H
//----------------------------------------------------------------------------
#include "precHeader.h"

#ifndef PDEF_H_
  #include "pDef.h"
#endif
#ifndef DEFIN_H_
  #include "Defin.h"
#endif
//----------------------------------------------------------------------------
#include "p_param_v.h"
#include "p_crypt.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
// classe generica da cui derivare per usare contenitori non template
class genericSet
{
  public:
    virtual ~genericSet() {}
  protected:
    genericSet() {}
};
//----------------------------------------------------------------------------
LPWSTR IMPORT_EXPORT str_newdup(LPCWSTR str);
LPSTR IMPORT_EXPORT str_newdup(LPCSTR str);
//----------------------------------------------------------------------------
// trasforma la stringa ANSI in UNICODE. len è in/out, se == zero, buff viene
// interpretato come stringa NULL terminated, altrimenti lo usa.
LPWSTR makeWCHAR(uint& len, LPCSTR buff, uint cp=CP_ACP);
// non alloca memoria ma usa il buffer passato
LPWSTR makeWCHAR(LPWSTR target, uint lenT, LPCSTR source, uint lenS, uint cp=CP_ACP);
//----------------------------------------------------------------------------
// trasforma la stringa UNICODE in ANSI. len è in/out, se == zero, buff viene
// interpretato come stringa NULL terminated, altrimenti lo usa.
LPSTR unmakeWCHAR(uint& len, LPCWSTR buff, uint cp=CP_ACP);
// non alloca memoria ma usa il buffer passato
LPSTR unmakeWCHAR(LPSTR target, uint lenT, LPCWSTR source, uint lenS, uint cp=CP_ACP);
//----------------------------------------------------------------------------
// trasforma il buffer passato verificando l'uso dell'unicode
// se è necessaria la conversione, esegue il delete di buff, altrimenti
// copia semplicemente l'indirizzo. Torna null in caso di errore.
LPSTR autoConvertA(LPVOID buff, uint& len, uint cp=CP_ACP);
LPWSTR autoConvertW(LPVOID buff, uint& len, uint cp=CP_ACP);
//----------------------------------------------------------------------------
// apre il file, ne converte il contenuto nel formato in uso e ne torna il buffer
// allocato. In dim ne memorizza la dimensione in numero di caratteri
LPSTR openFileA(LPCTSTR file, uint& dim);
LPWSTR openFileW(LPCTSTR file, uint& dim);
//----------------------------------------------------------------------------
#if defined(UNICODE) || defined(_UNICODE)
  #define autoConvert autoConvertW
  #define openFile openFileW
#else
  #define autoConvert autoConvertA
  #define openFile openFileA
#endif
//----------------------------------------------------------------------------
#if defined(UNICODE) || defined(_UNICODE)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
  template <class T>
  T* fillStr(T* str, int car, size_t len)
  {
    for(size_t i = 0; i < len; ++i)
      str[i] = car;
    return str;
  }
//----------------------------------------------------------------------------
  template <class S, class T>
  T* copyStr(T* target, const S* source, size_t len)
  {
    for(size_t i = 0; i < len; ++i)
      target[i] = (T)source[i];
    return target;
  }
//----------------------------------------------------------------------------
  template <class S, class T>
  int cmpStr(const S* str1, const T* str2, size_t len)
  {
    for(size_t i = 0; i < len; ++i) {
      int t = (int)((uint)str1[i] - (uint)str2[i]);
      if(t < 0)
        return -1;
      else if(t > 0)
        return 1;
      }
    return 0;
  }
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
  #define fillStr memset
  #define copyStr memcpy
  #define cmpStr  memcmp
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
template <class S, class T>
T* copyStrZ(T* target, const S* source, size_t len)
{
  for(size_t i = 0; i < len; ++i)
    if(!(target[i] = (T)source[i]))
      break;
  target[len] = 0;
  return target;
}
//----------------------------------------------------------------------------
template <class S, class T>
T* copyStrZ(T* target, const S* source)
{
  T* p = target;
  while(*source)
    *target++ =  (T)*source++;
  *target = 0;
  return p;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// converte e carica in buff il valore togliendo i decimali
void makeTruncString(REALDATA val, LPTSTR buff, size_t lenBuff);
void makeTruncStringU(REALDATA val, LPTSTR buff, size_t lenBuff);
//----------------------------------------------------------------------------
// converte e carica in buff il valore fissando i decimali
void makeFixedString(REALDATA val, LPTSTR buff, size_t lenBuff, unsigned dec = 2);
void makeFixedStringU(REALDATA val, LPTSTR buff, size_t lenBuff, unsigned dec = 2);
//----------------------------------------------------------------------------
void zeroTrim(LPTSTR t);
//----------------------------------------------------------------------------
LPSTR trim(LPSTR buff);
LPSTR lTrim(LPSTR buff);
LPWSTR trim(LPWSTR buff);
LPWSTR lTrim(LPWSTR buff);
//----------------------------------------------------------------------------
LPSTR  strAlign(LPSTR t, LPCSTR s, int len, bool left = true, bool zero = true);
LPWSTR strAlign(LPWSTR t, LPCWSTR s, int len, bool left = true, bool zero = true);

#define leftAlign(a, b, c) strAlign((a), (b), (c), true, false)
#define leftAlign_Z(a, b, c) strAlign((a), (b), (c), true, true)
#define rightAlign(a, b, c) strAlign((a), (b), (c), false, false)
#define rightAlign_Z(a, b, c) strAlign((a), (b), (c), false, true)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define ROUND_REAL(a) (long)(DWORD)((a) - ((a) < 0) + 0.5)
#define ROUND_REAL_64(a) (__int64)((a) - ((a) < 0) + 0.5)

#define ROUND_POS_REAL(a) (long)(DWORD)((a) + 0.5)
//--------------------------------------------------------------------
#define MK_I64(ft)  ( (reinterpret_cast<LARGE_INTEGER*> (&ft) )->QuadPart)
#define cMK_I64(ft) ( (reinterpret_cast<const LARGE_INTEGER*> (&ft) )->QuadPart)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
inline bool operator >(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) > cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator <(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) < cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator >=(const FILETIME& ft1, const FILETIME& ft2)
{
  return !(ft1 < ft2);
}
//--------------------------------------------------------------------
inline bool operator <=(const FILETIME& ft1, const FILETIME& ft2)
{
  return !(ft1 > ft2);
}
//--------------------------------------------------------------------
inline bool operator ==(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) == cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator !=(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) != cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator >(const FILETIME& ft1, __int64 ft2)
{
  return cMK_I64(ft1) > ft2;
}
//--------------------------------------------------------------------
inline bool operator <(const FILETIME& ft1, __int64 ft2)
{
  return cMK_I64(ft1) < ft2;
}
//--------------------------------------------------------------------
inline bool operator >=(const FILETIME& ft1, __int64 ft2)
{
  return !(ft1 < ft2);
}
//--------------------------------------------------------------------
inline bool operator <=(const FILETIME& ft1, __int64 ft2)
{
  return !(ft1 > ft2);
}
//--------------------------------------------------------------------
inline bool operator ==(const FILETIME& ft1, __int64 ft2)
{
  return cMK_I64(ft1) == ft2;
}
//--------------------------------------------------------------------
inline bool operator >(__int64 ft1, const FILETIME& ft2)
{
  return ft1 > cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator <(__int64 ft1, const FILETIME& ft2)
{
  return ft1 < cMK_I64(ft2);
}
//--------------------------------------------------------------------
inline bool operator >=(__int64 ft1, const FILETIME& ft2)
{
  return !(ft1 < ft2);
}
//--------------------------------------------------------------------
inline bool operator <=(__int64 ft1, const FILETIME& ft2)
{
  return !(ft1 > ft2);
}
//--------------------------------------------------------------------
inline bool operator ==(__int64 ft1, const FILETIME& ft2)
{
  return ft1 == cMK_I64(ft2);
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
#define I64_TO_FT(val) i64ToFT(val)
//--------------------------------------------------------------------
inline FILETIME  i64ToFT(__int64 val)
{
  FILETIME ft;
  MK_I64(ft) = val;
  return ft;
}
//--------------------------------------------------------------------
inline FILETIME operator +(const FILETIME& ft1, const FILETIME& ft2)
{
  return I64_TO_FT( cMK_I64(ft1) + cMK_I64(ft2) );
}
//--------------------------------------------------------------------
inline FILETIME operator +=(FILETIME& ft1, const FILETIME& ft2)
{
  return I64_TO_FT( MK_I64(ft1) += cMK_I64(ft2) );
}
//--------------------------------------------------------------------
inline FILETIME operator -(const FILETIME& ft1, const FILETIME& ft2)
{
  return I64_TO_FT( cMK_I64(ft1) - cMK_I64(ft2) );
}
//--------------------------------------------------------------------
inline FILETIME operator -=(FILETIME& ft1, const FILETIME& ft2)
{
  return I64_TO_FT( MK_I64(ft1) -= cMK_I64(ft2) );
}
//--------------------------------------------------------------------
inline FILETIME operator +(const FILETIME& ft1, __int64 ft2)
{
  return I64_TO_FT( cMK_I64(ft1) + ft2 );
}
//--------------------------------------------------------------------
inline FILETIME operator +=(FILETIME& ft1, __int64 ft2)
{
  return I64_TO_FT( MK_I64(ft1) += ft2 );
}
//--------------------------------------------------------------------
inline FILETIME operator -(const FILETIME& ft1, __int64 ft2)
{
  return I64_TO_FT( cMK_I64(ft1) - ft2 );
}
//--------------------------------------------------------------------
inline FILETIME operator -=(FILETIME& ft1, __int64 ft2)
{
  return I64_TO_FT( MK_I64(ft1) -= ft2 );
}
//--------------------------------------------------------------------
inline FILETIME operator +(__int64 ft1, const FILETIME& ft2)
{
  return I64_TO_FT( cMK_I64(ft1) + cMK_I64(ft2) );
}
//--------------------------------------------------------------------
inline FILETIME operator -(__int64 ft1, const FILETIME& ft2)
{
  return I64_TO_FT( ft1 - cMK_I64(ft2) );
}
//--------------------------------------------------------------------
#define SECOND_TO_I64  10000000i64
#define HOUR_TO_I64 (MINUTE_TO_I64 * 60)
#define MINUTE_TO_I64 (SECOND_TO_I64 * 60)
#define MK_I64_HOUR(a) hourToI64(a)
#define MK_I64_MINUTE(a) minToI64(a)
//--------------------------------------------------------------------
inline __int64 minToI64(__int64 v)
{
  return MINUTE_TO_I64 * v;
}
//--------------------------------------------------------------------
inline __int64 hourToI64(__int64 v)
{
  __int64 tmp = MK_I64_MINUTE(v);
  return tmp * 60;
}
//--------------------------------------------------------------------
void P_ShowHideTaskBar(BOOL bHide =FALSE);
bool isVisibleTaskBar();
// valida solo su win9x, disattiva tutti i tasti di sistema
bool disableSysKey(bool set);
// per winNT è gestito dal supervisore il controllo dei tasti CTRL, ALT, WINKEY
// (impostabili tramite dialogo di setup), ma non disabilita il CTRL+ALT+CANC
// (occorre creare una propria gina.dll da sostituire a quella di sistema)
//----------------------------------------------------------------------------
bool isWinNT();
bool isWinNT_Based();
bool isWin2000orLater();
bool isWin95();
bool isWin98orLater();
//--------------------------------------------------------------------
bool isWin2k();
bool isWinXP();
bool isWin2k3Server();
//--------------------------------------------------------------------
bool isWinXP_orLater();
bool isWin2k3Server_orLater();
//--------------------------------------------------------------------
// aggiunge in coda a path la barra rovescia se non già presente
// path deve avere _MAX_PATH come dimensione
int appendPath(LPTSTR path, LPCTSTR append = 0);
//--------------------------------------------------------------------
// aggiunge in coda a path la barra rovescia se non già presente
int appendPathSz(LPTSTR path, LPCTSTR append, size_t dim = _MAX_PATH);
// ricrea l'intero percorso
void createDirectoryRecurse(LPTSTR path);
//--------------------------------------------------------------------
template <class T> void safeDeleteP(T& p) { delete p; p = 0; }
template <class T> void safeDeleteA(T& p) { delete []p; p = 0; }
//--------------------------------------------------------------------
#include "restorePack.h"
#endif
