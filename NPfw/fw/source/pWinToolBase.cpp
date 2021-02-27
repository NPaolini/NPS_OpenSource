//-------------------- pWinToolBase.cpp ---------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
//#include "resource.h"
#include "pWinToolBase.h"
#include "pToolBar.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PWinToolBase::PWinToolBase(PWin * parent, LPCTSTR title, UINT idBkg, HINSTANCE hInstance) :
      baseClass(parent, title, hInstance), Bkg(idBkg, hInstance), active(true)
{
  Attr.style = WS_OVERLAPPED | WS_CAPTION | WS_BORDER | WS_POPUP;
//  Attr.style &= ~WS_VISIBLE;
//  Attr.style = WS_POPUP | WS_CAPTION | WS_VISIBLE | WS_BORDER;
//  Attr.exStyle = WS_EX_PALETTEWINDOW | WS_EX_TOOLWINDOW;
  Attr.exStyle = WS_EX_TOOLWINDOW;
  Attr.x = 10;
  Attr.y = 10;
  Attr.w = 360;
  Attr.h = 200;
}
//-----------------------------------------------------------
PWinToolBase::~PWinToolBase()
{
  destroy();
}
//-----------------------------------------------------------
LRESULT PWinToolBase::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      {
      HDC hdc = (HDC)wParam;
      RECT r;
      GetClientRect(hwnd, &r);
      POINT pt = { 0, 0 };
      SIZE sz = { r.right, r.bottom };
      Bkg.draw(hdc, pt, sz);
      return 1;
      }
    case WM_ACTIVATEAPP:
      active = toBool(wParam);
      PostMessage(*this, WM_NCACTIVATE, active, 0);
      break;

    case WM_NCACTIVATE:
      if(active)
        wParam = TRUE;
      break;
/*
    case WM_MOUSEACTIVATE:
      if(!GetActiveWindow())
        SetActiveWindow(*getAppl()->getMainWindow());
      return MA_NOACTIVATE;
*/
    case WM_NCHITTEST:
      if(active) {
        LRESULT result = PWin::windowProc(hwnd, message, wParam, lParam);
        if(HTCLIENT != result)
          SetFocus(*getParent());
        return result;
        }
      break;

    case WM_MOVE:
      SetFocus(*getParent());
//      SetFocus(*getAppl()->getMainWindow());
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
