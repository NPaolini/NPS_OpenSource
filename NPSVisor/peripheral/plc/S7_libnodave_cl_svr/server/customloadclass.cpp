//---------- customLoadClass.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "conn_dll.h"
#include "pdllComm.h"
//------------------------------------------------------------------
HMODULE pDll::hDLL;
int pDll::loadedWSA = -1;
prtConnProcDLL pDll::ConnProcDLL;
int pDll::User;
//------------------------------------------------------------------
#define NAME_DLL_CONN _T("nps_conn")
//------------------------------------------------------------------
bool pDll::init()
{
  if(!User) {
    if(!loadProc())
      return false;
    }
  ++User;
  return true;
}
//------------------------------------------------------------------
void pDll::end()
{
  --User;
  if(User <= 0) {
    if(hDLL) {
      FreeLibrary(hDLL);
      hDLL = 0;
      }
    User = 0;
    }
}
//------------------------------------------------------------------
static LPCTSTR libName = NAME_DLL_CONN _T(".dll");
static LPCSTR procName = "_ConnProcDLL";
//------------------------------------------------------------------
bool pDll::loadProc()
{
  hDLL = LoadLibrary(libName);
  if(hDLL) {
    FARPROC f = GetProcAddress(hDLL, procName);
    if(!f)
      f = GetProcAddress(hDLL, procName + 1);
    if(f) {
      ConnProcDLL = (prtConnProcDLL)f;
      return true;
      }
    }
  ConnProcDLL = ConnProcDummy;
  return false;
}
//----------------------------------------------------------------------------
