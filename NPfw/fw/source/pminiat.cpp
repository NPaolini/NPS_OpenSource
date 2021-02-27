//-------- Pminiat.cpp --------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "Pminiat.h"
#include "macro_utils.h"
#include "defin.h"
//---------------------------------------------------------
void drawRect(HDC hdc, const PRect& r, COLORREF color, HPEN usePen)
{
  HPEN pen;
  if(usePen)
    pen = usePen;
  else
    pen = CreatePen(PS_SOLID, 0, color);

  HGDIOBJ oldPen = SelectObject(hdc, pen);
#if 1
  POINT pt[5];
  pt[0].x = r.left;
  pt[0].y = r.top;

  pt[1].x = r.right - 1;
  pt[1].y = r.top;

  pt[2].x = r.right - 1;
  pt[2].y = r.bottom - 1;

  pt[3].x = r.left;
  pt[3].y = r.bottom - 1;

  pt[4].x = r.left;
  pt[4].y = r.top;

  Polyline(hdc, pt, SIZE_A(pt));

#else
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.left, r.bottom-1);
  LineTo(hdc, r.right-1, r.bottom-1);
  LineTo(hdc, r.right-1, r.top);
  LineTo(hdc, r.left, r.top);
#endif
  DeleteObject(SelectObject(hdc, oldPen));
}
//---------------------------------------------------------
Pminiat::Pminiat(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title, HINSTANCE hinst) :
  baseClass(parent, id, x, y, w, h, title, hinst), Selected(false)
{
}
//---------------------------------------------------------
Pminiat::Pminiat(PWin * parent, uint id, const PRect& r, LPCTSTR title, HINSTANCE hinst) :
  baseClass(parent, id, r, title, hinst), Selected(false)
{
}
//---------------------------------------------------------
Pminiat::~Pminiat()
{
  destroy();
}
//---------------------------------------------------------
void Pminiat::setSelected(bool set)
{
  Selected = set;
  InvalidateRect(*this, 0, 0);
}
//---------------------------------------------------------
static inline PclientScrollMiniat* getScrollMiniat(PWin* w)
{
  return getParentWin<PclientScrollMiniat>(w);
}
//---------------------------------------------------------
#define USE_MEMDC
//---------------------------------------------------------
LRESULT Pminiat::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
#ifdef USE_MEMDC
    case WM_ERASEBKGND:
      return 1;
#endif
    case WM_SIZE:
      mdc_custom.clear();
      break;
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(!hdc) {
          EndPaint(*this, &ps);
          return 0;
          }
#ifdef USE_MEMDC
        PRect r;
        GetClientRect(*this, r);
        if(r.Width() && r.Height()) {
          bool needDestroy = true;
          HBRUSH br = createBkg(needDestroy);
#if 1
          HDC mdcWork = mdc_custom.getMdc(this, hdc);
          FillRect(mdcWork, r, br);
          if(needDestroy)
            DeleteObject(HGDIOBJ(br));
          evPaint(mdcWork);
          paintSel(mdcWork);
          BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
#else
          HBITMAP  hBmpWork = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
          if(hBmpWork) {
            HDC mdcWork = CreateCompatibleDC(hdc);
            HGDIOBJ oldObj = SelectObject(mdcWork, hBmpWork);
            FillRect(mdcWork, r, br);
            if(needDestroy)
              DeleteObject(HGDIOBJ(br));
            evPaint(mdcWork);
            paintSel(mdcWork);
            BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
            DeleteObject(SelectObject(mdcWork, oldObj));
            DeleteDC(mdcWork);
            }
#endif
          }
#else
        evPaint(hdc);
        paintSel(hdc);
#endif
        EndPaint(*this, &ps);
        } while(false);
      return 0;

    case WM_LBUTTONUP:
      do {
        PclientScrollMiniat* par = getScrollMiniat(getParent());
        if(par)
          par->setSelected(Attr.id);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
void Pminiat::paintSel(HDC hdc)
{
  PRect rC;
  GetClientRect(*this, rC);

#if 1
  if(Selected) {
    HPEN pen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
    drawRect(hdc, rC, 0, pen);
    }
#else
  HPEN pen;
  if(Selected)
    pen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
  else
    pen = CreatePen(PS_SOLID, 0, 0);
//    pen = CreatePen(PS_DOT, 0, 0);

  drawRect(hdc, rC, 0, pen);
#endif
}
//---------------------------------------------------------
//---------------------------------------------------------
PclientScrollMiniat::~PclientScrollMiniat()
{
//  non è necessario, le window vengono distrutte dal genitore, cioè questa classe
//  flushPV(clients);
  destroy();
}
//---------------------------------------------------------
bool PclientScrollMiniat::create()
{
  uint totMiniat = nVert * nHorz;
  clients.setDim(totMiniat);
  clients[0] = makeMiniat(0, PRect(0, 0, 10, 10));
  SIZE sz = clients[0]->getBlockSize();
  PRect r(0, 0, sz.cx, sz.cy);

  clients[0]->Attr.x = 0;
  clients[0]->Attr.y = 0;
  clients[0]->Attr.w = sz.cx;
  clients[0]->Attr.h = sz.cy;

  uint t = 1;
  PRect r2(r);

  for(uint i = 1; i < nHorz; ++i, ++t) {
    r2.Offset(r2.Width(), 0);
    clients[t] = makeMiniat(t, r2);
    }

  r.Offset(0, r.Height());

  for(uint i = 1; i < nVert; ++i) {
    PRect r2(r);
    for(uint j = 0; j < nHorz; ++j, ++t) {
      clients[t] = makeMiniat(t, r2);
      r2.Offset(r2.Width(), 0);
      }
    r.Offset(0, r.Height());
    }

  if(!baseClass::create())
    return false;

  setSelected(clients[0]->getId());
  currSel = clients[0]->getId();
//  resize();
  return true;
}
//---------------------------------------------------------
SIZE PclientScrollMiniat::getBlockSize()
{
  if(!clients.getElem())
    return { 50, 50 };
  return clients[0]->getBlockSize();
}
//---------------------------------------------------------
void PclientScrollMiniat::setSelected(uint idc)
{
  if(currSel == idc)
    return;

  uint totMiniat = nVert * nHorz;
  int found = 0;
  for(uint i = 0; i < totMiniat && found < 2; ++i) {
    if(clients[i]->getId() == currSel) {
      clients[i]->setSelected(false);
      ++found;
      }
    else if(clients[i]->getId() == idc) {
      clients[i]->setSelected(true);
      ++found;
      }
    }
  currSel = idc;
}
//---------------------------------------------------------
/*
LRESULT PclientScrollMiniat::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
*/
//---------------------------------------------------------
void PclientScrollMiniat::resize()
{
  HWND hwnd = *this;
  if(!hwnd)
    return;
  SIZE sz = getBlockSize();
  SetWindowPos(hwnd, 0, 0, 0, sz.cx * nHorz, sz.cy * nVert, SWP_NOOWNERZORDER);
}
//---------------------------------------------------------

