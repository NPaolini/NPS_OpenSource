//--------------- s7_libnodave_common.cpp ------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "s7_libnodave_common.h"
//----------------------------------------------------------
static void replaceSlash(LPTSTR buff)
{
  for(size_t i = 0; buff[i]; ++i)
    switch(buff[i]) {
      case _T('\\'):
      case _T('.'):
      case _T(':'):
      case _T('/'):
      case _T('?'):
      case _T('*'):
        buff[i] = _T('_');
        break;
      }
}
//----------------------------------------------------------
void makeServerMutexName(LPTSTR mx, LPCTSTR ns)
{
  TCHAR t[_MAX_PATH];
  _tfullpath(t, ns, SIZE_A(t));
  replaceSlash(t);
  _tcscpy_s(mx, _MAX_PATH, _T("s7_libnodave_server_"));
  _tcscat_s(mx, _MAX_PATH, t);
}
//----------------------------------------------------------
void makeClientMutexName(LPTSTR mx, LPCTSTR ns)
{
  TCHAR t[_MAX_PATH];
  _tfullpath(t, ns, SIZE_A(t));
  replaceSlash(t);
  _tcscpy_s(mx, _MAX_PATH, _T("s7_libnodave_client_"));
  _tcscat_s(mx, _MAX_PATH, t);
}
//----------------------------------------------------------
