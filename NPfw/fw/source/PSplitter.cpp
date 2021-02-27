//-------------------- PSplitter.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PSplitter.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PSplitter::~PSplitter()
{
}
//-----------------------------------------------------------
#define DIM_MIN 6
//-----------------------------------------------------------
static uint calcVal(uint full, uint perc)
{
  while(perc * full < perc) {
    full /= 10;
    perc /= 10;
    }
  perc *= full;
  return perc / 1000;
}
//-----------------------------------------------------------
static uint calcPerc(uint full, uint part)
{
  while(full && part * 1000 < full) {
    full /= 10;
    part /= 10;
    }
  part *= 1000;
  return full ? part / full : 0;
}
//-----------------------------------------------------------
bool PSplitter::create()
{
  resize();
  return true;
}
//-----------------------------------------------------------
#define COPY_DT(a) other->a = a
//-----------------------------------------------------------
void PSplitter::copyDataTo(PSplitter* other)
{
  if(!other)
    return;
  COPY_DT(Parent);
  COPY_DT(W1);
  COPY_DT(W2);
  COPY_DT(Dim1);
  COPY_DT(Dim2);
  COPY_DT(Perc);
  COPY_DT(Width);
  COPY_DT(Style);

  COPY_DT(lastPos);
  COPY_DT(stat);
}
//-----------------------------------------------------------
#define COPY_DF(a) a = other->a
//-----------------------------------------------------------
void PSplitter::copyDataFrom(PSplitter* other)
{
  if(!other)
    return;
  COPY_DF(Parent);
  COPY_DF(W1);
  COPY_DF(W2);
  COPY_DF(Dim1);
  COPY_DF(Dim2);
  COPY_DF(Perc);
  COPY_DF(Width);
  COPY_DF(Style);

  COPY_DF(lastPos);
  COPY_DF(stat);
}
//-----------------------------------------------------------
bool PSplitter::checkWidth(uint dimF)
{
  if(!dimF)
    return false;
  if(Dim1 < DIM_MIN && locked == PSL_NONE) {
    Dim1 = DIM_MIN + 1;
    Perc = calcPerc(dimF - Width, Dim1);
    }
  Dim2 = dimF - Width - Dim1;
  if(Dim2 < DIM_MIN && locked == PSL_NONE) {
    Dim2 = DIM_MIN + 1;
    Perc = 1000 - calcPerc(dimF - Width, Dim2);
    Dim1 = dimF - Width - Dim2;
    }
  return true;
}
//-----------------------------------------------------------
bool PSplitter::calcByPerc()
{
  PRect r;
  GetClientRect(*Parent, r);
  uint dim = PSW_HORZ == Style ? r.Width() : r.Height();
  if(dim < DIM_MIN * 2 + Width)
    return false;
  switch(locked) {
    case PSL_BOTH:
      if(Dim1 != -1 && Dim2 != -1)
        return false;
     // fall through, nel caso non sia ancora stata assegnata la dimensione
    case PSL_NONE:
      Dim1 = (dim - Width) * Perc / 1000;
      break;
    case PSL_SECOND:
      Dim1 = dim - Width - Dim2;
      break;
    }
  return checkWidth(dim);
}
//-----------------------------------------------------------
bool PSplitter::resize()
{
  if(calcByPerc()) {
    update();
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void PSplitter::setPerc(uint perc, bool refresh)
{
  Perc = perc;
  calcByPerc();
  if(refresh)
    update();
}
//-----------------------------------------------------------
void PSplitter::setDim1(uint width, bool refresh)
{
  PRect r;
  GetClientRect(*Parent, r);
  uint dim = PSW_HORZ == Style ? r.Width() : r.Height();

  Dim1 = width;
  checkWidth(dim);
  if(refresh)
    update();
}
//-----------------------------------------------------------
void PSplitter::setDim2(uint width, bool refresh)
{
  PRect r;
  GetClientRect(*Parent, r);
  uint dim = PSW_HORZ == Style ? r.Width() : r.Height();

  Dim1 = dim - Width - width;
  checkWidth(dim);
  if(refresh)
    update();
}
//-----------------------------------------------------------
void PSplitter::setSplitterWidth(uint width, bool refresh)
{
  Width = width;
  if(refresh)
    update();
}
//-----------------------------------------------------------
void PSplitter::setStyle(uint style, bool refresh)
{
  Style = style;
  calcByPerc();
  if(refresh)
    update();
}
//-----------------------------------------------------------
uint PSplitter::getDim2() const
{
#if 1
  return Dim2;
#else
  PRect r;
  GetClientRect(*Parent, r);
  if(PSW_HORZ == Style)
    return r.Width() - Dim1 - Width;
  return r.Height() - Dim1 - Width;
#endif
}
//-----------------------------------------------------------
void PSplitter::updateHorz()
{
  PRect r;
  GetClientRect(*Parent, r);
  PRect r1(r);
  r1.right = Dim1;
  W1->setWindowPos(0, r1, SWP_NOZORDER);
  r1.left = Dim1 + Width;
  r1.right = r.right;
  W2->setWindowPos(0, r1, SWP_NOZORDER);

  PRect r2(Dim1, 0, Dim1 + Width, r.bottom);
  InvalidateRect(*Parent, r2, 0);
}
//-----------------------------------------------------------
void PSplitter::updateVert()
{
  PRect r;
  GetClientRect(*Parent, r);
  PRect r1(r);
  r1.bottom = Dim1;
  W1->setWindowPos(0, r1, SWP_NOZORDER);
  r1.top = Dim1 + Width;
  r1.bottom = r.bottom;
  W2->setWindowPos(0, r1, SWP_NOZORDER);

  PRect r2(0, Dim1, r.right, Dim1 + Width);
  InvalidateRect(*Parent, r2, 0);
}
//-----------------------------------------------------------
void PSplitter::update()
{
  if(!W1 || !W2)
    return;
  if(PSW_HORZ == Style)
    updateHorz();
  else
    updateVert();
}
//-----------------------------------------------------------
void PSplitter::windowProcSplit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      if(PSD_NONE != getDrawStyle()) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*Parent, &ps);
        if(!hdc) {
          EndPaint(*Parent, &ps);
          break;
          }
        evPaint(hdc);
        EndPaint(*Parent, &ps);
        }
      break;

    case WM_SIZE:
      resize();
      break;

    case WM_CAPTURECHANGED:
      if(PSL_NONE == locked) {
        POINT pt = { lastPos, lastPos };
        evMouseUp(pt);
        }
      break;

    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      if(PSL_NONE == locked)
        evMouse(message, wParam, lParam);
      break;
    }
}
//-----------------------------------------------------------
void PSplitter::evMouse(UINT message, WPARAM wp, LPARAM lParam)
{
  if(ReadOnly)
    return;
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };

  switch(message) {
    case WM_MOUSEMOVE:
      if(PSW_HORZ == Style)
        evMouseMoveHorz(pt);
      else
        evMouseMoveVert(pt);
      break;

    case WM_LBUTTONDOWN:
      evMouseDown(pt);
      break;

    case WM_LBUTTONUP:
      evMouseUp(pt);
      break;
    }
}
//-----------------------------------------------------------
void PSplitter::evMouseUp(const POINT& pt)
{
  if(!stat)
    return;
  stat = 0;

  SetCursor(LoadCursor(0, IDC_ARROW));
  HDC hdc = GetDC(*Parent);
  int oldRop = SetROP2(hdc, R2_XORPEN); // R2_NOTXORPEN

  HPEN pen = CreatePen(PS_SOLID, Width, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  PRect r;
  GetClientRect(*Parent, r);

  if(PSW_HORZ == Style)
    drawXorHorz(hdc, lastPos, r);
  else
    drawXorVert(hdc, lastPos, r);

  DeleteObject(SelectObject(hdc, oldPen));
  SetROP2(hdc, oldRop);
  ReleaseDC(*Parent, hdc);
  ReleaseCapture();

  POINT pt2 = { lastPos, lastPos };
//  ScreenToClient(*Parent, &pt2);
  if(PSW_HORZ == Style) {
    Dim1 = pt2.x;
    Perc = calcPerc(r.Width(), Dim1);
    }
  else {
    Dim1 = pt2.y;
    Perc = calcPerc(r.Height(), Dim1);
    }
  update();
}
//-----------------------------------------------------------
void PSplitter::evMouseDown(const POINT& pt)
{
  // occorre verificare se si è sullo splitter?
  if(stat)
    return;
  stat = 1;
  SetCapture(*Parent);

  HDC hdc = GetDC(*Parent);
  int oldRop = SetROP2(hdc, R2_XORPEN); // R2_NOTXORPEN

  HPEN pen = CreatePen(PS_SOLID, Width, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  PRect r;
  GetClientRect(*Parent, r);

  if(PSW_HORZ == Style) {
    lastPos = pt.x;
    if(lastPos < DIM_MIN)
      lastPos = DIM_MIN;
    else if(lastPos > r.right - DIM_MIN - (int)Width)
      lastPos = r.right - DIM_MIN - Width;
    drawXorHorz(hdc, lastPos, r);
    SetCursor(LoadCursor(0, IDC_SIZEWE));
    }
  else {
    lastPos = pt.y;
    if(lastPos < DIM_MIN)
      lastPos = DIM_MIN;
    else if(lastPos > r.bottom - DIM_MIN - (int)Width)
      lastPos = r.bottom - DIM_MIN - Width;
    drawXorVert(hdc, lastPos, r);
    SetCursor(LoadCursor(0, IDC_SIZENS));
    }

  DeleteObject(SelectObject(hdc, oldPen));
  SetROP2(hdc, oldRop);
  ReleaseDC(*Parent, hdc);

}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PSplitter::drawXorHorz(HDC hdc, int pos, const PRect& r)
{
  POINT pt = { pos, pos };
  int x = pt.x + Width / 2;
  MoveToEx(hdc, x, 0, 0);
  LineTo(hdc, x, r.Height());
}
//-----------------------------------------------------------
void PSplitter::drawXorVert(HDC hdc, int pos, const PRect& r)
{
  POINT pt = { pos, pos };
  int y = pt.y + Width / 2;
  MoveToEx(hdc, 0, y, 0);
  LineTo(hdc, r.Width(), y);
}
//-----------------------------------------------------------
void PSplitter::evMouseMoveHorz(const POINT& pt)
{
  SetCursor(LoadCursor(0, IDC_SIZEWE));
  if(!stat)
    return;

  PRect r;
  GetClientRect(*Parent, r);
  if(pt.x < r.left + DIM_MIN)
    return;
  if(pt.x > r.right - DIM_MIN - (int)Width)
    return;
  if(pt.x == lastPos)
    return;

  int last = lastPos;
  lastPos = pt.x;

  HDC hdc = GetDC(*Parent);
  int oldRop = SetROP2(hdc, R2_XORPEN); // R2_NOTXORPEN

  HPEN pen = CreatePen(PS_SOLID, Width, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  drawXorHorz(hdc, last, r);
  drawXorHorz(hdc, lastPos, r);

  DeleteObject(SelectObject(hdc, oldPen));
  SetROP2(hdc, oldRop);
  ReleaseDC(*Parent, hdc);

}
//-----------------------------------------------------------
void PSplitter::evMouseMoveVert(const POINT& pt)
{
  SetCursor(LoadCursor(0, IDC_SIZENS));
  if(!stat)
    return;
  PRect r;
  GetClientRect(*Parent, r);
  if(pt.y < r.top + DIM_MIN)
    return;
  if(pt.y > r.bottom - DIM_MIN - (int)Width)
    return;
  if(pt.y == lastPos)
    return;

  int last = lastPos;
  lastPos = pt.y;

  HDC hdc = GetDC(*Parent);
  int oldRop = SetROP2(hdc, R2_XORPEN);
  HPEN pen = CreatePen(PS_SOLID, Width, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  drawXorVert(hdc, last, r);
  drawXorVert(hdc, lastPos, r);

  DeleteObject(SelectObject(hdc, oldPen));
  SetROP2(hdc, oldRop);
  ReleaseDC(*Parent, hdc);

}
//-----------------------------------------------------------
void PSplitter::evPaint(HDC hdc)
{
  if(PSW_HORZ == Style)
    evPaintHorz(hdc);
  else
    evPaintVert(hdc);
}
//-----------------------------------------------------------
static void evPaintLine(HDC hdc, const PRect& r, int width, COLORREF color)
{
  HPEN pen = CreatePen(PS_SOLID, width, color);
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.right, r.bottom);
  DeleteObject(SelectObject(hdc, oldPen));
}
//-----------------------------------------------------------
void PSplitter::evPaintHorz(HDC hdc)
{
  PRect r;
  GetClientRect(*Parent, r);

  if(PSD_FILL & getDrawStyle()) {
    int x = Dim1 + Width / 2;
    evPaintLine(hdc, PRect(x, r.top, x, r.bottom), Width, getBkgColor());
    }
  if(PSD_LEFT & getDrawStyle()) {
    int x = Dim1;
    evPaintLine(hdc, PRect(x, r.top, x, r.bottom), 1, getLightColor());
    }

  if(PSD_RIGHT & getDrawStyle()) {
    int x = Dim1 + Width - 1;
    evPaintLine(hdc, PRect(x, r.top, x, r.bottom), 1, getDarkColor());
    }
}
//-----------------------------------------------------------
void PSplitter::evPaintVert(HDC hdc)
{
  PRect r;
  GetClientRect(*Parent, r);

  if(PSD_FILL & getDrawStyle()) {
    int y = Dim1 + Width / 2;
    evPaintLine(hdc, PRect(r.left, y, r.right, y), Width, getBkgColor());
    }
  if(PSD_LEFT & getDrawStyle()) {
    int y = Dim1;
    evPaintLine(hdc, PRect(r.left, y, r.right, y), 1, getLightColor());
    }

  if(PSD_RIGHT & getDrawStyle()) {
    int y = Dim1 + Width - 1;
    evPaintLine(hdc, PRect(r.left, y, r.right, y), 1, getDarkColor());
    }
}
//-----------------------------------------------------------

