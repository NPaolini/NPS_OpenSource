//------ p_strdup.cpp --------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include "p_util.h"
//----------------------------------------------------------------------------
template <typename T, size_t (*fz)(const T*)>
T* t_str_newdup(const T* str)
{
  if(!str) {
    T* tmp = new T[1];
    tmp[0] = 0;
    return tmp;
    }
  size_t len = fz(str) + 1;
  T* tmp = new T[len];
  for(size_t i = 0; i < len; ++i)
    tmp[i] = str[i];
  return tmp;
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fz)(const T*)>
T* t_trim(T* buff)
{
  if(!buff)
    return 0;
  int len = (int)fz(buff);

  for(--len; len >= 0; --len)
    if((unsigned)buff[len] > (T)' ')
      break;

  buff[len + 1] = 0;
  return buff;
}
//----------------------------------------------------------------------------
template <typename T>
T* t_lTrim(T* buff)
{
  if(!buff || (unsigned)buff[0] > (T)' ')
    return buff;

  size_t offs = 0;
  for(;buff[offs]; ++offs)
    if((unsigned)buff[offs] > (T)' ')
      break;
  for(size_t i = 0;; ++i, ++offs) {
    buff[i] = buff[offs];
    if(!buff[i])
      break;
    }

  return buff;
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fz)(const T*)>
T* t_strAlign(T* t, const T* s, int len, bool left, bool zero)
{
  if(!*s) {
    if(zero)
      *t = 0;
    return t;
    }
  if(left) {
    int i;
    for(i = 0; i < len; ++i)
      if(s[i] && s[i] != (T)' ')
        break;
    len -= i;
    memmove(t, s + i, len * sizeof(*t));
    }
  else {
    int i = (int)fz(s);
    if(i > len)
      i = len;
    memmove(t + len - i, s, i * sizeof(*t));
    }
  if(zero)
    t[len]=0;
  return t;
}
//----------------------------------------------------------------------------
LPSTR str_newdup(LPCSTR str)
{
  return t_str_newdup<char, strlen>(str);
}
//----------------------------------------------------------------------------
LPSTR trim(LPSTR buff)
{
  return t_trim<char, strlen>(buff);
}
//----------------------------------------------------------------------------
LPSTR lTrim(LPSTR buff)
{
  return t_lTrim<char>(buff);
}
//----------------------------------------------------------------------------
LPSTR strAlign(LPSTR t, LPCSTR s, int len, bool left, bool zero)
{
  return t_strAlign<char, strlen>(t, s, len, left, zero);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPWSTR str_newdup(LPCWSTR str)
{
  return t_str_newdup<wchar_t, wcslen>(str);
}
//----------------------------------------------------------------------------
LPWSTR trim(LPWSTR buff)
{
  return t_trim<wchar_t, wcslen>(buff);
}
//----------------------------------------------------------------------------
LPWSTR lTrim(LPWSTR buff)
{
  return t_lTrim<wchar_t>(buff);
}
//----------------------------------------------------------------------------
LPWSTR strAlign(LPWSTR t, LPCWSTR s, int len, bool left, bool zero)
{
  return t_strAlign<wchar_t, wcslen>(t, s, len, left, zero);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void makeTruncString(double val, LPTSTR buff, size_t lenBuff)
{
  long v = ROUND_REAL(val);
  _ltot_s(v, buff, lenBuff, 10);
}
//----------------------------------------------------------------------------
void makeTruncStringU(double val, LPTSTR buff, size_t lenBuff)
{
  DWORD v = ROUND_REAL(val);
  _stprintf_s(buff, lenBuff, _T("%u"), v);
}
//----------------------------------------------------------------------------
#define MAX_REALDATA (MAXLONG * 20.0)
//----------------------------------------------------------------------------
void makeFixedString(double val, LPTSTR buff, size_t lenBuff, unsigned dec)
{
  if(!dec)
    makeTruncString(val, buff, lenBuff);
  else {
    if(val > MAX_REALDATA || val < -MAX_REALDATA) {
      _tcscpy_s(buff, lenBuff, _T("Err"));
      return;
      }
  _stprintf_s(buff, lenBuff, _T("%0.*f"), dec, val);
  }
}
//----------------------------------------------------------------------------
void makeFixedStringU(double val, LPTSTR buff, size_t lenBuff, unsigned dec)
{
  if(!dec)
    makeTruncStringU(val, buff, lenBuff);
  else {
    if(val > MAX_REALDATA || val < -MAX_REALDATA) {
      _tcscpy_s(buff, lenBuff, _T("Err"));
      return;
      }
  _stprintf_s(buff, lenBuff, _T("%0.*f"), dec, val);
  }
}
//----------------------------------------------------------------------------
int appendPath(LPTSTR path, LPCTSTR append)
{
  if(append)
    return appendPathSz(path, append);

  size_t len = _tcslen(path);
  if(len > 0 && _T('\\') != path[len - 1]) {
    path[len++] = _T('\\');
    path[len] = 0;
    }
  return int(len);
}
//----------------------------------------------------------------------------
int appendPathSz(LPTSTR path, LPCTSTR append, size_t dim)
{
  size_t len = _tcslen(path);
  if(len > 0 && _T('\\') != path[len - 1]) {
    path[len++] = _T('\\');
    path[len] = 0;
    }
  _tcscat_s(path, dim, append);
  len = _tcslen(path);
  return int(len);
}
//----------------------------------------------------------------------------
void createDirectoryRecurse(LPTSTR path)
{
  if(!path || !*path)
    return;
  if(!CreateDirectory(path, 0)) {
    int len = (int)_tcslen(path);
    int i;
    for(i = len - 1; i > 0; --i)
      if(_T('\\') == path[i])
        break;
    if(i) {
      path[i] = 0;
      createDirectoryRecurse(path);
      path[i] = _T('\\');
      CreateDirectory(path, 0);
      }
    }
}
//----------------------------------------------------------------------------
void zeroTrim(LPTSTR t)
{
  if(!_tcschr(t, _T('.')))
    return;
  int len = (int)_tcslen(t);
  for(int i = len - 1; i > 0; --i) {
    if(_T('.') == t[i]) {
      t[i] = 0;
      break;
      }
    else if(_T('0') == t[i])
      t[i] = 0;
    else break;
    }
}
