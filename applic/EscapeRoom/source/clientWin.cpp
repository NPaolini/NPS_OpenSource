//-------------------- clientWin.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "clientWin.h"
#include "pPanel.h"
#include "p_txt.h"
#include <math.h>
//-----------------------------------------------------------
clientWin::~clientWin()
{
//  delete img;
  destroy();
  delete []imgPath;
}
//-----------------------------------------------------------
bool clientWin::create()
{
  if(!baseClass::create())
    return false;
  return true;
}
//-------------------------------------------------------------------
COLORREF makeColor(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  while (p) {
    r = _ttoi(p);
    p = findNextParamTrim(p);
    if (!p)
      break;
    g = _ttoi(p);
    p = findNextParamTrim(p);
    if (!p)
      break;
    b = _ttoi(p);
    break;
  }
  return RGB(r, g, b);
}
//-------------------------------------------------------------------
void getMargin(LPCTSTR p, POINT& pt, SIZE& sz)
{
  int m[4] = {0};
  for(uint i = 0; i < SIZE_A(m); ++i) {
    if(!p)
      break;
    m[i] = _ttoi(p);
    p = findNextParamTrim(p);
    }
  pt.y += m[0];
  pt.x += m[3];
  sz.cx -= m[1] + m[3];
  sz.cy -= m[0] + m[2];
}
//-------------------------------------------------------------------
void clientWin::evPaint(HDC hdc, const PRect& r)
{
  if(!img)
    return;
  PRect r2;
  GetClientRect(*getParent(), r2);
  POINT pt = { r.left, r.Height()- r2.Height()  };
  SIZE sz = { r.Width(), r2.Height() };
  img->draw(hdc, pt, sz);

  pt.x = r.left;
  pt.y = r.top;
  sz.cx = r.Width();
  sz.cy = r.Height();

  TCHAR imgName[_MAX_PATH];
  getKeyPath(IMG_INFO, imgName);
  if(*imgName) {
    makeTrueFullPath(imgName, SIZE_A(imgName));
    PBitmap bmp(imgName);
    if(bmp.isValid()) {
      DWORD perc;
      getKeyParam(IMG_INFO_PERC, &perc);
      if(!perc)
        perc = 66;
      DWORD top;
      getKeyParam(IMG_INFO_TOP, &top);

      r2 = r;
      if(top) {
        r2.bottom = r.Height() * perc / 100.0;
        pt.y = r2.bottom;
        }
      else
        r2.top = r.Height() * (100.0 - perc) / 100.0;
      sz.cy -= r2.Height();
      pInfoImg().draw(hdc, &bmp, r2);
      }
    }
#if 0
  PVect<LPCTSTR> txt;
  txt[0] = _T("Welcome");
  txt[1] = _T("to");
  txt[2] = _T("Escape Room");
#else
  DWORD curr = 0;
  getKeyParam(TEXT_CURR, &curr);
  if(!curr)
    return;
  TCHAR text[4096];
  TCHAR t[64];
  wsprintf(t, TEXT_BASE _T("%d"), curr);
  getKeyPath(t, text);
  if(!*text)
    return;
  LPTSTR txt = translateToCRNL(text, text);
#endif
  infoText it;
  TCHAR fontName[256];
  TCHAR color[256];
  getKeyParam(TEXT_BORDER, (LPDWORD)&it.nBorder);
  getKeyPath(TEXT_FONT, fontName);
  getKeyPath(TEXT_COLOR, color);

  it.fontname = fontName;//_T("arial");
  it.padding = 0;
//  it.background = RGB(96, 96, 96);
  it.background = (DWORD)-1;
  it.color = makeColor(color);
  getKeyPath(TEXT_MARGIN, color);
  getMargin(color, pt, sz);
  pInfoText(it).draw(hdc, txt, pt, sz);
}
//-----------------------------------------------------------
bool clientWin::isChanged()
{
  bool t = changed;
  changed = false;
  return t;
}
//-------------------------------------------------------------------
void clientWin::setBackground(LPCTSTR path)
{
  if(imgPath && !_tcsicmp(path, imgPath))
    return;
  delete []imgPath;
  imgPath = str_newdup(path);
  delete img;
  img = new PBitmap(imgPath);
  InvalidateRect(*this, 0, 0);
  PWin* next = Next();
  P_TimerLed* tml = 0;
  while(next != this) {
    tml = dynamic_cast<P_TimerLed*>(next);
    if(tml)
      break;
    next = next->Next();
    }
  if(tml)
    tml->resetImg();
  changed = true;
}
//-------------------------------------------------------------------
void clientWin::resize()
{
  mdc.clear();
  baseClass::resize();
}
//---------------------------------------------------------
LRESULT clientWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
          bool needDestroy = true;
          HBRUSH br = (HBRUSH)GetStockObject(BLACK_BRUSH);
          HDC mdcWork = mdc.getMdc(this, hdc);
          FillRect(mdcWork, r, br);
          evPaint(mdcWork, r);
          BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
          }
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define WHITE RGB(255, 255, 255)
#define BLACK RGB(0, 0, 0)
//-----------------------------------------------------------
void pInfoText::draw(HDC hdc, const PVect<LPCTSTR>& txt, const POINT& pt, const SIZE& sz)
{
  if(!sz.cy || !sz.cx)
    return;
  uint nElem = txt.getElem();
  if(!nElem)
    return;
  TCHAR t[4096];
  _tcscpy_s(t, txt[0]);
  for(uint i = 1; i < nElem; ++i) {
    _tcscat_s(t, _T("\r\n"));
    _tcscat_s(t, txt[i]);
    }
  draw(hdc, t, pt, sz);
}
//-----------------------------------------------------------
void pInfoText::draw(HDC hdc, LPCTSTR txt, const POINT& pt, const SIZE& sz)
{
  if(!sz.cy || !sz.cx)
    return;
  int heightFont = sz.cy;
  HFONT font = D_FONT(heightFont, 0, 0, Info.fontname);
  PRect r(0, 0, sz.cx, sz.cy);
  r.MoveTo(pt.x, pt.y);
  SetTextAlign(hdc, TA_LEFT | TA_TOP);
  UINT format = DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS;
  HGDIOBJ oldf = SelectObject(hdc, font);
  PRect calc;
  double hf = heightFont;
  do {
    calc = r;
    DrawText(hdc, txt, -1, calc, format | DT_CALCRECT);
    if (calc.Width() <= r.Width() && calc.Height() <= r.Height())
      break;
    double ratioX = (double)calc.Width() / r.Width();
    double ratioY = (double)calc.Height() / r.Height();
    double ratio2 = sqrt(max(ratioX, ratioY));
    hf /= ratio2;
    if(hf < 4)
      hf = 4;
    font = D_FONT(ROUND_REAL(hf), 0, 0, Info.fontname);
    DeleteObject(SelectObject(hdc, font));
  } while (hf > 4);
  bool transp = (DWORD)-1 == Info.background;
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  int dy = r.Height() - calc.Height();
  r.Offset(0, dy / 2);
  if(!transp) {
    PRect r2(r);
    r2.Inflate(Info.padding, Info.padding);
    PPanel(r2, Info.background, PPanel::DN_FILL).draw(hdc);
    }
#if false
  calc.MoveTo(r.left + (r.Width() - calc.Width()) / 2, r.top + (r.Height() - calc.Height()) / 2 -dy / 2);
  PPanel(calc, Info.background, PPanel::BORDER_TRANSP).draw(hdc);
#endif
  COLORREF old;
  if(Info.nBorder > 1) {
    old = SetTextColor(hdc, WHITE);
    PRect r2(r);
    r2.Offset(-2, -2);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(4, 0);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(0, 4);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(-4, 0);
    DrawText(hdc, txt, -1, r2, format);
    }
  if(Info.nBorder > 0) {
    SetTextColor(hdc, BLACK);
    PRect r2(r);
    r2.Offset(-1, -1);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(2, 0);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(0, 2);
    DrawText(hdc, txt, -1, r2, format);
    r2.Offset(-2, 0);
    DrawText(hdc, txt, -1, r2, format);
    }
  SetTextColor(hdc, Info.color);
  DrawText(hdc, txt, -1, r, format);
  DeleteObject(SelectObject(hdc, oldf));
  SetBkMode(hdc, oldMode);
}
//-----------------------------------------------------------
void pInfoImg::draw(HDC hdc, PBitmap* bmp, const PRect& r)
{
  SIZE sz = bmp->getSize();
  POINT pt = { r.left, r.top };
  double ratioX = (double)r.Width() / sz.cx;
  double ratioY = (double)r.Height() / sz.cy;
  if(ratioX > ratioY) {
    sz.cx *= ratioY;
    sz.cy = r.Height();
    pt.x = (r.Width() - sz.cx) / 2;
    }
  else {
    sz.cy *= ratioX;
    sz.cx = r.Width();
    pt.y = (r.Height() - sz.cy) / 2;
    }
  bmp->draw(hdc, pt, sz);
}
