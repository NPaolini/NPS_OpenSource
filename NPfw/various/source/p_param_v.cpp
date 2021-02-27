//------ p_param_v.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_param_v.h"
#include "p_util.h"
//----------------------------------------------------------------------------
template <typename T, size_t (*fzLen)(const T*)>
const T* T_findNextParam(const T* p, int howmanySep, T sep)
{
  if(!p)
    return 0;
  if(!howmanySep)
    return p;

  int nSep = 0;
  while(*p) {
    if(sep == *p) {
      ++nSep;
      if(nSep >= howmanySep) {
        ++p;
        return *p ? p : 0;
        }
      }
    ++p;
    }
  return 0;
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fzLen)(const T*)>
const T* T_findNextParamTrim(const T* p, int howmanySep, T sep)
{
  p = T_findNextParam<T, fzLen>(p, howmanySep, sep);
  if(p) {
    while(*p && T(' ') >= (unsigned)*p)
      ++p;
    return *p ? p : 0;
    }
  return 0;
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fzLen)(const T*)>
uint T_extractParam(T* target, size_t maxlen, const T* p, uint afterSep, T sep)
{
  if(target)
    *target = 0;
  p = T_findNextParam<T, fzLen>(p, afterSep, sep);
  if(!p)
    return 0;
  const T* p2 = T_findNextParam<T, fzLen>(p, 1, sep);
  size_t len = p2 ? p2 - p - 1 : fzLen(p);

  if(target) {
    maxlen = min(len, maxlen - 1);
    copyStr(target, p, maxlen);
    target[maxlen] = 0;
    }
  return uint(len + 1);
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fzLen)(const T*)>
uint T_extractParamTrim(T* target, size_t maxlen, const T* p, uint afterSep, T sep)
{
  if(target)
    *target = 0;
  p = T_findNextParamTrim<T, fzLen>(p, afterSep, sep);
  if(!p)
    return 0;
  const T* p2 = T_findNextParamTrim<T, fzLen>(p, 1, sep);
  size_t len = p2 ? p2 - p - 1 : fzLen(p);

  if(target) {
    maxlen = min(len, maxlen - 1);
    copyStr(target, p, maxlen);
    target[maxlen] = 0;
    }
  return uint(len + 1);
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fzLen)(const T*)>
uint getLastLen(const T* p, T sep)
{
  size_t len = fzLen(p);
  if(1 == len && sep == *p)
    return 0;
  return uint(len);
}
//----------------------------------------------------------------------------
template <typename PVV, typename PV, typename T, size_t (*fzLen)(const T*)>
uint T_splitParam(PVV& target, const T* p, T sep)
{
  uint nElem = 0;
  uint len = 1; // qualunque valore diverso da zero
  bool needAdd =true;
  while(p) {
    const T* p2 = T_findNextParam<T, fzLen>(p, 1, sep);
    if(!p2) {
      size_t l = fzLen(p);
      // se l'ultimo valore finisce col separatore, va eliminato il separatore
      // ma così facendo farebbe un ciclo in più, allora si bypassa l'aggiunta dopo il ciclo
      if(l > 1 && sep == p[l - 1]) {
        p2 = p + l;
        needAdd = false;
        }
      }
    len = uint(p2 ? p2 - p - 1 : getLastLen<T, fzLen>(p, sep));
    PV& pc = target[nElem];
    pc.setDim(len + 1);
    for(uint i = 0; i < len; ++i)
      pc[i] = p[i];
    pc[len] = 0;
    p = p2;
    ++nElem;
    }
  if(!len && needAdd) {
    PV& pc = target[nElem];
    pc[0] = 0;
    ++nElem;
    }
  return nElem;
}
//----------------------------------------------------------------------------
template <typename PVV, typename PV, typename T, size_t (*fzLen)(const T*)>
uint T_splitParamTrim(PVV& target, const T* p, T sep)
{
  uint nElem = 0;
  uint len = 1; // qualunque valore diverso da zero
  bool needAdd =true;
  while(p) {
    const T* p2 = T_findNextParamTrim<T, fzLen>(p, 1, sep);
    if(!p2) {
      size_t l = fzLen(p);
      // se l'ultimo valore finisce col separatore, va eliminato il separatore
      // ma così facendo farebbe un ciclo in più, allora si bypassa l'aggiunta dopo il ciclo
      if(l > 1 && sep == p[l - 1]) {
        p2 = p + l;
        needAdd = false;
        }
      }
    len = uint(p2 ? p2 - p - 1 : getLastLen<T, fzLen>(p, sep));
    PV& pc = target[nElem];
    pc.setDim(len + 1);
    for(uint i = 0; i < len; ++i)
      pc[i] = p[i];
    pc[len] = 0;
    p = p2;
    ++nElem;
    }
  if(!len && needAdd) {
    PV& pc = target[nElem];
    pc[0] = 0;
    ++nElem;
    }
  return nElem;
}
//----------------------------------------------------------------------------
template <typename PVV, typename PV, typename T>
uint T_unionParam(PV& target, const PVV& source, T sep)
{
  uint nElem = source.getElem();
  if(!nElem)
    return 0;
  uint totLen = 0;
  for(uint i = 0; i < nElem; ++i) {
    const PV& pc = source[i];
    totLen += pc.getElem();
    }
  target.setDim(target.getElem() + totLen + nElem + 1);

  uint curLen = target.getElem();
  for(uint i = 0; i < nElem; ++i) {
    const PV& pc = source[i];
    uint len = pc.getElem();
    for(uint j = 0; j < len; ++j, ++curLen) {
      if(!pc[j])
        break;
      target[curLen] = pc[j];
      }
    target[curLen++] = sep;
    }
  // il -1 per eliminare l'ultimo separatore
  target[curLen - 1] = 0;
  return curLen;
}
//----------------------------------------------------------------------------
#define PAR_2_A char, strlen
#define PAR_2_U wchar_t, wcslen
//----------------------------------------------------------------------------
#define PAR_3_A pvvCharA, pvCharA, char
#define PAR_3_U pvvCharU, pvCharU, wchar_t
//----------------------------------------------------------------------------
#define PAR_4_A pvvCharA, pvCharA, char, strlen
#define PAR_4_U pvvCharU, pvCharU, wchar_t, wcslen
//----------------------------------------------------------------------------
#define MAKE_FZ_2_3(fz, T, t1, t2, t3, tRet) tRet fz##T(t1 p1, t2 p2, t3 p3) { return T_##fz<PAR_2_##T>(p1, p2, p3); }
#define MAKE_FZ_2_5(fz, T, t1, t2, t3, t4, t5, tRet) tRet fz##T(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) \
    { return T_##fz<PAR_2_##T>(p1, p2, p3, p4, p5); }
#define MAKE_FZ_3_3(fz, T, t1, t2, t3, tRet) tRet fz##T(t1 p1, t2 p2, t3 p3) { return T_##fz<PAR_3_##T>(p1, p2, p3); }
#define MAKE_FZ_4_3(fz, T, t1, t2, t3, tRet) tRet fz##T(t1 p1, t2 p2, t3 p3) { return T_##fz<PAR_4_##T>(p1, p2, p3); }
//----------------------------------------------------------------------------
MAKE_FZ_2_3(findNextParam, U, LPCWSTR, int, wchar_t, LPCWSTR)
MAKE_FZ_2_3(findNextParam, A, LPCSTR, int, char, LPCSTR)
MAKE_FZ_2_3(findNextParamTrim, U, LPCWSTR, int, wchar_t, LPCWSTR)
MAKE_FZ_2_3(findNextParamTrim, A, LPCSTR, int, char, LPCSTR)
//----------------------------------------------------------------------------
MAKE_FZ_2_5(extractParam, U, LPWSTR, size_t, LPCWSTR, uint, wchar_t, uint)
MAKE_FZ_2_5(extractParam, A, LPSTR, size_t, LPCSTR, uint, char, uint)
//----------------------------------------------------------------------------
MAKE_FZ_2_5(extractParamTrim, U, LPWSTR, size_t, LPCWSTR, uint, wchar_t, uint)
MAKE_FZ_2_5(extractParamTrim, A, LPSTR, size_t, LPCSTR, uint, char, uint)
//----------------------------------------------------------------------------
MAKE_FZ_3_3(unionParam, U, pvCharU&, const pvvCharU&, wchar_t, uint)
MAKE_FZ_3_3(unionParam, A, pvCharA&, const pvvCharA&, char, uint)
//----------------------------------------------------------------------------
MAKE_FZ_4_3(splitParam, U, pvvCharU&, LPCWSTR, wchar_t, uint)
MAKE_FZ_4_3(splitParam, A, pvvCharA&, LPCSTR, char, uint)
//----------------------------------------------------------------------------
MAKE_FZ_4_3(splitParamTrim, U, pvvCharU&, LPCWSTR, wchar_t, uint)
MAKE_FZ_4_3(splitParamTrim, A, pvvCharA&, LPCSTR, char, uint)
//----------------------------------------------------------------------------
