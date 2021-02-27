//--------- manageSplashScreen.cpp -------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "manageSplashScreen.h"
//----------------------------------------------------------------------------
#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
//----------------------------------------------------------------------------
_REG(SPLASH_SCREEN);
UINT getRegMessageSplash() { return WM_SPLASH_SCREEN; }
//----------------------------------------------------------------------------
manageSplashScreen::~manageSplashScreen()
{
  if(hwSplash && IsWindow(hwSplash))
    PostMessage(hwSplash, WM_SPLASH_SCREEN, MAKEWPARAM(WMC_CLOSE, 0), 0);
}
//----------------------------------------------------------------------------
static
LPTSTR getPathSplashScreen()
{
  return _T("svSplash.exe");
}
//----------------------------------------------------------------------------
bool manageSplashScreen::run()
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEONFEEDBACK;

  PROCESS_INFORMATION pi;
  LPTSTR path = getPathSplashScreen();
  TCHAR buff[500];
  wsprintf(buff, _T("%s /H%ld"), path, HWMain);

  if(CreateProcess(0, buff, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
//    WaitForInputIdle(pi.hProcess, 30 * 1000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void manageSplashScreen::waitEnd()
{
  if(hwSplash && IsWindow(hwSplash))
    PostMessage(hwSplash, getRegMessageSplash(), MAKEWPARAM(WMC_END_INIT, 0), 0);
}
//----------------------------------------------------------------------------
void manageSplashScreen::sendMessageSplash(int id, LPCTSTR msg)
{
  if(hwSplash && IsWindow(hwSplash)) {
    if(WMC_LOADING_PRF_0 <= id && id < WMC_LOADING_PRF_0 + MAX_PERIPH_INFO_SPLASH)
      ++totPrf;
    else if(WMC_LOADED_PRF_0 <= id && id < WMC_LOADED_PRF_0 + MAX_PERIPH_INFO_SPLASH) {
      ++loadedPrf;
      if(!getRemainPrf()) {
//        ShowWindow(HWMain, SW_SHOWNORMAL);
        PostMessage(HWMain, getRegMessageSplash(), MAKEWPARAM(WMC_REMOVE_SPLASH, failedPrf), 0);
//        return;
        }
      }
    else if(WMC_FAILED_PRF_0 <= id && id < WMC_FAILED_PRF_0 + MAX_PERIPH_INFO_SPLASH) {
      ++failedPrf;
      if(!getRemainPrf()) {
//        ShowWindow(HWMain, SW_SHOWNORMAL);
        PostMessage(HWMain, getRegMessageSplash(), MAKEWPARAM(WMC_REMOVE_SPLASH, failedPrf), 0);
//        return;
        }
      }
    COPYDATASTRUCT cds;
    cds.lpData = (LPVOID)msg;
    cds.dwData = id;
#ifdef UNICODE
    cds.dwData |= WMC_MSG_IS_UNICODE;
#endif
    cds.cbData = (_tcslen(msg) + 1) * sizeof(*msg);
    SendMessage(hwSplash, WM_COPYDATA, (WPARAM)HWMain, (LPARAM)&cds);
    }
}
