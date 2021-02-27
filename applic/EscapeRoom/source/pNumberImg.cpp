//-------------------- pNumberImg.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pNumberImg.h"
#include "pPanel.h"
#include "clientWin.h"
#include "extMonitor.h"
//-----------------------------------------------------------
#define MUL_DOUBLE(i,d) (i = (uint)(i * d))
//-----------------------------------------------------------
void pNumberImg::draw(HDC hdc, int number, const POINT& pt, const SIZE& sz)
{
  if(!Info.img.getElem())
    return;
  SIZE sz_single = Info.img[0]->getSize();
  SIZE sz_all = sz_single;
  SIZE sz_point = { 0, 0 };
  int totDec = 0;
  if(Info.nDec) {
    sz_point = Info.img[POINT_OFFSET_IMG]->getSize();
    totDec = Info.totNumber / Info.nDec - 1 + toBool(Info.totNumber % Info.nDec);
    }
  sz_all.cx *= Info.totNumber;
  if(Info.sign)
    sz_all.cx += sz_single.cx;
  sz_all.cx += sz_point.cx * totDec;
  if(!sz_all.cx || !sz_all.cy)
    return;
  double scaleX = (double)(sz.cx - 2) / sz_all.cx;
  double scaleY = (double)(sz.cy - 2) / sz_all.cy;
  SIZE sz_final = sz;
  if(scaleX > scaleY) {
    scaleX = scaleY;
    sz_final.cx = (UINT)(sz_all.cx * scaleY);
    }
  else if(scaleX < scaleY) {
    scaleY = scaleX;
    sz_final.cy = (UINT)(sz_all.cy * scaleX);
    }

  MUL_DOUBLE(sz_single.cx, scaleX);
  MUL_DOUBLE(sz_single.cy, scaleY);
  MUL_DOUBLE(sz_point.cx, scaleX);
  MUL_DOUBLE(sz_point.cy, scaleY);

  POINT pt_final = pt;

  if(-1 == pt.x)
    pt_final.x = (sz.cx - sz_final.cx) / 2;
  else if(pt.x < 0)
    pt_final.x = sz.cx - sz_final.cx + pt.x;
  if(-1 == pt.y)
    pt_final.y = (sz.cy - sz_final.cy) / 2;
  else if(pt.y < 0)
    pt_final.y = sz.cy - sz_final.cy + pt.y;
  if(pt_final.x < 0)
    pt_final.x = 0;
  if(pt_final.y < 0)
    pt_final.y = 0;

  int n = number;
  bool transp = (DWORD)-1 == Info.background;

  if(!transp) {
    PRect r(0, 0, sz_final.cx, sz_final.cy);
    r.Offset(pt_final.x, pt_final.y);
    PPanel(r, Info.background, PPanel::DN_FILL).draw(hdc);
    }
  POINT pt1 = { sz_final.cx - sz_single.cx + pt_final.x + 1, pt_final.y + 1 };
  PRect r(0, 0, sz_single.cx, sz_single.cy);
  r.Offset(pt1.x, pt1.y);
  if(Info.nDec) {
    for(int j = 0; j < totDec; ++j) {
      for(uint i = 0; i < Info.nDec; ++i) {
        int v = n % 10;
        n /= 10;
        Info.img[v]->drawTransp(hdc, r);
        r.Offset(-sz_single.cx, 0);
        }
      r.Offset(sz_single.cx - sz_point.cx, 0);
      r.right = r.left + sz_point.cx;
      Info.img[POINT_OFFSET_IMG]->drawTransp(hdc, r);
      r.right = r.left + sz_single.cx;
      r.Offset(-sz_single.cx, 0);
      }
    }
  for(uint i = Info.nDec * totDec; i < Info.totNumber; ++i) {
    int v = n % 10;
    n /= 10;
    if(!n && !Info.nDec && i)
      break;
    Info.img[v]->drawTransp(hdc, r);
    r.Offset(-sz_single.cx, 0);
    if(!n)
      break;
    }
}
//-----------------------------------------------------------
P_TimerLed::~P_TimerLed()
{
  destroy();
  flushPV(Info.img);
  delete Img;
}
//-----------------------------------------------------------
#define MAX_INIT 6000
#define FIRST_BMP IDB_LED_0
//-----------------------------------------------------------
static bool loadPng(LPTSTR path, uint ix)
{
  LPCTSTR finalPoint = _T("Point");
  LPCTSTR finalSign = _T("Sign");
  if(ix <= 9)
    wsprintf(path, _T("led\\led%d.png"), ix);
  else if(POINT_OFFSET_IMG == ix)
    wsprintf(path, _T("led\\ledPoint.png"));
  else
    wsprintf(path, _T("led\\ledSign.png"));
  return P_File::P_exist(path);
}
//-------------------------------------------------------------------
static int getTotNumber(int v)
{
  int t = 0;
  while(v > 0) {
    ++t;
    v /= 10;
    }
  return t;
}
//-----------------------------------------------------------
void P_TimerLed::reset()
{
  int currVal = ManTimer.getValue();
  Info.totNumber = getTotNumber(currVal);
  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
bool P_TimerLed::create()
{
  mdc.setBkg((DWORD)-1);
  int currVal = ManTimer.getValue();
  Info.totNumber = getTotNumber(currVal);
  Info.nDec = 2;
  Info.totImg = 12;
  Info.sign = false;
  Info.background = (DWORD)-1;
  Info.img.setDim(Info.totImg);
  TCHAR path[_MAX_PATH];
  for(uint i = 0; i < Info.totImg; ++i) {
    if(loadPng(path, i))
      Info.img[i] = new PBitmap(path);
    else
      Info.img[i] = new PBitmap(FIRST_BMP + i, getHInstance());
    }
  if(!baseClass::create())
    return false;
  return true;
}
//-------------------------------------------------------------------
void P_TimerLed::evPaint(HDC hdc, const PRect& r)
{
  if(!CW) {
    PWin* next = Next();
    while(next != this) {
      CW = dynamic_cast<clientWin*>(next);
      if(CW)
        break;
      next = next->Next();
      }
    }
  if(CW) {
    if(!Img) {
      PBitmap* img = CW->getImage();
      if(img) {
        PRect r2;
        GetClientRect(*getParent(), r2);
        POINT pt = { r.left, r.top };
        SIZE sz = { r.Width(), r2.Height() };
        img->draw(hdc, pt, sz);
        sz.cy = r.Height();
        Img = new PBitmap(hdc, pt, sz);
        }
      }
    else {
      POINT pt = { r.left, r.top };
      SIZE sz = { r.Width(), r.Height() };
      Img->draw(hdc, pt, sz);
      }
    }
  POINT pt = { -1, -1 };
  SIZE sz = { r.Width(), r.Height() };
  pNumberImg(Info).draw(hdc, ManTimer.getValue(), pt, sz);
}
//-------------------------------------------------------------------
void P_TimerLed::resize()
{
  mdc.clear();
  delete Img;
  Img = 0;
  baseClass::resize();
  extMonitor* w = getParentWin<extMonitor>(this);
  if(!w) {
    w = getMonitor(this);
    PSplitWin* p = getParentWin<PSplitWin>(this);
    if(p)
      w->setPerc(p->getSplitter()->getPerc());
    }
}
//---------------------------------------------------------
LRESULT P_TimerLed::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(!hdc) {
          EndPaint(*this, &ps);
          return 0;
          }
        PRect r;
        GetClientRect(*this, r);
        if(r.Width() && r.Height()) {
          HBRUSH br = (HBRUSH)GetStockObject(BLACK_BRUSH);
          HDC mdcWork = mdc.getMdc(this, hdc);
          FillRect(mdcWork, r, br);
          evPaint(mdcWork, r);
          BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
          }
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    case WM_RBUTTONUP:
      do {
        PSplitWin* w = getParentWin<PSplitWin>(this);
        if(!w)
          PostMessage(*getParent(), message, wParam, lParam);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void P_TimerLed::resetImg()
{
  delete Img;
  Img = 0;
  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
