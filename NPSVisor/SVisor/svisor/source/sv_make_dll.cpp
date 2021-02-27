//-------- sv_make_dll.cpp -----------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
//------------------------------------------------------------------
#define DEF_PTR_DEFINE
#include "sv_make_dll.h"
#undef DEF_PTR_DEFINE
//------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
//------------------------------------------------------------------
static HINSTANCE hInst = 0;
HINSTANCE getHinstance() { return hInst; }
//------------------------------------------------------------------
template <typename T>
bool getProcAddress(HMODULE hdll, LPCSTR procName, T& proc)
{
  FARPROC f = GetProcAddress(hdll, procName + 1);
  if(!f)
    f = GetProcAddress(hdll, procName);
  if(!f)
    return false;
  proc = (T)f;
  return true;
}
//------------------------------------------------------------------
#define get_proc(hDLL, a) getProcAddress<npSV_##a##_>(hDLL, "_npSV_" #a, npSV_##a)
//------------------------------------------------------------------
#define GET_PROC(a) if(!get_proc(hInst, a)) break
//------------------------------------------------------------------
static BOOL loadAddr(HINSTANCE hinstDLL)
{
  BOOL ok = TRUE;
  while(!hInst) {
    ok = FALSE;
    hInst = GetModuleHandle(0);
    if(!hInst)
       break;
    GET_PROC(GetGlobalString);
    GET_PROC(GetLocalString);
    GET_PROC(GetPageString);
    GET_PROC(Set);
    GET_PROC(GetSet);
    GET_PROC(Get);
    GET_PROC(MultiSet);
    GET_PROC(GetNormaliz);
    GET_PROC(GetBodyRoutine);
    ok = TRUE;
    break;
    }
  hInst = hinstDLL;
  return ok;
}
//------------------------------------------------------------------
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
          return loadAddr(hinstDLL);
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
//          break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
          break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
//------------------------------------------------------------------
BOOL WINAPI DllEntryPoint(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
  return DllMain(hinstDLL, fdwReason, lpReserved);
}
//------------------------------------------------------------------
