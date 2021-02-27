//------------------- extMonitor.cpp ------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <Windowsx.h>
#include "extMonitor.h"
//----------------------------------------------------------------------------
#define COORDS_PATH_MON _T("coordsMon")
#define SIZE_PATH_MON   _T("sizeMon")
//-------------------------------------------------------------------
class myclientWin : public clientWin
{
  private:
    typedef clientWin baseClass;
  public:
    myclientWin(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst) { Attr.style |= WS_DISABLED; }

    ~myclientWin() { destroy(); }

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//---------------------------------------------------------
LRESULT myclientWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_RBUTTONUP:
      PostMessage(*getParent(), message, wParam, lParam);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
class myP_TimerLed : public P_TimerLed
{
  private:
    typedef P_TimerLed baseClass;
  public:
    myP_TimerLed(PWin * parent, const manageTimer& mt, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, mt, id, x, y, w, h, title, hinst) { Attr.style |= WS_DISABLED; }

    ~myP_TimerLed() { destroy(); }

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//---------------------------------------------------------
LRESULT myP_TimerLed::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_RBUTTONUP:
      PostMessage(*getParent(), message, wParam, lParam);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define REDIM_FHD(v) (v * 2 / 10)
//-----------------------------------------------------------
extMonitor::extMonitor(PWin* parent, const manageTimer& mt) :
     baseClass(parent, 0, 0, 0, 0, 0, 0), Split(0), tml(0), cw(0)
{
  Attr.style = WS_POPUP | WS_BORDER | WS_VISIBLE | WS_SIZEBOX | WS_CLIPCHILDREN;
  Attr.x = 10;
  Attr.y = 10;
  Attr.w = REDIM_FHD(1920);
  Attr.h = REDIM_FHD(1080);
  setCaption(_T("Escape Room"));
  Split = new PSplitter(this, 0, 0, 50, 0, PSplitter::PSW_VERT);
  tml = new myP_TimerLed(this, mt, 1111);
  cw = new myclientWin(this, 1112);
  Split->setW1(tml);
  Split->setW2(cw);
  Split->setReadOnly(true);
}
//---------------------------------------------------------
extMonitor::~extMonitor()
{
  destroy();
  delete Split;
}
//---------------------------------------------------------
void extMonitor::refreshInfo()
{
  InvalidateRect(*cw, 0, 0);
}
//---------------------------------------------------------
void extMonitor::reset() { tml->reset(); }
//---------------------------------------------------------
bool extMonitor::create()
{
  DWORD coords = 0;
  getKeyParam(COORDS_PATH_MON, &coords);
  if(coords && (DWORD)-1 != coords) {
    Attr.x = (short)LOWORD(coords);
    Attr.y = (short)HIWORD(coords);
    getKeyParam(SIZE_PATH_MON, &coords);
    if(coords && (DWORD)-1 != coords) {
      Attr.w = (short)LOWORD(coords);
      Attr.h = (short)HIWORD(coords);
      }
    }

  if(!baseClass::create())
    return false;
  if((DWORD)-1 == coords) {
    PRect r;
    GetWindowRect(*this, r);
    int w = GetSystemMetrics(SM_CXSCREEN);
    r.MoveTo(w + r.left, r.top);
    setWindowPos(0, r, SWP_NOZORDER);
    ShowWindow(*this, SW_MAXIMIZE);
    }
  return Split->create();
}
//---------------------------------------------------------
void extMonitor::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.style |= CS_DBLCLKS;
}
//---------------------------------------------------------
extern bool isMaximized(HWND hwnd);
//---------------------------------------------------------
void extMonitor::toggleFullScreen()
{
  if(isMaximized(*this))
    ShowWindow(*this, SW_RESTORE);
  else
    ShowWindow(*this, SW_MAXIMIZE);
}
//---------------------------------------------------------
LRESULT extMonitor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      if(isMaximized(hwnd)) {
        setKeyParam(COORDS_PATH_MON, (DWORD)-1);
        }
      else {
        PRect r;
        GetWindowRect(hwnd, r);
        DWORD t = MAKELONG(r.left, r.top);
        setKeyParam(COORDS_PATH_MON, t);
        t = MAKELONG(r.Width(), r.Height());
        setKeyParam(SIZE_PATH_MON, t);
        }
      break;
    case WM_RBUTTONUP:
      rightClick();
      break;
    case WM_NCHITTEST:
      do {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        PRect r;
        GetClientRect(*this, r);
        MapWindowPoints(*this, HWND_DESKTOP, (LPPOINT)(LPRECT)r, 2);
        if(xPos < r.left || xPos > r.right || yPos < r.top + 30 || yPos >r.bottom)
            break;
        return HTCAPTION;
        } while(false);
      break;
    case WM_LBUTTONDBLCLK:
      toggleFullScreen();
      return 0;
    }
  if (Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void extMonitor::setBackground(LPCTSTR path)
{
  cw->setBackground(path);
}
//----------------------------------------------------------------------------
extern bool isMaximized(HWND hwnd);
//----------------------------------------------------------------------------
void extMonitor::rightClick()
{
  bool isMax = isMaximized(*this);
  menuInfo mi[] = {
      {  getStringOrDef(IDT_M_ADD_ROW, _T("Full Screen")), isMax },
      {  getStringOrDef(IDT_M_ADD_COL, _T("Restore")), !isMax },
      };
  uint nItem = SIZE_A(mi);
  uint res = getResultMenu(this, mi, nItem);
  switch(res) {
    case 0:
      ShowWindow(*this, SW_MAXIMIZE);
      break;
    case 1:
      ShowWindow(*this, SW_RESTORE);
      break;
    }
}
