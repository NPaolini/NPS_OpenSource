//----- pPscrollContainer.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "PscrollContainer.h"
#include "macro_utils.h"
//---------------------------------------------------------
PscrollContainer::PscrollContainer(PWin * parent, uint id, const PscrollInfo& iScroll, int x, int y, int w, int h,
      LPCTSTR title, HINSTANCE hinst) :
  baseClass(parent, id, x, y, w, h, title, hinst), IScroll(iScroll), maxShowVert(0), maxShowHorz(0), client(0),
  disabledScroll(false)
{
  init();
}
//---------------------------------------------------------
PscrollContainer::PscrollContainer(PWin * parent, uint id, const PscrollInfo& iScroll, const PRect& r, LPCTSTR title, HINSTANCE hinst) :
  baseClass(parent, id, r, title, hinst), IScroll(iScroll), maxShowVert(0), maxShowHorz(0), client(0), disabledScroll(false)
{
  init();
}
//---------------------------------------------------------
PscrollContainer::~PscrollContainer()
{
  destroy();
}
//---------------------------------------------------------
void PscrollContainer::init()
{
  uint style = 0;
  switch(IScroll.getScrollType()) {
    case PscrollInfo::stVert:
      style = WS_VSCROLL;
      break;
    case PscrollInfo::stBoth:
      style = WS_VSCROLL;
      // fall through
    case PscrollInfo::stHorz:
      style |= WS_HSCROLL;
      break;
    }
  Attr.style |= WS_CHILD | style | WS_BORDER;
}
//---------------------------------------------------------
void PscrollContainer::resize()
 {
  PscrollInfo::scrollType type = IScroll.getScrollType();
  if(PscrollInfo::stNone == type)
    return;

  PRect r;
  GetClientRect(*this, r);
  if(!r.Width())
    return;
  SIZE sz = client->getBlockSize();

  if(sz.cx)
    maxShowHorz = r.Width() / sz.cx;

  if(sz.cy)
    maxShowVert = r.Height() / sz.cy;

  SCROLLINFO si;
  memset(&si, 0, sizeof(si));
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_RANGE;
  si.nMin = 0;
  si.nMax = IScroll.getMaxVert();
  si.nPage = UINT(maxShowVert * GetDeviceCaps(GetWindowDC(*this), LOGPIXELSY) / 96.0f);

  switch(IScroll.getScrollType()) {
    case PscrollInfo::stVert:
      SetScrollInfo(*this, SB_VERT, &si, true);
      break;
    case PscrollInfo::stBoth:
      SetScrollInfo(*this, SB_VERT, &si, true);
      // fall through
    case PscrollInfo::stHorz:
      si.nMax = IScroll.getMaxHorz();
      si.nPage = maxShowHorz;
      SetScrollInfo(*this, SB_HORZ, &si, true);
      break;
    }
}
//---------------------------------------------------------
bool PscrollContainer::create()
{
  client = makeClient();
  if(!baseClass::create())
    return false;
  resize();
  return true;
}
//---------------------------------------------------------
LRESULT PscrollContainer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_SIZE:
      resize();
      break;
    case WM_MOUSEWHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_HSCROLL:
      if(!disabledScroll)
        evHScrollBar((HWND)lParam, short(LOWORD(wParam)), short(HIWORD(wParam)));
      break;

    case WM_VSCROLL:
      if(!disabledScroll)
        evVScrollBar((HWND)lParam, short(LOWORD(wParam)), short(HIWORD(wParam)));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
void PscrollContainer::evMouseWheel(short delta, short x, short y)
{
  if(!(PscrollInfo::stVert & IScroll.getScrollType()))
    return;
  int tD = MulDiv(delta, maxShowVert / 2, WHEEL_DELTA);
//  int tD = MulDiv(delta, maxShowVert, 5 * WHEEL_DELTA);
  int curr = GetScrollPos(*this, SB_VERT);
  curr -= tD;
  SendMessage(*this, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, curr), 0);
}
//---------------------------------------------------------
void PscrollContainer::checkMarginX(int& pos)
{
  if(pos < 0)
    pos = 0;
  else if((uint)pos > IScroll.getMaxHorz() - maxShowHorz)
    pos = IScroll.getMaxHorz() - maxShowHorz;
}
//---------------------------------------------------------
void PscrollContainer::checkMarginY(int& pos)
{
  if(pos < 0)
    pos = 0;
  else if((uint)pos > IScroll.getMaxVert() - maxShowVert)
    pos = IScroll.getMaxVert() - maxShowVert;
}
//---------------------------------------------------------
void PscrollContainer::evHScrollBar(HWND child, int flags, int pos)
{
  int curr = GetScrollPos(*this, SB_HORZ);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
#if 0
    default:
      if(pos)
        break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
#else
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = IScroll.getMaxHorz() - maxShowHorz;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - maxShowHorz;
      break;
    case SB_PAGERIGHT:
      pos = curr + maxShowHorz;
      break;
#endif
    }
  checkMarginX(pos);
  SetScrollPos(*this, SB_HORZ, pos, TRUE);
  client->setHPos(pos);
}
//---------------------------------------------------------
void PscrollContainer::evVScrollBar(HWND child, int flags, int pos)
{
  int curr = GetScrollPos(*this, SB_VERT);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
#if 0
    default:
      if(pos)
        break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
#else
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = IScroll.getMaxVert() - maxShowVert;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - maxShowVert;
      break;
    case SB_PAGERIGHT:
      pos = curr + maxShowVert;
      break;
#endif
    }
  checkMarginY(pos);
  SetScrollPos(*this, SB_VERT, pos, TRUE);
  client->setVPos(pos);
}
//---------------------------------------------------------
void PscrollContainer::resetScroll()
{
  switch(IScroll.getScrollType()) {
    case PscrollInfo::stVert:
      SetScrollPos(*this, SB_VERT, 0, TRUE);
      break;
    case PscrollInfo::stBoth:
      SetScrollPos(*this, SB_VERT, 0, FALSE);
      // fall through
    case PscrollInfo::stHorz:
      SetScrollPos(*this, SB_HORZ, 0, TRUE);
      break;
    }
}
//---------------------------------------------------------
void PclientScrollWin::setPos()
{
  SIZE sz = getBlockSize();
  PRect r(0, 0, sz.cx, sz.cy);
  r.MoveTo(-(int)FirstH * sz.cx, -(int)FirstV * sz.cy);
  setWindowPos(0, r, SWP_NOSIZE | SWP_NOZORDER);
}
//---------------------------------------------------------
void PclientScrollWin::resetScroll()
{
  FirstH = 0;
  FirstV = 0;
  setPos();
  PscrollContainer* par = getParentWin<PscrollContainer>(getParent());
  if(par)
    par->resetScroll();
}
//---------------------------------------------------------
LRESULT PclientScrollWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
bool PclientScrollWin::create()
{
  if(!baseClass::create())
    return false;
  resize();
  return true;
}
