//-------------- simple_helper.cpp -------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "simple_helper.h"
//----------------------------------------------------------------------------
simple_BSTR::simple_BSTR(LPCSTR name)
{
  int len = MultiByteToWideChar(CP_ACP, 0, name, -1, 0, 0);
  LPWSTR buff = new WCHAR[len + 1];
  MultiByteToWideChar(CP_ACP, 0, name, -1, buff, len);
  P = SysAllocString(buff);
  delete []buff;
}
//----------------------------------------------------------------------------
