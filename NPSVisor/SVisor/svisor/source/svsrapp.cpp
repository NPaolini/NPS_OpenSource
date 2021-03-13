//------ svsrapp.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <objbase.h>
#include <Shellapi.h>
#include <Shlwapi.h>
//----------------------------------------------------------------------------
//#include <stdio.h>

#include "mainApp.h"
#include "1.h"
#include "sizer.h"
#include "manageSplashScreen.h"
#include "p_txt.h"
#include "currAlrm.h"
//----------------------------------------------------------------------------
static int whereIs(LRESULT pos);
//----------------------------------------------------------------------------
static manageSplashScreen* MSS = 0;
//----------------------------------------------------------------------------
void closeHWSplashScreen()
{
  delete MSS;
  MSS = 0;
}
//----------------------------------------------------------------------------
bool sendMessageSplash(int id, LPCTSTR msg)
{
  if(MSS) {
    MSS->sendMessageSplash(id, msg);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool isOnSplash()
{
  return toBool(MSS);
}
//-----------------------------------------------------------
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT

#define SPI_GETACTIVEWINDOWTRACKING         0x1000
#define SPI_SETACTIVEWINDOWTRACKING         0x1001
#define SPI_GETMENUANIMATION                0x1002
#define SPI_SETMENUANIMATION                0x1003
#define SPI_GETCOMBOBOXANIMATION            0x1004
#define SPI_SETCOMBOBOXANIMATION            0x1005
#define SPI_GETLISTBOXSMOOTHSCROLLING       0x1006
#define SPI_SETLISTBOXSMOOTHSCROLLING       0x1007
#define SPI_GETGRADIENTCAPTIONS             0x1008
#define SPI_SETGRADIENTCAPTIONS             0x1009
#define SPI_GETMENUUNDERLINES               0x100A
#define SPI_SETMENUUNDERLINES               0x100B
#define SPI_GETACTIVEWNDTRKZORDER           0x100C
#define SPI_SETACTIVEWNDTRKZORDER           0x100D
#define SPI_GETHOTTRACKING                  0x100E
#define SPI_SETHOTTRACKING                  0x100F
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#define SPI_GETACTIVEWNDTRKTIMEOUT          0x2002
#define SPI_SETACTIVEWNDTRKTIMEOUT          0x2003
#define SPI_GETFOREGROUNDFLASHCOUNT         0x2004
#define SPI_SETFOREGROUNDFLASHCOUNT         0x2005
//#endif

#endif
//----------------------------------------------------------------------------
static DWORD prevCountSetF = 0;
static bool gActive;
//----------------------------------------------------------------------------
bool isActiveApp() { return gActive; }
//----------------------------------------------------------------------------
static void activeSetForeg(bool active)
{
  gActive = active;
  if(!active || prevCountSetF)
    return;
  if(!SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)&prevCountSetF, 0))
    prevCountSetF = (DWORD)-1;
  else {
    DWORD value = 0;
    if(!SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)value,
          SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE))
      prevCountSetF = (DWORD)-1;
    }
}
//----------------------------------------------------------------------------
static void restoreSetForeg()
{
  if(!prevCountSetF || (DWORD)-1 == prevCountSetF)
    return;
  SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)prevCountSetF,
        SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
}
//-----------------------------------------------------------
mainWin::mainWin(HINSTANCE hInstance) : PMainWin(_T("SuperVisor"), hInstance), idTimer(0), sysTrayObj(0),
      manObjSysTray(0), idTimerTray(0)
{
  client = allocMainClient(this, hInstance);
  Attr.x = -1;
  Attr.y = -1;
  Attr.w = sizer::getWidth() + 2;
  Attr.h = sizer::getHeight() + 2;
  customizeMain(this);
  Attr.style &= ~WS_VISIBLE;
}
//----------------------------------------------------------------------------
mainWin::~mainWin() { delete sysTrayObj; delete manObjSysTray; destroy();  restoreSetForeg(); }
//-----------------------------------------------------------
#define ID_TIMER_SPLASH 99
// se il tickCount di windows è minore, presuppone che sia in autorun
#define MIN_START (1000 * 60 * 3)
//-----------------------------------------------------------
//#define SIZEABLE
//-----------------------------------------------------------
static bool startOnSysTray()
{
  return false;
/*
  LPCTSTR p = getString(ID_SYS_TRAY);
  int toSysTray = p ? _ttoi(p) : 0;
  return 2 == toSysTray;
*/
}
//----------------------------------------------------------------------------
bool setKeyDWord(LPCTSTR keyName, DWORD value);
bool getKeyDWord(LPCTSTR keyName, DWORD& value);
//-----------------------------------------------------------
struct lastPosMain
{
  union {
    struct {
      short int x;
      short int y;
      };
    DWORD v;
    } U;
  lastPosMain() { U.v = 0; }
  lastPosMain(int x, int y) {
    U.x = x; U.y = y;  }
  operator DWORD&() { return U.v; }
};
//-----------------------------------------------------------
#define LAST_POS_KEY _T("LastPos")
//-----------------------------------------------------------
bool retrieveLastPos(PRect& rOffs)
{
  lastPosMain lpm;
  if(getKeyDWord(LAST_POS_KEY, lpm)) {
    rOffs.MoveTo(lpm.U.x, lpm.U.y);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void savePos(HWND hwnd)
{
  PRect r;
  GetWindowRect(hwnd, r);
  lastPosMain lpm(r.left, r.top);
  setKeyDWord(LAST_POS_KEY, lpm);
}
//-----------------------------------------------------------
bool mainWin::create()
{
  getGestAlarm()->initialize();
  manObjSysTray = new manTipsObj(client);
  if(!manObjSysTray->getElem()) {
    delete manObjSysTray;
    manObjSysTray = 0;
    }
  if(manObjSysTray)
    Attr.style |= WS_MINIMIZEBOX | WS_SYSMENU;
#ifdef SIZEABLE
  Attr.style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
#endif
  if(!PMainWin::create()) {
    PostQuitMessage(EXIT_FAILURE);
    return false;
    }
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON_NPS));
#if 1
#define iconSm icon
#else
  HICON iconSm  = (HICON)LoadImage(getHInstance(), MAKEINTRESOURCE(IDI_ICON_NPS), IMAGE_ICON, 16, 16, 0);
#endif
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(iconSm));

  PRect r;
  GetClientRect(*this, r);
  client->setWindowPos(0, r, SWP_NOZORDER);
  LPCTSTR p = getString(ID_RESOLUTION_BASE);
  p = findNextParamTrim(p);
  if(p) {
    int ixMon = _ttoi(p);
    PRect rOffs(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    p = findNextParamTrim(p);
    int addX = 0;
    int addY = 0;
    if(ixMon >= 0 && p && _ttoi(p)) {
      GetWindowRect(*this, rOffs);
      rOffs.MoveTo(0, 0);
      addX = 2;
      addY = 2;
      }
    switch(ixMon) {
      case 0:  // è il primo, nessuna azione
      case 10:
      default:
        if(retrieveLastPos(rOffs))
          ixMon = -1;
        else
          ixMon = 0;
        break;
      case 20:
        rOffs.Offset(rOffs.Width(), 0);
        addY = 0;
        break;
      case 30:
        rOffs.Offset(rOffs.Width() * 2, 0);
        addX *= 2;
        addY = 0;
        break;
      case 40:
        rOffs.Offset(rOffs.Width() * 3, 0);
        addX *= 3;
        addY = 0;
        break;
      case 130:
        rOffs.Offset(0, rOffs.Height());
        addX = 0;
        break;
      case 140:
        rOffs.Offset(rOffs.Width(), rOffs.Height());
        break;
      }
    if(ixMon) {
      SetWindowPos(*this, 0, rOffs.left + addX, rOffs.top + addY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      UpdateWindow(*this);
      }
    }

  // se non riesce a creare lo splash riattiva il visibile e avvia
  // subito le periferiche (altrimenti saranno avviate all'arrivo
  // della risposta dello splash o dopo un tempo prestabilito)

  MSS = new manageSplashScreen(getHandle());
  uint delay = 0;
  p = getString(ID_RUNDELAY);
  if(p && GetTickCount() < MIN_START)
    delay = _ttoi(p) * 1000;
  if(!MSS->run()) {
    closeHWSplashScreen();
//    PostMessage(*this, getRegMessageSplash(), MAKEWPARAM(WMC_REMOVE_SPLASH, 0), 0);
    Sleep(delay);
    runPerif(client);
    }
  else {
    idTimer = ID_TIMER_SPLASH;
    Sleep(delay);
//    PostMessage(*this, getRegMessageSplash(), MAKEWPARAM(WMC_START_TIMER, 0), 0);
    SetTimer(*this, ID_TIMER_SPLASH, 30000, 0);
    }
  return true;
}
//-----------------------------------------------------------
void mainWin::killTimer()
{
  if(idTimer) {
    idTimer = 0;
    KillTimer(*this, ID_TIMER_SPLASH);
    }
}
//----------------------------------------------------------
LRESULT mainWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(getRegMessageSplash() == message) {
    switch(LOWORD(wParam)) {
      case WMC_REMOVE_SPLASH:
        killTimer();
        if(MSS)
          MSS->waitEnd();
        if(HIWORD(wParam)) {
          SendMessage(*this, getRegMessageSplash(), MAKEWPARAM(WMC_TIMEOUT_SPLASH, 0), 0);
          closeHWSplashScreen();
          break;
          }
        closeHWSplashScreen();
        ShowWindow(*this, SW_SHOWNORMAL);
        do {
          mainClient* mc = getMain();
//          mc->reloadBody();
          mc->showPart();
          if(startOnSysTray())
            PostMessage(*this, WM_GO_TO_TRAY, 0, 0);
          } while(false);
        break;
      case WMC_SPLASH_HWND:
        killTimer();
        if(MSS) {
          MSS->setHwSplash((HWND)lParam);
          PostMessage(MSS->getHWSplashScreen(), getRegMessageSplash(), MAKEWPARAM(WMC_START_TIMER, 0), 0);
          runPerif(client);
          }
        break;
      case WMC_HIDE_THIS:
        ShowWindow(*this, SW_HIDE);
        break;

      case WMC_TIMEOUT_SPLASH:
        killTimer();
        do {
          if(IDNO == msgBoxByLangGlob(this, ID_MSG_PRPH_NOT_LOADED, ID_TITLE_PRPH_NOT_LOADED,
                MB_YESNO | MB_ICONSTOP | MB_SYSTEMMODAL)) {
            closeHWSplashScreen();
            PostQuitMessage(EXIT_FAILURE);
            return 0;
            }
          } while(false);
        PostMessage(*this, getRegMessageSplash(), MAKEWPARAM(WMC_REMOVE_SPLASH, 0), 0);

        break;

      }
    return PMainWin::windowProc(hwnd, message, wParam, lParam);
    }
  if(sysTrayObj) {
    LRESULT result;
    if(sysTrayObj->windowProc(hwnd, message, wParam, lParam, result))
      return result;
    }

  switch(message) {
    case WM_DESTROY:
      savePos(hwnd);
      break;
    case WM_ACTIVATEAPP:
      activeSetForeg(toBool(wParam));
      break;

    case WM_TIMER:
      if(ID_TIMER_SPLASH == wParam) {
        if(!MSS) { // && non avviati
          killTimer();
          PostMessage(*this, getRegMessageSplash(), MAKEWPARAM(WMC_REMOVE_SPLASH, 0), 0);
          runPerif(client);
          }
        }
      break;

    case WM_SYSCOMMAND:
      switch(wParam & 0xFFF0) {
        case SC_MINIMIZE:
          if(!manObjSysTray)
            break;
          PostMessage(*this, WM_GO_TO_TRAY, 0, 0);
          return true;
        case SC_CLOSE:
          return true;
        }
      break;

    case WM_GO_TO_TRAY:
      if(manObjSysTray) {
        if(!sysTrayObj) {
          sysTrayObj = new sysTray(this, *manObjSysTray);
          idTimerTray = SetTimer(*this, ID_TIMER_CHECK_TRAY, 3000, 0);
          }
        }
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case WM_REMOVE_FROM_TRAY:
          if(idTimerTray) {
            KillTimer(*this, idTimerTray);
            idTimerTray = 0;
            }
          delete sysTrayObj;
          sysTrayObj = 0;
          break;
        }
      break;

    case WM_SIZE:
      do {
        if(getHandle()) {
          PRect r;
          GetClientRect(*this, r);
          client->setWindowPos(0, r, SWP_NOZORDER);
          lParam = MAKELPARAM(sizer::getWidth(), sizer::getHeight());
          }
        } while(false);
      break;
/**/
    case WM_GETMINMAXINFO:
      do {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;

        lpmmi->ptMaxPosition.x = -1;
        lpmmi->ptMaxPosition.y = -1;

        lpmmi->ptMaxSize.x = sizer::getWidth() + 2;
        lpmmi->ptMaxSize.y = sizer::getHeight() + 2;

#ifdef SIZEABLE
        lpmmi->ptMinTrackSize.x = sizer::getWidth() / 2;
        lpmmi->ptMinTrackSize.y = sizer::getHeight() / 2;
#else
        lpmmi->ptMinTrackSize.x = sizer::getWidth() + 2;
        lpmmi->ptMinTrackSize.y = sizer::getHeight() + 2;
#endif
        lpmmi->ptMaxTrackSize.x = sizer::getWidth() + 2;
        lpmmi->ptMaxTrackSize.y = sizer::getHeight() + 2;
        } while(false);
      break;
/**/
/*
    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        EndPaint(*this, &ps);
        return 0;
        } while(false);
*/
#ifndef SIZEABLE
    case WM_NCHITTEST:
      do {
        LRESULT result = PMainWin::windowProc(hwnd, message, wParam, lParam);
        int action = whereIs(result);
        switch(action) {
          case 1:
            return HTCAPTION;
          case -1:
            return HTCLIENT;
          default:
            return result;
          }
        } while(false);
#endif
    }
  return PMainWin::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static int whereIs(LRESULT pos)
{
  static int canMove;
  if(!canMove) {
    canMove = (int)sizer::getWidth() < GetSystemMetrics(SM_CXSCREEN) ? 1 : -1;
    if(canMove > 0) {
      LPCTSTR p = getString(ID_HIDE_BAR);
      if(p && 1 == _ttoi(p))
        canMove = 2;
      }
    }
  if(canMove < 0)
    return -1;

  switch(pos) {
    case HTCAPTION:     // In a title bar
      if(1 == canMove)
        return 1;
      break;
    case HTBOTTOM:      // In the lower horizontal border of a window
    case HTBOTTOMLEFT:  // In the lower-left corner of a window border
    case HTBOTTOMRIGHT: // In the lower-right corner of a window border
    case HTCLIENT:      // In a client area
    case HTGROWBOX:     // In a size box (same as HTSIZE)
    case HTLEFT:        // In the left border of a window
    case HTRIGHT:       // In the right border of a window
//    case HTSIZE:        // In a size box (same as HTGROWBOX)
    case HTTOP:         // In the upper horizontal border of a window
    case HTTOPLEFT:     // In the upper-left corner of a window border
    case HTTOPRIGHT:    // In the upper right corner of a window border
    case HTMENU:        // In a menu
      if(2 == canMove)
        return 1;
    default:
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
class PSVisorApp : public PAppl
{
  public:
    PSVisorApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow)  {   }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new mainWin(hInstance);
    }
};
//----------------------------------------------------------------------------
extern bool ONLY_ONE;
//----------------------------------------------------------------------------
#define SECOND_BEFORE_ABORT 10000
#define TIME_BETWEEN_REPEAT 200
#define REPEAT_BEFORE_ABORT (SECOND_BEFORE_ABORT / TIME_BETWEEN_REPEAT)
//----------------------------------------------------------------------------
extern bool checkCode();
extern void releaseSvBase();
//----------------------------------------------------------------------------
static bool useMappingByStdMsg()
{
 LPCTSTR p = getString(ID_USE_MAPPING_BY_STD_MSG);
 return !(p && 1 == _ttoi(p));
}
//----------------------------------------------------------------------------
static bool noMappedFile = false;
bool useMappedFilePrph1() { return !noMappedFile && useMappingByStdMsg(); }
//----------------------------------------------------------------------------
static bool noLoadDll = false;
bool canLoadDll() { return !noLoadDll; }
//----------------------------------------------------------------------------
static bool isFlag(LPCTSTR p, LPCTSTR match)
{
  while(*match) {
    if(*match != *p)
      return false;
    ++match;
    ++p;
    }
  return !*p || _T(' ') == *p;
}
//----------------------------------------------------------------------------
static void parseArg()
{
  int Argc;
  LPTSTR * Argv;

#ifdef UNICODE
  Argv = CommandLineToArgvW(GetCommandLineW(), &Argc );
#else
  Argc = __argc;
  Argv = __argv;
#endif

  while(--Argc > 0) {
    if(Argv[Argc][0] == _T('/') || Argv[Argc][0] == _T('-')) {
      LPCTSTR p = Argv[Argc] + 1;
      while(*p && (unsigned)*p <= _T(' '))
        ++p;
      if(isFlag(p, _T("nofmem")))
        noMappedFile = true;
      else if(isFlag(p, _T("nod")))
        noLoadDll = true;
      }
    }

#ifdef UNICODE
  ::GlobalFree((HGLOBAL)Argv);
#endif

}
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
  if(!checkCode()) {
    MessageBox(0, _T("File -[ ") NAME_SVBASE _T(" ]- mancante!!"), _T("Errore"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
    releaseSvBase();
    return 0;
    }
  InitCommonControls();

  parseArg();

  CoInitialize(0);

  int result= PSVisorApp(hInstance, SW_HIDE).run(_T("SuperVisor"));

  CoUninitialize();
  releaseSvBase();
void resetGlobalCacheLang();
void resetTxt();
void resetAlarmTxt();
void flushDll();

  // va eseguita prima di rilasciare i testi perché ne mantiene i riferimenti
  // che servono per salvare su file
  getGestAlarm()->resetAll();
  resetAlarmTxt();

  resetTxt();

  resetGlobalCacheLang();
  flushDll();
  return result;
}
//----------------------------------------------------------------------------
