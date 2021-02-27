//----------- PSliderWin.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "headerMsg.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "PSliderWin.h"
#include "ppanel.h"
#include "p_util.h"
#include "pTraspBitmap.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
REG_WM(CUSTOM_MSG_SLIDER);
//----------------------------------------------------------------------------
PSliderWin::PSliderWin(PWin * parent, uint id, int x, int y, int w, int h, const infoSlider& is,
            LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, title, hinst),
      IS(is), Value(0), Left(0), Right(0), LeftSel(0), RightSel(0), onDragging(dNone),
      onAnchor(dNone), Ratio(1.0), BitsHide(0)
{
  init();
}
//----------------------------------------------------------------------------
PSliderWin::PSliderWin(PWin * parent, uint id, const PRect& r, const infoSlider& is,
            LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, id, r, title, hinst),
      IS(is), Value(0), Left(0), Right(0), LeftSel(0), RightSel(0), onDragging(dNone),
      onAnchor(dNone), Ratio(1.0), BitsHide(0)
{
  init();
}
//----------------------------------------------------------------------------
PSliderWin::~PSliderWin ()
{
  destroy();
}
//----------------------------------------------------------------------------
void PSliderWin::init()
{
  Attr.style |= WS_CHILD;
  szMargin.cx = 0;
  szMargin.cy = 0;
  uint ids[] = { IS.idSkin, IS.idSlider, IS.idLeftSlider, IS.idRightSlider };

  for(uint i = 0; i < SIZE_A(ids); ++i) {
    if(ids[i]) {
      iSlBmp[i].id = ids[i];
      iSlBmp[i].bmp = new PBitmap(ids[i], getHInstance());
      }
    else
      BitsHide |= 1 << i;
    }
  Curs[dNone] = LoadCursor(NULL, IDC_ARROW);
  Curs[dLeft] = LoadCursor(getHInstance(), MAKEINTRESOURCE(IDC_CURS_SLIDER_SX));
  Curs[dRight] = LoadCursor(getHInstance(), MAKEINTRESOURCE(IDC_CURS_SLIDER_DX));
  Curs[dValue] = LoadCursor(getHInstance(), MAKEINTRESOURCE(IDC_CURS_SLIDER));
}
//----------------------------------------------------------------------------
bool PSliderWin::create()
{
  if(!baseClass::create())
    return false;
  PBitmap* bmp = getBitmap(IS.idSkin);
  if(!bmp)
    return false;
  SIZE sz = bmp->getSize();
  if(!sz.cx || !sz.cy)
    return false;

  SetWindowPos(*this, 0, 0, 0, sz.cx, sz.cy, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
  return true;
}
//----------------------------------------------------------------------------
PBitmap* PSliderWin::getBitmap(uint id)
{
  if(!id)
    return 0;
  for(uint i = 0; i < SIZE_A(iSlBmp); ++i) {
    if(id == iSlBmp[i].id)
      return iSlBmp[i].bmp;

    }
  return 0;
}
//----------------------------------------------------------------------------
void PSliderWin::setLimits(DWORD left, DWORD right)
{
  Right = right;
  Left = left;
  calcRatio();
}
//----------------------------------------------------------------------------
void PSliderWin::setSelections(DWORD left, DWORD right)
{
  RightSel = right;
  LeftSel = left;
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
void PSliderWin::setValue(DWORD val)
{
  val = min(max(val, Left), Right);

  DWORD oldPix = (DWORD)(Ratio * Value + 0.5);
  DWORD newPix = (DWORD)(Ratio * val + 0.5);
  if(oldPix != newPix) {
    PRect r = getRectObject(IS.idSlider, Value);
    r.Inflate(2, 2);
    if(r.left < 0)
      r.left = 0;
    if(r.top < 0)
      r.top = 0;
    InvalidateRect(*this, r, false);
    r = getRectObject(IS.idSlider, val);
    r.Inflate(2, 2);
    if(r.left < 0)
      r.left = 0;
    if(r.top < 0)
      r.top = 0;
    InvalidateRect(*this, r, false);
    }
  Value = val;
}
//----------------------------------------------------------------------------
static SIZE maxWidth(const SIZE& sz1, const SIZE& sz2)
{
  SIZE sz = { max(sz1.cx, sz2.cx), max(sz1.cy, sz2.cy) };
  return sz;
}
//----------------------------------------------------------------------------
void PSliderWin::calcRatio()
{
  double len = Right - Left;
  if(!len)
    return;
  PBitmap* bmp = getBitmap(IS.idSkin);
  if(!bmp)
    return;
  szMargin.cx = 0;
  szMargin.cy = 0;

  bmp = getBitmap(IS.idSlider);

  if(bmp)
    szMargin = bmp->getSize();
  bmp = getBitmap(IS.idLeftSlider);
  if(bmp)
    szMargin = maxWidth(szMargin, bmp->getSize());

  bmp = getBitmap(IS.idRightSlider);
  if(bmp)
    szMargin = maxWidth(szMargin, bmp->getSize());

  szMargin.cx /= 2;
  szMargin.cy /= 2;

  PRect r = getPaintRect();
  SIZE szWin = { r.Width(), r.Height() };

  Ratio = (IS.Format & infoSlider::sVertCurrTop) ? szWin.cy : szWin.cx;
  Ratio /= len;
}
//----------------------------------------------------------------------------
LRESULT PSliderWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      evMouse(message, wParam, lParam);
      break;

    case WM_ERASEBKGND:
      evPaintBkg((HDC)wParam);
      return 1;

    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(hdc)
          evPaint(hdc, toBool(ps.fErase));
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
PRect PSliderWin::getRectObject(uint idc, DWORD offset)
{
  PRect r;
  PBitmap* bmp = getBitmap(idc);
  if(!bmp)
    return r;

  SIZE sz = bmp->getSize();
  offset -= Left;
  int t = (int)(Ratio * offset + 0.5);

  bool isSlider = idc == IS.idSlider;

  // rovescia il controllo da fare
  if(IS.Format & infoSlider::sHorzCurrBottom)
    isSlider = !isSlider;

  r =   getPaintRect();

  switch(IS.Format) {
    case infoSlider::sHorzCurrTop:
      r.left += t - sz.cx / 2;
      r.right = r.left + sz.cx;
      if(isSlider)
        r.bottom = r.top + sz.cy;
      else
        r.top = r.bottom - sz.cy;
      break;

    case infoSlider::sVertCurrTop:
      r.top += t - sz.cy / 2;
      r.bottom = r.top + sz.cy;
      if(isSlider)
        r.right = r.left + sz.cx;
      else
        r.left = r.right - sz.cx;
      break;

    case infoSlider::sHorzCurrBottom:
      r.left += t - sz.cx / 2;
      r.right = r.left + sz.cx;
      if(!isSlider)
        r.bottom = r.top + sz.cy;
      else
        r.top = r.bottom - sz.cy;
      break;

    case infoSlider::sVertCurrBottom:
      r.top += t - sz.cy / 2;
      r.bottom = r.top + sz.cy;
      if(!isSlider)
        r.right = r.left + sz.cx;
      else
        r.left = r.right - sz.cx;
      break;
    }
  return r;
}
//-----------------------------------------------------------
bool PSliderWin::isOverObject(uint idc, DWORD offset, const POINT& pt)
{
  PRect r = getRectObject(idc, offset);
  return r.IsIn(pt);
}
//-----------------------------------------------------------
void PSliderWin::pointToValue(const POINT& pt, const SIZE& sz)
{
  int pos;
  if(IS.Format & infoSlider::sVertCurrTop) {
    pos = pt.y - szMargin.cy;
    }
  else {
    pos = pt.x - szMargin.cx;
    }
  if(pos < 0)
    pos = 0;

  pos = (int)(pos / Ratio);
  pos += Left;
  if((DWORD)pos < Left)
    pos = Left;
  else if((DWORD)pos > Right)
    pos = Right;


  switch(onDragging) {
    case dLeft:
      LeftSel = min((DWORD)pos, RightSel);
      break;

    case dRight:
      RightSel = max((DWORD)pos, LeftSel);
      break;
    case dValue:
      Value = min((DWORD)pos, Right);
      break;
    }
}
//-----------------------------------------------------------
void PSliderWin::evMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  switch(message) {
    case WM_MOUSEMOVE:
      evMouseMove(pt);
      SetFocus(*this);
      break;

    case WM_LBUTTONDOWN:
      evMouseLDown(pt);
      break;

    case WM_LBUTTONUP:
      evMouseLUp(pt);
      break;
    }
}
//-----------------------------------------------------------
PRect PSliderWin::getOldRect()
{
  PRect r;
  uint idc = IS.idSlider;
  DWORD val = Value;
  switch(onDragging) {
    case dLeft:
      idc = IS.idLeftSlider;
      val = LeftSel;
      break;
    case dRight:
      idc = IS.idRightSlider;
      val = RightSel;
      break;
    case dValue:
      break;
    default:
      return r;
    }
  return getRectObject(idc, val);
}
//-----------------------------------------------------------
void PSliderWin::setPointer(uint type)
{
  switch(type) {
    case dNone:
    case dRight:
    case dLeft:
    case dValue:
      SetCursor(Curs[type]);
      break;
    }
  onAnchor = type;
}
//-----------------------------------------------------------
void PSliderWin::evMouseMove(const POINT& pt)
{
  if(dNone == onDragging) {
    if(isVisible(slVal) && isOverObject(IS.idSlider, Value, pt))
      setPointer(dValue);
    else {
      bool onLeft = isVisible(slLeft) && isOverObject(IS.idLeftSlider, LeftSel, pt);
      bool onRight = isVisible(slRight) && isOverObject(IS.idRightSlider, RightSel, pt);
      if(onLeft || onRight) {
        if(onLeft && onRight) {
          if(LeftSel < (Left + Right) / 2)
            setPointer(dRight);
          else
            setPointer(dLeft);
          }
        else if(onLeft)
          setPointer(dLeft);
        else
          setPointer(dRight);
        }
      else if(onAnchor != dNone)
        setPointer(dNone);
      }
    return;
    }

  setPointer(onAnchor);
  UINT idc = 0;
  switch(onAnchor) {
    case dRight:
      idc = IS.idRightSlider;
      break;
    case dLeft:
      idc = IS.idLeftSlider;
      break;
    case dValue:
      idc = IS.idSlider;
      break;
    }

  SIZE sz = { 0, 0 };
  PBitmap* bmp = getBitmap(idc);
  if(bmp)
    sz = bmp->getSize();

  PRect r = getOldRect();
  r.Inflate(2, 2);
  InvalidateRect(*this, r, false);
  pointToValue(pt, sz);
  r = getOldRect();
  r.Inflate(2, 2);
  InvalidateRect(*this, r, false);

  WPARAM wp;
  LPARAM lp;
  switch(onDragging) {
    case dValue:
      wp = MAKEWPARAM(CM_MOVE_SLIDER, Attr.id);
      lp = (LPARAM)Value;
      break;
    case dLeft:
      wp = MAKEWPARAM(CM_MOVE_LEFT_SLIDER, Attr.id);
      lp = (LPARAM)LeftSel;
      break;
    case dRight:
      wp = MAKEWPARAM(CM_MOVE_RIGHT_SLIDER, Attr.id);
      lp = (LPARAM)RightSel;
      break;
    default:
      return;
    }

  PostMessage(*getParent(), WM_CUSTOM_MSG_SLIDER, wp, lp);
}
//-----------------------------------------------------------
void PSliderWin::evMouseLDown(const POINT& pt)
{
  if(onAnchor == dNone)
    return;

  SetCapture(*this);
  onDragging = onAnchor;
  setPointer(onAnchor);
}
//-----------------------------------------------------------
void PSliderWin::evMouseLUp(const POINT& pt)
{
  if(onAnchor == dNone)
    return;

  WPARAM wp;
  LPARAM lp;
  switch(onDragging) {
    case dValue:
      wp = MAKEWPARAM(CM_END_MOVE_SLIDER, Attr.id);
      lp = (LPARAM)Value;
      break;
    case dLeft:
      wp = MAKEWPARAM(CM_END_MOVE_LEFT_SLIDER, Attr.id);
      lp = (LPARAM)LeftSel;
      break;
    case dRight:
      wp = MAKEWPARAM(CM_END_MOVE_RIGHT_SLIDER, Attr.id);
      lp = (LPARAM)RightSel;
      break;
    default:
      return;
    }

  PostMessage(*getParent(), WM_CUSTOM_MSG_SLIDER, wp, lp);
  InvalidateRect(*this, 0, 0);
  ReleaseCapture();
  onDragging = dNone;
}
//----------------------------------------------------------------------------
void PSliderWin::paintSlider(HDC hdc, const PRect& r, uint idc, DWORD format, DWORD val)
{
  PBitmap* bmp = getBitmap(idc);
  if(!bmp)
    return;
  POINT pt = { 0, 0 };

  SIZE szSlider = bmp->getSize();
  DWORD pos = (DWORD)((val - Left) * Ratio);
  switch(format) {
    case infoSlider::sHorzCurrBottom:
      pt.y = r.bottom - szSlider.cy;
    case infoSlider::sHorzCurrTop:
      pt.x = r.left + pos - szSlider.cx / 2;
      break;

    case infoSlider::sVertCurrBottom:
      pt.x = r.right - szSlider.cx;
    case infoSlider::sVertCurrTop:
      pt.y = r.bottom - pos + szSlider.cy / 2;
      break;
    }
  PTraspBitmap(this, bmp, pt).Draw(hdc);
}
//----------------------------------------------------------------------------
void PSliderWin::evPaintBkg(HDC hdc)
{
  PBitmap* bmp = getBitmap(IS.idSkin);
  if(!bmp)
    return;
  POINT pt = { 0, 0 };
  bmp->draw(hdc, pt);
}
//----------------------------------------------------------------------------
PRect PSliderWin::getPaintRect()
{
  PRect r;
  PBitmap* bmp = getBitmap(IS.idSkin);
  if(!bmp)
    return r;
  SIZE sz = bmp->getSize();
  if(IS.Format & infoSlider::sVertCurrTop)
    return PRect(0, szMargin.cy, sz.cx, sz.cy - szMargin.cy);
  return PRect(szMargin.cx, 0, sz.cx - szMargin.cx, sz.cy);
}
//----------------------------------------------------------------------------
bool isEmpty(const PRect& r)
{
  return !r.Width() || !r.Height();
}
//----------------------------------------------------------------------------
void PSliderWin::evPaint(HDC hdc, bool eraseBkg)
{
  PBitmap* bmp = getBitmap(IS.idSkin);
  if(!bmp)
    return;
  POINT pt = { 0, 0 };
  if(!eraseBkg)
    bmp->draw(hdc, pt);

  PRect r = getPaintRect();

  PRect rSel;
  PRect r2(r);
  if(IS.Format & infoSlider::sVertCurrTop) {
    uint dim = (r2.Width() / 8) * 3;
    r2.left += dim;
    r2.right -= dim;

    rSel = r2;
    rSel.top = (int)((LeftSel - Left) * Ratio);
    rSel.bottom = (int)((RightSel - Left) * Ratio);
    rSel.Offset(0, szMargin.cy);
    rSel.Inflate(-1, 0);
    }
  else {
    uint dim = (r2.Height() / 8) * 3;
    r2.top += dim;
    r2.bottom -= dim;

    rSel = r2;
    rSel.left = (int)((LeftSel - Left) * Ratio);
    rSel.right = (int)((RightSel - Left) * Ratio);
    rSel.Offset(szMargin.cx, 0);
    rSel.Inflate(0, -1);
    }

  if(!isVisible(slRight) && !isVisible(slLeft))
    rSel.left = rSel.right;
  else {
    if(!isVisible(slLeft))
      rSel.left = r2.left;
    else if(!isVisible(slRight))
      rSel.right = r2.right;
    }
  PPanel(r2, IS.Bar, PPanel::DN_FILL).draw(hdc);
  if(!isEmpty(rSel))
    PPanel(rSel, IS.selBar, PPanel::FILL).draw(hdc);

  DWORD format = IS.Format ^ infoSlider::sHorzCurrBottom;

  if(isVisible(slLeft))
    paintSlider(hdc, r, IS.idLeftSlider, format, LeftSel);

  if(isVisible(slRight))
    paintSlider(hdc, r, IS.idRightSlider, format, RightSel);

  if(isVisible(slVal))
    paintSlider(hdc, r, IS.idSlider, IS.Format, Value);
}
//----------------------------------------------------------------------------
bool PSliderWin::isVisible(uint which)
{
  return !toBool(BitsHide & (1 << which));
}
//----------------------------------------------------------------------------
void PSliderWin::hideShow(uint bitHide, uint bitShow)
{
  DWORD old = BitsHide;
  BitsHide &= ~bitShow;
  BitsHide |= bitHide;
  if(old != BitsHide)
    InvalidateRect(*this, 0, 0);
}
