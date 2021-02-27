//--------------------- svmMainClient.cpp ------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "svmMainClient.h"
#include "svmMainWorkArea.h"
#include "common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//-----------------------------------------------------------
childScrollContainer::childScrollContainer(PWin* parent, uint id, HINSTANCE hInstance) :
      baseClass(parent, id, PscrollInfo(), PRect(), 0, hInstance)
{
  Attr.style |= WS_CLIPCHILDREN;
}
//-----------------------------------------------------------
childScrollContainer::~childScrollContainer()
{
  destroy();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool childScrollContainer::create()
{
  if(!baseClass::create())
    return false;
  return true;
}
//-----------------------------------------------------------
void childScrollContainer::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
}
//-----------------------------------------------------------
PRect lgToDp(PRect r)
{
  HDC hdc = GetDC(0);
  SetMapMode(hdc, MM_LOMETRIC);
  LPtoDP(hdc, (LPPOINT)(LPRECT)r, 2);
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(0, hdc);
  return r;
}
//-----------------------------------------------------------
PRect getLogRectBase()
{
  dataProject& dp = getDataProject();
  PRect r(0, 0, dp.getPageWidth(), dp.getPageHeight());
//  PRect r(0, 0, 2100, -2970);
  return r;
}
//-----------------------------------------------------------
PclientScrollWin* childScrollContainer::makeClient()
{
  PRect r = getLogRectBase();
  svmManZoom mZ;
  mZ.setCurrZoom(svmManZoom::zOne);
  mZ.calcToScreen(r);

  return new childScrollWorkArea(this, ID_CLIENT_SCROLL, lgToDp(r));
}
//-----------------------------------------------------------
void childScrollContainer::resize_test()
{
  client->resetScroll();
  PostMessage(*this, WM_SEND_DIM, 0, 0);
}
//-----------------------------------------------------------
static
bool diff(const PRect& r1, const PRect& r2)
{
  return r1.Width() != r2.Width() || r1.Height() != r2.Height();
}
//-----------------------------------------------------------
void childScrollContainer::manageDim()
{
  PRect rc;
  GetWindowRect(*client, rc);
  rc.MoveTo(0, 0);
  PRect r;
  bool needRecalc = false;
  do {
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
  bool canPan = PscrollInfo::stNone != IScroll.getScrollType();
  PostMessage(*client, WM_HAS_SCROLL, canPan, 0);
}
//-----------------------------------------------------------
void childScrollContainer::managePan(POINT* delta)
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
void childScrollContainer::setScrollPos(POINT* pt)
{
  switch(IScroll.getScrollType()) {
    case PscrollInfo::stHorz:
      SetScrollPos(*this, SB_HORZ, pt->x, TRUE);
      break;
    case PscrollInfo::stBoth:
      SetScrollPos(*this, SB_HORZ, pt->x, TRUE);
      // fall through
    case PscrollInfo::stVert:
      SetScrollPos(*this, SB_VERT, pt->y, TRUE);
      break;
    }
}
//-----------------------------------------------------------
LRESULT childScrollContainer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize_test();
      return true;
    case WM_SEND_DIM:
      manageDim();
      break;
    case WM_SEND_PAN:
      managePan((POINT*)lParam);
      break;
    case WM_HAS_SCROLL:
      return PscrollInfo::stNone != IScroll.getScrollType();
    case WM_SEND_SCROLL_POS:
      setScrollPos((POINT*)lParam);
      break;
    case WM_ERASEBKGND:
      SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNSHADOW));
      break;
    case WM_MOUSEWHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
void childScrollContainer::evMouseWheel(short delta, short x, short y)
{
  if(!(PscrollInfo::stVert & IScroll.getScrollType()))
    return;
  long maxShow = maxShowVert;
  dataProject& dp = getDataProject();
  dp.calcToScreenV(maxShow);
  int tD = MulDiv(delta, maxShow, 5 * WHEEL_DELTA);
  int curr = GetScrollPos(*this, SB_VERT);
  curr -= tD;
  SendMessage(*this, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, curr), 0);
}
//----------------------------------------------------------------------------
