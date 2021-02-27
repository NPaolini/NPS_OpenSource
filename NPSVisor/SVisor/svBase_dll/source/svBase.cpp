//-------- svBase.cpp ---------------------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <shlwapi.h>
#include <shlobj.h>
//-----------------------------------------------------------
//#define USE_SHOW_TRACE
#ifdef USE_SHOW_TRACE
  #define SHOW_TRACE(a, b) show_trace(a, b);
  void show_trace(LPCTSTR msg1, LPCTSTR msg2)
  {
    TCHAR t[4096];
    wsprintf(t, _T("%s - %s"), msg1, msg2);
    MessageBox(0, t, _T("Error"), MB_OK | MB_ICONSTOP);
  }
#else
  #define SHOW_TRACE(a, b)
#endif
//-----------------------------------------------------------
static LPBYTE cryptCode;
static HINSTANCE g_hinstDLL;
//-----------------------------------------------------------
static void loadResKey()
{
  HRSRC hResource = ::FindResource(g_hinstDLL, MAKEINTRESOURCE(111), RT_RCDATA);
  while(hResource) {
    DWORD size = ::SizeofResource(g_hinstDLL, hResource);
    if(!size)
      break;

    const void* pResourceData = ::LockResource(::LoadResource(g_hinstDLL, hResource));
    if(!pResourceData)
      break;

    cryptCode = new BYTE[size + 2];
    CopyMemory(cryptCode, pResourceData, size);
    break;
    }
}
//-----------------------------------------------------------
static void unloadResKey()
{
  delete []cryptCode;
  cryptCode = 0;
}
//-----------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------
__declspec(dllexport) void SV_Base(LPBYTE buff)
{
  DWORD k = *(LPDWORD)buff;
 *(LPDWORD)buff ^= *(LPDWORD)(cryptCode + 8);
 *(LPDWORD)(buff + 4) = ~k ^ *(LPDWORD)(cryptCode + 12);
  DWORD t = *(LPDWORD)(cryptCode + 16);
  DWORD crc =  *(LPDWORD)(cryptCode + 20);
  *(LPDWORD)(buff + 12) = (*(LPDWORD)(buff + 12) ^ *(LPDWORD)(buff + 8) ^ k ^ t) + k + crc;
}
//-----------------------------------------------------------
__declspec(dllexport) void SV_Ver(LPBYTE buff)
{
  DWORD k = *(LPDWORD)buff;
 *(LPDWORD)buff ^= *(LPDWORD)(cryptCode + 16);
}
//-----------------------------------------------------------
#ifdef __cplusplus
 }
#endif
//-----------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    // Perform actions based on the reason for calling.
  switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
      // Initialize once for each new process.
      // Return FALSE to fail DLL load.
      g_hinstDLL = hinstDLL;
      loadResKey();
      break;

    case DLL_THREAD_ATTACH:
      // Do thread-specific initialization.
      break;

    case DLL_THREAD_DETACH:
      // Do thread-specific cleanup.
      break;

    case DLL_PROCESS_DETACH:
      // Perform any necessary cleanup.
      unloadResKey();
      break;
    }
  return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
//-----------------------------------------------------------
BOOL WINAPI DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
  return DllMain(hinstDLL, fdwReason, lpReserved);
}
//-----------------------------------------------------------
