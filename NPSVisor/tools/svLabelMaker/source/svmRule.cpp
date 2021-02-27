//------------------ svmRule.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmRule.h"
#include "common.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
void mdcCustom::clear()
{
  if(mdc) {
    SetBkMode(mdc, oldMode);
    SetBkColor(mdc, oldBkg);
    SetTextColor(mdc, oldColor);
    SetTextAlign(mdc, oldAlign);
    SelectObject(mdc, oldPen);
    SelectObject(mdc, oldBmp);
    SetGraphicsMode(mdc, GM_COMPATIBLE);
    DeleteDC(mdc);
    DeleteObject(hBmpTmp);
    DeleteObject(bkg);
    mdc = 0;
    }
}
//-----------------------------------------------------------
HDC mdcCustom::getMdc(PWin* owner, HDC hdc)
{
  PRect r;
  GetClientRect(*owner, r);
  if(!mdc && r.Width()) {
    mdc = CreateCompatibleDC(hdc);
    SetGraphicsMode(mdc, GM_ADVANCED);
    hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
    oldBmp = SelectObject(mdc, hBmpTmp);
    oldPen = SelectObject(mdc, pen);
    oldAlign = SetTextAlign(mdc, TA_BOTTOM | TA_CENTER);
    oldColor = SetTextColor(mdc, RGB(0, 0, 220));
    oldBkg = SetBkColor(mdc, cBkg);
    oldMode = SetBkMode(mdc, OPAQUE);
    bkg = CreateSolidBrush(cBkg);
    }
  if(mdc)
    FillRect(mdc, r, bkg);
  return mdc;
}
//-----------------------------------------------------------
bool PRule::create()
{
  if(!baseClass::create())
    return false;
  return true;
}
//-----------------------------------------------------------
void PRule::setStartLg(long start)
{
  if(start != Start) {
    Start = start;
    InvalidateRect(*this, 0, 0);
    }
}
//-----------------------------------------------------------
void PRule::setStartPx(long start)
{
  HDC hdc = GetDC(*this);
  SetMapMode(hdc, MM_LOMETRIC);
  POINT pt = { start, 0 };
  DPtoLP(hdc, &pt, 1);
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(*this, hdc);
  setStartLg(pt.x);
}
//-----------------------------------------------------------
void PRule::setZoom(svmManZoom::zoomX zoom, bool force)
{
  if(Zoom != zoom || force) {
    Zoom = zoom;
    InvalidateRect(*this, 0, 0);
    }
}
//-----------------------------------------------------------
LRESULT PRule::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      return 1;
    case WM_SIZE:
      Mdc.clear();
      break;
    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);
        if(!hdc) {
          EndPaint(*this, &Paint);
          return 0;
          }

        HDC mdc = Mdc.getMdc(this, hdc);
        if(mdc) {
          SetMapMode(mdc, MM_LOMETRIC);
          XFORM xForm;
          FLOAT z = 1;
          svmManZoom mZ;
          mZ.setCurrZoom(Zoom);
          xForm.eM11 = 1;
          xForm.eM22 = 1;
          if(isHorz) {
            mZ.calcToScreenH(z);
            xForm.eM11 = z;
            }
          else {
            mZ.calcToScreenV(z);
            xForm.eM22 = z;
            }

          xForm.eM12 = (FLOAT) 0.0;
          xForm.eM21 = (FLOAT) 0.0;

          xForm.eDx  = (FLOAT) 0.0;
          xForm.eDy  = (FLOAT) 0.0;
          SetWorldTransform(mdc, &xForm);
          evPaint(mdc);
          xForm.eM11 = (FLOAT) 1.0;
          xForm.eM22 = (FLOAT) 1.0;
          SetWorldTransform(mdc, &xForm);
          SetMapMode(mdc, MM_TEXT);
          PRect rect2(Paint.rcPaint);
          BitBlt(hdc, rect2.left, rect2.top, rect2.Width(), rect2.Height(), mdc, rect2.left, rect2.top, SRCCOPY);
          }
        EndPaint(hwnd, &Paint);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PRule::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#define DEF_STEP 100
#define VERT_ANGLE 2700
//-----------------------------------------------------------
#define ADD_POINT(buff, v) \
    wsprintf(buff, _T("%d.%02d"), (v) / 100, (v) % 100)
//-----------------------------------------------------------
void PRuleX::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  int h = r.Height();
  LONG step = DEF_STEP;
  LONG start = Start % DEF_STEP;
  int start2 = Start - start;

  int end = r.Width() + start;
  for(int i = -start; i < end; i += step) {
    MoveToEx(hdc, i, 0, 0);
    LineTo(hdc, i, h);
    }
  h /= 2;
  for(int i = step / 2 - start; i < end; i += step) {
    MoveToEx(hdc, i, 0, 0);
    LineTo(hdc, i, h);
    }
  TCHAR buff[64];
  HFONT fnt = D_FONT_ORIENT(h, 0, 0, 0, _T("arial"));
  HGDIOBJ oldF = SelectObject(hdc, fnt);
  h *= 2;
  if(Zoom > svmManZoom::zOne) {
    for(int i = -start; i < end; i += step, start2 += DEF_STEP) {
      ADD_POINT(buff, start2);
      ExtTextOut(hdc, i, h, 0 , 0, buff, _tcslen(buff), 0);
      }
    }
  else {
    step *= 2;
    int i = -start;
    if(!((start2 / DEF_STEP) & 1)) {
      start2 += DEF_STEP;
      i += step / 2;
      }
    for(; i < end; i += step, start2 += DEF_STEP * 2) {
      ADD_POINT(buff, start2);
      ExtTextOut(hdc, i, h, 0 , 0, buff, _tcslen(buff), 0);
      }
    }
  DeleteObject(SelectObject(hdc, oldF));
}
//-----------------------------------------------------------
void PRuleY::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  int w = r.Width();
  LONG step = DEF_STEP;
  LONG start = Start % DEF_STEP;
  int start2 = Start - start;

  int end = r.Height() - start;
  for(int i = start; i > end; i -= step) {
    MoveToEx(hdc, 0, i, 0);
    LineTo(hdc, w, i);
    }
  w /= 2;
  for(int i = start + step / 2; i > end; i -= step) {
    MoveToEx(hdc, 0, i, 0);
    LineTo(hdc, w, i);
    }

  TCHAR buff[64];
  HFONT fnt = D_FONT_ORIENT(w, 0, VERT_ANGLE, 0, _T("arial"));
  HGDIOBJ oldF = SelectObject(hdc, fnt);
  w *= 2;
  if(Zoom > svmManZoom::zOne) {
    for(int i = start; i > end; i -= step, start2 += DEF_STEP) {
      ADD_POINT(buff, start2);
      ExtTextOut(hdc, r.Width(), i, 0 , 0, buff, _tcslen(buff), 0);
      }
    }
  else {
    step *= 2;
    int i = start;
    if(!((start2 / DEF_STEP) & 1)) {
      start2 += DEF_STEP;
      i -= step / 2;
      }
    for(; i > end; i -= step, start2 += DEF_STEP * 2) {
      ADD_POINT(buff, start2);
      ExtTextOut(hdc, r.Width(), i, 0 , 0, buff, _tcslen(buff), 0);
      }
    }
  DeleteObject(SelectObject(hdc, oldF));
}
//-----------------------------------------------------------
