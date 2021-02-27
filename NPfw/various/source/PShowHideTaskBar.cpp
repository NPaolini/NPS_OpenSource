//-------------- PShowHideTaskBar.cpp ----------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_util.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void P_ShowHideTaskBar(BOOL bHide)
{
  HWND pWnd = ::FindWindow(_T("Shell_TrayWnd"), _T(""));

  if(pWnd) {
    if(bHide ^ IsWindowVisible(pWnd))
      return;
    ShowWindow(pWnd, bHide ? SW_HIDE : SW_SHOW);
    }
}
//----------------------------------------------------------------------------
bool isVisibleTaskBar()
{
  HWND pWnd = ::FindWindow(_T("Shell_TrayWnd"), _T(""));

  if(pWnd)
    return toBool(IsWindowVisible(pWnd));

  return true;
}
//----------------------------------------------------------------------------
#ifndef SPI_SETSCREENSAVERRUNNING
  #define SPI_SETSCREENSAVERRUNNING SPI_SCREENSAVERRUNNING
#endif
bool disableSysKey(bool set)
{
  if(isWinNT() || isWin2000orLater())
    return false;
  DWORD dummy;
  if(!SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, set ? TRUE : 0, &dummy, 0)) {
    return false;
    }
  return true;
}
//-----------------------------------------------------------------
enum winVer { notChecked, verWin95, verWin98OrLater, verNT, verNT2000, verXP, verNT2003S, verNTOver };
static winVer verWin = notChecked;
//-----------------------------------------------------------------
static void checkVer()
{
  OSVERSIONINFO ver;
  ZeroMemory(&ver, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  verWin = verWin95;
  if(GetVersionEx(&ver)) {
    if(VER_PLATFORM_WIN32_NT == ver.dwPlatformId) {
      if(ver.dwMajorVersion > 4) {
        switch(ver.dwMinorVersion) {
          case 0:
            verWin = verNT2000;
            break;
          case 1:
            verWin = verXP;
            break;
          case 2:
            verWin = verNT2003S;
            break;
          default:
            verWin = verNTOver;
          }
        }
      else
        verWin = verNT;
      }
    else if(ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
      if((ver.dwMajorVersion > 4) ||
            ((ver.dwMajorVersion == 4) && (ver.dwMinorVersion > 0)))
        verWin = verWin98OrLater;
      }
    }
}
//-----------------------------------------------------------------
bool isWinNT()
{
  if(notChecked == verWin)
    checkVer();
  return verNT == verWin;
}
//-----------------------------------------------------------------
bool isWinNT_Based()
{
  if(notChecked == verWin)
    checkVer();
  return verNT <= verWin;
}
//-----------------------------------------------------------------
bool isWin2k()
{
  if(notChecked == verWin)
    checkVer();
  return verNT2000 == verWin;
}
//-----------------------------------------------------------------
bool isWinXP()
{
  if(notChecked == verWin)
    checkVer();
  return verXP == verWin;
}
//-----------------------------------------------------------------
bool isWin2k3Server()
{
  if(notChecked == verWin)
    checkVer();
  return verNT2003S == verWin;
}
//-----------------------------------------------------------------
bool isWinXP_orLater()
{
  if(notChecked == verWin)
    checkVer();
  return verXP <= verWin;
}
//-----------------------------------------------------------------
bool isWin2k3Server_orLater()
{
  if(notChecked == verWin)
    checkVer();
  return verNT2003S <= verWin;
}
//-----------------------------------------------------------------
bool isWin2000orLater()
{
  if(notChecked == verWin)
    checkVer();
  return verNT2000 <= verWin;
}
//-----------------------------------------------------------------
bool isWin95()
{
  if(notChecked == verWin)
    checkVer();
  return verWin95 == verWin;
}
//-----------------------------------------------------------------
bool isWin98orLater()
{
  if(notChecked == verWin)
    checkVer();
  return verWin98OrLater == verWin;
}
//----------------------------------------------------------------------------
