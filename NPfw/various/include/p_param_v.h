//----- p_param_v.h -------------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_PARAM_V_H
#define P_PARAM_V_H
//----------------------------------------------------------------------------
#include "precHeader.h"

#ifndef PDEF_H_
  #include "pDef.h"
#endif
#ifndef DEFIN_H_
  #include "Defin.h"
#endif
#include "p_vect.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
typedef PVect<wchar_t> pvCharU;
typedef PVect<pvCharU> pvvCharU;
//----------------------------------------------------------------------------
typedef PVect<char> pvCharA;
typedef PVect<pvCharA> pvvCharA;
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define pvChar pvCharU
  #define pvvChar pvvCharU
#else
  #define pvChar pvCharA
  #define pvvChar pvvCharA
#endif
//----------------------------------------------------------------------------
// torna il puntatore alla posizione trovata dopo 'howmanySep' 'sep'
// torna zero se va oltre o se 'p' == 0
#ifdef UNICODE
  #define findNextParam findNextParamU
  LPCSTR findNextParamA(LPCSTR p, int howmanySep = 1, char sep = ',');
#else
  #define findNextParam findNextParamA
  LPCWSTR findNextParamU(LPCWSTR p, int howmanySep = 1, wchar_t sep = L',');
#endif
LPCTSTR findNextParam(LPCTSTR p, int howmanySep = 1, TCHAR sep = _T(','));
//----------------------------------------------------------------------------
// richiama quella sopra, ma scarta gli spazi e i caratteri di tabulazione
#ifdef UNICODE
  #define findNextParamTrim findNextParamTrimU
  LPCSTR findNextParamTrimA(LPCSTR p, int howmanySep = 1, char sep = ',');
#else
  #define findNextParamTrim findNextParamTrimA
  LPCWSTR findNextParamTrimU(LPCWSTR p, int howmanySep = 1, wchar_t sep = L',');
#endif
LPCTSTR findNextParamTrim(LPCTSTR p, int howmanySep = 1, TCHAR sep = _T(','));
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// copia in target la porzione di 'p' che si trova dopo 'afterSep' 'sep' e prima
// del successivo 'sep'. Torna la dimensione necessaria a contenere quella porzione
// se la dimensione eccede maxlen ne vengono copiati solo maxlen (compreso lo zero term.)
// Se target == 0 calcola solo la dimensione.
#ifdef UNICODE
  #define extractParam extractParamU
  uint extractParamA(LPSTR target, size_t maxlen, LPCSTR p, uint afterSep, char sep = ',');
  #define extractParamTrim extractParamTrimU
  uint extractParamTrimA(LPSTR target, size_t maxlen, LPCSTR p, uint afterSep, char sep = ',');
#else
  #define extractParam extractParamA
  uint extractParamU(LPWSTR target, uint maxlen, LPCWSTR p, uint afterSep, wchar_t sep = L',');
  #define extractParamTrim extractParamTrimA
  uint extractParamTrimU(LPWSTR target, uint maxlen, LPCWSTR p, uint afterSep, wchar_t sep = L',');
#endif
uint extractParam(LPTSTR target, size_t maxlen, LPCTSTR p, uint afterSep, TCHAR sep = _T(','));
uint extractParamTrim(LPTSTR target, size_t maxlen, LPCTSTR p, uint afterSep, TCHAR sep = _T(','));
//----------------------------------------------------------------------------
// target è un vettore di vettori ciascuno dei quali conterrà un parametro estratto da 'p'
#ifdef UNICODE
  #define splitParam splitParamU
  uint splitParamA(pvvCharA& target, LPCSTR p, char sep = ',');
  #define splitParamTrim splitParamTrimU
  uint splitParamTrimA(pvvCharA& target, LPCSTR p, char sep = ',');
#else
  #define splitParam splitParamA
  uint splitParamU(pvvCharU& target, LPCWSTR p, wchar_t sep = L',');
  #define splitParamTrim splitParamTrimA
  uint splitParamTrimU(pvvCharU& target, LPCWSTR p, wchar_t sep = L',');
#endif
uint splitParam(pvvChar& target, LPCTSTR p, TCHAR sep = _T(','));
uint splitParamTrim(pvvChar& target, LPCTSTR p, TCHAR sep = _T(','));
//----------------------------------------------------------------------------
// funzione inversa a split, ricrea in target la stringa inserendo il separatore
#ifdef UNICODE
  #define unionParam unionParamU
  uint unionParamA(pvCharA& target, const pvvCharA& source, char sep = ',');
#else
  #define unionParam unionParamA
  uint unionParamU(pvCharU& target, const pvvCharU& source, wchar_t sep = L',');
#endif
uint unionParam(pvChar& target, const pvvChar& source, TCHAR sep = _T(','));
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
