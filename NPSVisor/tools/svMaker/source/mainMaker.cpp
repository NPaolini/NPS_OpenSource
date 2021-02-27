//-------------------- mainMaker.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <shlobj.h>
//-----------------------------------------------------------
#include "svmMainClient.h"
#include "resource.h"
#include "pBitmap.h"
#include "sizer.h"
//----------------------------------------------------------------------------
#ifdef _WIN64
  #define NAME_SVBASE _T("npsvBase64.dll")
  #pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #define NAME_SVBASE _T("npsvBase.dll")
  #pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
//----------------------------------------------------------------------------
//#define USE_ANIMATED_WINDOW
//----------------------------------------------------------------------------
#ifdef USE_ANIMATED_WINDOW
//-----------------------------------------------------------
extern void closeWithAnim(HWND hwnd, bool check);
//----------------------------------------------------------------------------
class PSplashWin : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PSplashWin(PWin* parent, PBitmap& bmp) : baseClass(parent), Bmp(bmp), idTimer(0)
    {
      SIZE sz = Bmp.getSize();
      Attr.x = (sizer::getWidth() - sz.cx) / 2;
      Attr.y = (sizer::getHeight() - sz.cy) / 2;
      Attr.w = sz.cx;
      Attr.h = sz.cy;
      Attr.exStyle = WS_EX_TOPMOST;
      Attr.style |= WS_POPUP;
      Attr.style &= ~WS_VISIBLE;
    }
    ~PSplashWin() { destroy(); }
    bool create();
    void startTimer(DWORD time);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void onTimer();
  private:
    PBitmap& Bmp;
    int idTimer;
};
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#ifndef WS_EX_LAYERED
  #define WS_EX_LAYERED 0x80000
  #define LWA_COLORKEY 1
  #define LWA_ALPHA 2
#endif
//----------------------------------------------------------------------------
typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);
//----------------------------------------------------------------------------
#define INCR_STEP 5
#define MAX_STEP_VAL 280
#define MIN_STEP_VAL 40
//----------------------------------------------------------------------------
class PSplashWin : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PSplashWin(PWin* parent, PBitmap& bmp) : baseClass(parent), Bmp(bmp), idTimer(0),
    factor(10), incr(INCR_STEP), pSetLayeredWindowAttributes(0), hDLL(0)
    {
      hDLL = LoadLibrary(_T("user32"));
      if(hDLL)
        pSetLayeredWindowAttributes = (PSLWA) GetProcAddress(hDLL,"SetLayeredWindowAttributes");
      SIZE sz = Bmp.getSize();
      Attr.x = (sizer::getWidth() - sz.cx) / 2;
      Attr.y = (sizer::getHeight() - sz.cy) / 2;
      Attr.w = sz.cx;
      Attr.h = sz.cy;
      Attr.exStyle = WS_EX_TOPMOST | (pSetLayeredWindowAttributes ? WS_EX_LAYERED : 0);
      Attr.style |= WS_POPUP;
      Attr.style &= ~WS_VISIBLE;
    }
    ~PSplashWin() { if(hDLL)   FreeLibrary(hDLL); destroy(); }
    bool create();
    void startTimer(DWORD time);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void onTimer();
  private:
    PBitmap& Bmp;
    int idTimer;
    int factor;
    int incr;
    PSLWA pSetLayeredWindowAttributes;
    HMODULE hDLL;
};
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
LRESULT PSplashWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_TIMER:
      if(idTimer == wParam)
        onTimer();
      break;
    case WM_DESTROY:
      if(idTimer)
        KillTimer(hwnd, idTimer);
      break;

    case WM_CHAR:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
#ifdef USE_ANIMATED_WINDOW
      closeWithAnim(hwnd, true);
#endif
      DestroyWindow(*this);
      break;

    case WM_PRINT:
    case WM_PRINTCLIENT:
      do {
        POINT pt = { 0, 0 };
        SIZE sz = { Attr.w, Attr.h };
        Bmp.draw((HDC)wParam, pt, sz);
        } while(false);
      break;

    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);

        POINT pt = { 0, 0 };
        SIZE sz = { Attr.w, Attr.h };
        Bmp.draw(hdc, pt, sz);
        EndPaint(hwnd, &Paint);
        } while(false);
      break;

    case WM_ERASEBKGND:
      return 1;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);

}
//-----------------------------------------------------------
void PSplashWin::startTimer(DWORD time)
{
  if(idTimer)
    return;
  SetTimer(*this, idTimer = 1, time, 0);
}
//-----------------------------------------------------------
#define REPEAT_TIMER 500
//-----------------------------------------------------------
void PSplashWin::onTimer()
{
  static int count;
  if(++count >= REPEAT_TIMER) {
    PostMessage(*this, WM_CHAR, 0, 0);
    return;
    }
#ifndef USE_ANIMATED_WINDOW
  if(!pSetLayeredWindowAttributes)
    return;
  if(factor >= MAX_STEP_VAL)
    incr = -INCR_STEP;
  else if(factor <= MIN_STEP_VAL)
    incr = INCR_STEP;
  factor += incr;
  int fact = min(factor, 255);
  pSetLayeredWindowAttributes (*this, RGB(255, 255, 255), fact, LWA_COLORKEY|LWA_ALPHA);
#endif
}
//-----------------------------------------------------------
#define USE_REGION
//#define USE_HOLE_REGION
//-----------------------------------------------------------
#ifdef USE_HOLE_REGION
static void subtractRegion(HRGN hRgn, const PRect& r)
{
  uint radius = rand() %  10;
  uint x = (rand() - radius) % r.Width() + r.left;
  uint y = (rand() - radius) % r.Height() + r.top;
  bool ell = rand() & 7;
  HRGN hr;
  if(ell)
    hr = CreateEllipticRgn(x - radius, y - radius, x + radius, y + radius);
  else
    hr = CreateRectRgn(x - radius, y - radius, x + radius, y + radius);
  CombineRgn(hRgn, hRgn, hr, RGN_DIFF);
  DeleteObject(hr);
}
#endif
//-----------------------------------------------------------
bool PSplashWin::create()
{
  if(!baseClass::create())
    return false;
#ifdef USE_REGION
  PRect r;
  GetWindowRect(*this, r);
  r.MoveTo(0, 0);
  HRGN hRgn = CreateRoundRectRgn(0, 0, r.right, r.bottom, r.right / 15, r.bottom / 15);
  if(hRgn) {
#ifdef USE_HOLE_REGION
    srand(GetTickCount());
    PRect r1(0, 0, 70, 150);
    PRect r2(70, 0, r.right, 90);
    PRect r3(420, 90, r.right, 170);
    PRect r4(0, 225, r.right, r.bottom);

    for(int i = 0; i < 10; ++i) {
      subtractRegion(hRgn, r1);
      subtractRegion(hRgn, r3);
      }
    for(int i = 0; i < 40; ++i) {
      subtractRegion(hRgn, r2);
      subtractRegion(hRgn, r4);
      }
#endif
    SetWindowRgn(*this, hRgn, true);
    }
#endif
#ifdef USE_ANIMATED_WINDOW
  if(isWinXP_orLater()) {
      uint playTime = 500;
      AnimateWindow(*this, playTime, AW_BLEND | AW_SLIDE);
      InvalidateRect(*this, 0, 1);
      UpdateWindow(*this);
      }
    else
#endif
      ShowWindow(*this, SW_SHOWNORMAL);

  return true;
}
//-----------------------------------------------------------
static
struct forThread
{
  HINSTANCE hInstance;
  HANDLE hEvent;
  DWORD time;
} logoThread;
//-----------------------------------------------------------
void startTimerForSplash(DWORD time)
{
  logoThread.time = time / REPEAT_TIMER;
  SetEvent(logoThread.hEvent);
  while(logoThread.hEvent)
    Sleep(20);
}
//-----------------------------------------------------------
unsigned FAR PASCAL showLogo(void*)
{
  PAppl* ap = 0;
  while(true) {
    ap = getAppl();
    if(ap)
      break;
    Sleep(100);
    }

  PBitmap bmp(LOGO_IMAGE, ap->getHinstance(), RT_RCDATA);
  if(bmp.isValid()) {
    PSplashWin sw(0/*parent*/, bmp);
    sw.create();
    while(sw.getHandle()) {
      DWORD result = MsgWaitForMultipleObjects(1, &logoThread.hEvent, 0, INFINITE, QS_ALLINPUT);
      if(WAIT_OBJECT_0 == result)
        sw.startTimer(logoThread.time);

      else if(WAIT_OBJECT_0 + 1 != result)
        break;
      ap->pumpMessages();
      }
    }
  CloseHandle(logoThread.hEvent);
  logoThread.hEvent = 0;
  SetForegroundWindow(*ap->getMainWindow());
  SetFocus(*ap->getMainWindow());
  return 0;
}
//-----------------------------------------------------------
static
LPCTSTR get_ClassName()
{
  return _T("npsvMaker_ClassWin");
}
//-----------------------------------------------------------
class makerWin : public PscrollContainer
{
  private:
    typedef PscrollContainer baseClass;

  public:
    makerWin(LPCTSTR title, HINSTANCE hInstance) :
      baseClass(0, 0, PscrollInfo(), PRect(), title, hInstance)
    {
      setFlag(pfMainWindow);
      Attr.style &= ~WS_CHILD;
      Attr.style |= WS_OVERLAPPEDWINDOW | WS_VISIBLE;
      Attr.exStyle |= WS_EX_APPWINDOW;

      bkg = CreateHatchBrush(HS_CROSS, RGB(0, 0, 0));
      Attr.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
      Attr.x = 1;
      Attr.y = 1;
      Attr.w = 20;
      Attr.h = 20;
    }

    ~makerWin() { DeleteObject(bkg); }

    bool create();

    virtual bool preProcessMsg(MSG& msg)
    {
      if(!TranslateAccelerator(getHandle(), haccel, &msg))
        return false;
      return toBool(IsDialogMessage(getHandle(), &msg));
    }
  protected:
    HBRUSH bkg;
    HACCEL haccel;

    virtual PclientScrollWin* makeClient() { return new svmMainClient(this, getHInstance()); }
    virtual LPCTSTR getClassName() const { return ::get_ClassName(); }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
  private:
    void manageDim(dimWin* dwin);
    void managePan(POINT* delta);
};
//-----------------------------------------------------------
#define CLASS_NAME_SVLANG _T("svLang_Class")
//-----------------------------------------------------------
static LPCTSTR getMsgWarn()
{
  return _T("Il programma svLang è in esecuzione,\r\n")
         _T("poiché può interferire col svMaker è\r\n")
         _T("preferibile non averli entrambi in funzione,\r\n")
         _T("o almeno che non lavorino sullo stesso progetto.");
}
//-----------------------------------------------------------
static void checkRunningSvLang(PWin* owner)
{
  if(FindWindow(CLASS_NAME_SVLANG, 0))
    MessageBox(*owner, getMsgWarn(), _T("Attenzione"), MB_OK | MB_ICONINFORMATION);
}
//-----------------------------------------------------------
bool makerWin::create()
{
  if(!baseClass::create())
    return false;

  haccel = LoadAccelerators(getHInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  checkRunningSvLang(this);
  return true;
}
//-----------------------------------------------------------
static
bool diff(const PRect& r1, const PRect& r2)
{
  return r1.Width() != r2.Width() || r1.Height() != r2.Height();
}
//-----------------------------------------------------------
void makerWin::manageDim(dimWin* dwin)
{
  if(IsIconic(*this))
    return;
  PRect& rc = dwin->clientRect;
  PRect r2;
  GetWindowRect(*this, r2);
  svmMainClient* mc = (svmMainClient*)client;
  if(mc->needRecalcArea()) {
    mc->clearRecalcArea();
    if(!IsZoomed(*this)) {
      IScroll.setScrollType(PscrollInfo::stNone);
      PRect work;
      SystemParametersInfo(SPI_GETWORKAREA, 0,(LPRECT)work, 0);
      PRect r(rc);
      DWORD dwStyle = GetWindowLong(*this, GWL_STYLE);
      dwStyle &= ~(WS_VSCROLL | WS_HSCROLL);
      SetWindowLong(*this, GWL_STYLE, dwStyle);
      AdjustWindowRect(r, dwStyle, FALSE);
      r.MoveTo(0, 0);
      if(r.bottom > work.bottom)
        r.bottom = work.bottom;
      if(r.right > work.right)
        r.right = work.right;
      setWindowPos(0, r, SWP_NOZORDER| SWP_FRAMECHANGED);
      }
    }
  bool needRecalc = false;
  do {
    PRect r;
    GetClientRect(*this, r);

    DWORD style = GetWindowLong(*this, GWL_STYLE);
    style &= ~(WS_VSCROLL | WS_HSCROLL);
    uint scroolType = 0;
    if(r.bottom < rc.bottom) {
      style |= WS_VSCROLL;
      scroolType = PscrollInfo::stVert;
      }
    if(r.right < rc.right) {
      style |= WS_HSCROLL;
      scroolType |= PscrollInfo::stHorz;
      }
    IScroll.setScrollType((PscrollInfo::scrollType)scroolType);
    IScroll.setMaxVert(rc.bottom + 10);
    IScroll.setMaxHorz(rc.right + 10);

    SetWindowLong(*this, GWL_STYLE, style);
    SetWindowPos(*this, 0,0,0,0,0, SWP_FRAMECHANGED| SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER );
    PRect r2;
    GetClientRect(*this, r2);
    needRecalc = diff(r, r2);
    } while(needRecalc);

  resize();
}
//-----------------------------------------------------------
void makerWin::managePan(POINT* delta)
{
  disabledScroll = true;
  int currX = GetScrollPos(*this, SB_HORZ);
  int currY = GetScrollPos(*this, SB_VERT);
  switch(IScroll.getScrollType()) {
    case PscrollInfo::stHorz:
      currX += delta->x;
      checkMarginX(currX);
      SetScrollPos(*this, SB_HORZ, currX, TRUE);
      break;
    case PscrollInfo::stBoth:
      currX += delta->x;
      checkMarginX(currX);
      SetScrollPos(*this, SB_HORZ, currX, TRUE);
      // fall through
    case PscrollInfo::stVert:
      currY += delta->y;
      checkMarginY(currY);
      SetScrollPos(*this, SB_VERT, currY, TRUE);
      break;
    default:
      disabledScroll = false;
      return;
    }
  client->setScrollPos(currX, currY);
  disabledScroll = false;
}
//-----------------------------------------------------------
LRESULT makerWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SYSCOMMAND:
      switch(wParam) {
        case SC_CLOSE:
          ((svmMainClient*)client)->closeProgr();
          return 0;
        }
      break;
    case WM_COMMAND:
      PostMessage(*client, message, wParam, lParam);
      break;
    case WM_SEND_DIM:
      manageDim((dimWin*)lParam);
      break;
    case WM_SEND_PAN:
      managePan((POINT*)lParam);
      break;
    case WM_HAS_SCROLL:
      return PscrollInfo::stNone != IScroll.getScrollType();
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      if(GetForegroundWindow() != *this)
        SetForegroundWindow(*this);
      break;

    case WM_SIZE:
      PostMessage(*client, WM_SIZE, wParam, lParam);
      break;
    case WM_GETMINMAXINFO:
      do {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMinTrackSize.x = 300;
        lpmmi->ptMinTrackSize.y =150;
        } while(false);
      break;

    case WM_ERASEBKGND:
      SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNSHADOW));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void makerWin::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = bkg;
}
//-----------------------------------------------------------
class makerApp : public PAppl
{
  public:
    makerApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new makerWin(title, hInstance);
    }
};
//-----------------------------------------------------------
#define USE_KEY_FILE
#ifdef USE_KEY_FILE
#include <shlwapi.h>
#include <shlobj.h>
#include "p_checkkey.h"
//----------------------------------------------------------------------------
#define CUR_SV_BASE_VER 0x07000001
#define DIM_FAKE_JMP 8
#define DIM_KEY_CRYPT DIM_svKEY
//----------------------------------------------------------------------------
static HMODULE hsvBase;
//----------------------------------------------------------------------------
static BYTE decrypted_buff[DIM_KEY_CRYPT * 2 + 2];
static bool Crypted;
bool isCrypted() { return Crypted; }
LPCBYTE getDecryptCode() { return decrypted_buff; }
//----------------------------------------------------------------------------
#define USE_SHOW_TRACE
#ifdef USE_SHOW_TRACE
  #define SHOW_TRACE(a, b) show_trace(a, b)
  bool show_trace(LPCTSTR msg1, LPCTSTR msg2)
  {
    TCHAR t[4096];
    wsprintf(t, _T("%s - %s"), msg1, msg2);
    MessageBox(0, t, _T("Error"), MB_OK | MB_ICONSTOP);
    return false;
  }
#else
  #define SHOW_TRACE(a, b) false
#endif
//----------------------------------------------------------------------------
static FARPROC getProc(HINSTANCE hI, LPCSTR name)
{
  // prima prova con l'underscore
  FARPROC f = GetProcAddress(hI, name);
  if(!f)
    // prova senza underscore
    f = GetProcAddress(hI, name + 1);
  return f;
}
//----------------------------------------------------------------------------
bool hasKey();
//----------------------------------------------------------------------------
static bool performCheckCode(HMODULE hMod)
{
  Crypted = false;
  bool success = false;
  while(!success) {
    int offs = (GetTickCount() & 0xfc) | 8;
    typedef void (*SV_Fake)(bool&);
    BYTE cryptCode[DIM_KEY_CRYPT + 2 + DIM_FAKE_JMP];
    DWORD k = GetTickCount();
    *(LPDWORD)cryptCode = k;

    typedef void (*SV_Base)(LPBYTE buff);
    SV_Base bs = (SV_Base) getProc(hMod, "_SV_Ver");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Ver)"), _T("failed"));
      break;
      }
    bs(cryptCode);
    *(LPDWORD)cryptCode ^= k;
    if(CUR_SV_BASE_VER > *(LPDWORD)cryptCode) {
      SHOW_TRACE(_T("cryptCode Ver"), _T("failed"));
      break;
      }
    DWORD ver = *(LPDWORD)cryptCode;

    bs = (SV_Base) getProc(hMod, "_SV_Base");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Base)"), _T("failed"));
      break;
      }

    LPDWORD pdw = (LPDWORD)cryptCode;
    k = GetTickCount();
    pdw[0] = k;
    pdw[2] = offs ^ ver;
    pdw[3] = k;
    bs(cryptCode);
    pdw[0] ^= k;
    pdw[1] ^= ~k;

#if 0
// per creare la chiave vuota
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i)
      cryptCode[i] = 0x20;
#endif
    crypt(cryptCode, decrypted_buff, KEY_BASE_CRYPT, DIM_KEY_CRYPT);
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i) {
      if(decrypted_buff[i] != 0x20) {
        Crypted = true;
        break;
        }
      }
    success = true;
    break;
    }
  return success;
}
//----------------------------------------------------------------------------
bool hasKey()
{
  if(hsvBase)
    return true;
  bool success = false;
  while(!success) {
    hsvBase = LoadLibrary(NAME_SVBASE);
    if(!hsvBase) {
      SHOW_TRACE(_T("loadlibrary"), _T("failed"));
      break;
      }
    success = performCheckCode(hsvBase);
    break;
    }
  return success;
}
//---------------------------------------------------------------------
void releaseSvBase()
{
  if(!hsvBase)
    return;
  FreeLibrary(hsvBase);
  hsvBase = 0;
}
//---------------------------------------------------------------------
bool getKeyCode(LPBYTE key)
{
  ZeroMemory(key, DIM_svKEY);
  if(Crypted)
    memcpy(key, getDecryptCode(), DIM_svKEY);
  return true;
}
//---------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
#define SECOND_BEFORE_ABORT 10000
#define TIME_BETWEEN_REPEAT 200
#define REPEAT_BEFORE_ABORT (SECOND_BEFORE_ABORT / TIME_BETWEEN_REPEAT)
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef USE_KEY_FILE
  if(!hasKey()) {
    releaseSvBase();
    return EXIT_FAILURE;
    }
#endif
  HANDLE hMutex = CreateMutex(0, true, get_ClassName());
  if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
    int i;
    for(i = 0; i < REPEAT_BEFORE_ABORT; ++i) {
      HWND hWnd = ::FindWindow(get_ClassName(), NULL);
      if(hWnd) {
        SetForegroundWindow(hWnd);
        break;
        }
      SleepEx(TIME_BETWEEN_REPEAT, false);
      }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
    }

  InitCommonControls();

  DWORD idThread;

  logoThread.hEvent = CreateEvent(0, true, false, 0);
  logoThread.hInstance = hInstance;

  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)showLogo, 0, 0, (LPDWORD)&idThread);

  if(hThread)
    CloseHandle(hThread);

  int result = makerApp(hInstance, nCmdShow).run(_T("SVisorMaker"));

  if(hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    }
#ifdef USE_KEY_FILE
  releaseSvBase();
#endif
  return result;
}
//-------------------------------------------------
