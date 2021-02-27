//------------ svSplashWin.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svSplashWin.h"
#include <stdio.h>
#include "prect.h"
#include "p_txt.h"
#include "p_util.h"
//----------------------------------------------------------------------------
#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
_REG(SPLASH_SCREEN);
//----------------------------------------------------------------------------
#define DEF_INFO_FILE _T("InfoSplash.txt")
#define ID_SKIN        1
#define ID_TRANSP_INFO 2
#define ID_INFO_RECT  10
#define ID_COLOR_INFO 11
#define ID_WAIT_TIME  12
//----------------------------------------------------------------------------
static uint gThreshold;
static uint gUseHSV;
static COLORREF gTransp;
static uint gHSVTransp[3];
//----------------------------------------------------------------------------
//-----------------------------------------------------------
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//    if s == 0, then h = -1 (undefined)
void RGBtoHSV( double r, double g, double b, double *h, double *s, double *v )
{
  double vmin;
  double vmax;
  double delta;
  vmin = min( min( r, g), b );
  vmax = max( max( r, g), b );
  *v = vmax;       // v
  delta = vmax - vmin;
  if( vmax != 0 )
    *s = delta / vmax;   // s
  else {
    // r = g = b = 0    // s = 0, v is undefined
    *s = 0;
    *h = -1;
    return;
    }
  if(!delta)
    *h = 0;
  else {
    if( r == vmax )
      *h = ( g - b ) / delta;   // between yellow & magenta
    else if( g == vmax )
      *h = 2 + ( b - r ) / delta; // between cyan & yellow
    else
      *h = 4 + ( r - g ) / delta; // between magenta & cyan
    *h *= 60;       // degrees
    while( *h < 0 )
      *h += 360;
    }
}
/*
//-----------------------------------------------------------
void HSVtoRGB( double *r, double *g, double *b, double h, double s, double v )
{
  int i;
  double f, p, q, t;
  if( s == 0 ) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }
  h /= 60;      // sector 0 to 5
  i = (int)floor( h );
  f = h - i;      // factorial part of h
  p = v * ( 1 - s );
  q = v * ( 1 - s * f );
  t = v * ( 1 - s * ( 1 - f ) );
 switch( i ) {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:    // case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
  }
}
*/
//-----------------------------------------------------------
static bool isEquBMP(COLORREF test)
{
  return gTransp == test;
}
//-----------------------------------------------------------
static bool isEquThreshold(COLORREF test)
{
  uint t = abs(GetRValue(test) - GetRValue(gTransp));
  if(t >= gThreshold)
    return false;
  t += abs(GetGValue(test) - GetGValue(gTransp));
  if(t >= gThreshold)
    return false;
  t += abs(GetBValue(test) - GetBValue(gTransp));
  if(t >= gThreshold)
    return false;
  return true;
}
//-----------------------------------------------------------
static bool isEquHSV(COLORREF test)
{
  double h;
  double s;
  double v;
  RGBtoHSV(GetRValue(test) / 255.0, GetGValue(test) / 255.0, GetBValue(test) / 255.0, &h, &s, &v);
  h *= 255.0/360.0;

  uint t = abs((int)(h - gHSVTransp[0]));
  if(t >= gThreshold)
    return false;
  s *= 255;
  t += abs((int)(s - gHSVTransp[1]));
  if(t >= gThreshold)
    return false;
  v *= 255;
  t += abs((int)(v - gHSVTransp[2]));
  if(t >= gThreshold)
    return false;
  return true;
}
//-----------------------------------------------------------
svSplashWin::svSplashWin(LPCTSTR title, HINSTANCE hinst)
:
  baseClass(title, hinst), idTimer(0), Bkg(0), Time(0), idTimerEffect(0),
   m_myWater(0), m_myFire(0), Bits1(0), Bits2(0)
{
  setOfString set(DEF_INFO_FILE);
  LPCTSTR p = set.getString(ID_SKIN);
//  MessageBox(0, _T("wait"), _T("test"), MB_OK); // for debug
  if(p) {
    Bkg = new PBitmap(p);
    if(!Bkg->isValid()) {
      delete Bkg;
      Bkg = 0;
      }
    }
  int x1 = 0;
  int y1 = 0;
  int x2 = 100;
  int y2 = 100;
  p = set.getString(ID_INFO_RECT);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &x1, &y1, &x2, &y2);
  infoRect = PRect(x1, y1, x2, y2);

  int r = 0;
  int g = 0;
  int b = 0;
  p = set.getString(ID_COLOR_INFO);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  iColor = RGB(r, g, b);

  Time = 30;
  p = set.getString(ID_WAIT_TIME);
  if(p)
    Time = _ttoi(p);
  p = set.getString(ID_TRANSP_INFO);
  if(p)
    _stscanf_s(p, _T("%d,%d"), &gThreshold, &gUseHSV);
}
//----------------------------------------------------------------------------
svSplashWin::~svSplashWin()
{
  delete Bkg;
  Bkg = 0;
  destroy();
  delete m_myWater;
  delete m_myFire;
  delete []Bits1;
  delete []Bits2;
}
//----------------------------------------------------------------------------
extern HWND getHwSvisor();
//----------------------------------------------------------------------------
void svSplashWin::Destroy(HWND hwnd)
{
  if(idTimer) {
    KillTimer(hwnd, idTimer);
    idTimer = 0;
    }
  if(idTimerEffect) {
    KillTimer(hwnd, idTimerEffect);
    idTimerEffect = 0;
    }
  SetForegroundWindow(getHwSvisor());
}
//----------------------------------------------------------------------------
#define USE_ANIM
//----------------------------------------------------------------------------
static void closeWithAnim(HWND hwnd)
{
#ifdef USE_ANIM
  if(isWinXP_orLater()) {
    uint playTime = 500;
    AnimateWindow(hwnd, playTime, AW_HOR_NEGATIVE | AW_VER_POSITIVE | AW_SLIDE | AW_CENTER | AW_HIDE);
    }
#endif
}
//----------------------------------------------------------------------------
//#define SHOW_ON_BAR
//----------------------------------------------------------------------------
bool svSplashWin::create()
{
  Attr.style = WS_POPUP;
#ifdef USE_ANIM
  if(!isWinXP_orLater())
#endif
    Attr.style |= WS_VISIBLE;
  Attr.exStyle = WS_EX_TOOLWINDOW | // rende la barra più piccola
                  WS_EX_TOPMOST |
#ifdef SHOW_ON_BAR
                 // il flag precedente non fa apparire l'icona sulla
                 // barra dei task, va forzata con questo
                  WS_EX_APPWINDOW;
#else
                  0;
#endif
  if(!baseClass::create())
    return false;
  bool useComplexRegion = resize();
  PostMessage(getHwSvisor(), WM_SPLASH_SCREEN, MAKEWPARAM(WMC_SPLASH_HWND, 0), reinterpret_cast<LPARAM>(getHandle()));

#ifdef USE_ANIM
  if(isWinXP_orLater()) {
    uint playTime = 500;
    AnimateWindow(*this, playTime, AW_CENTER | AW_SLIDE);
    }
#endif
  PRect r;
  GetWindowRect(*this, r);
  uint size = (r.Width() * 4) * r.Height();
  Bits1 = new BYTE[size];
  Bits2 = new BYTE[size];

  m_myWater = new CWaterRoutine;
  m_myWater->Create(r.Width(), r.Height());
  m_myWater->m_density = 3;

  m_myFire = new CFireRoutine;
  m_myFire->m_iAlpha = 60;// want a 60% alpha
  m_myFire->m_iHeight = r.Height();
  m_myFire->m_iWidth = r.Width();
  m_myFire->InitFire();

  idTimerEffect = SetTimer(*this, 255, 50, 0);
  SetTimer(*this, idTimer = 1, 60 * 1000, 0);
  return true;
}
//----------------------------------------------------------------------------
#define ROUND_BORDER 3
typedef bool (*cmp_pixel)(COLORREF);
//----------------------------------------------------------------------------
HRGN createRegion(HDC hdc, const SIZE& sz)
{
  gTransp = GetPixel(hdc, 0, 0);
  cmp_pixel cmpPixel;
  if(gThreshold) {
    if(gThreshold > 300)
      gThreshold = 300;
    if(gUseHSV) {
      cmpPixel = isEquHSV;
      double h;
      double s;
      double v;
      RGBtoHSV(GetRValue(gTransp) / 255.0, GetGValue(gTransp) / 255.0, GetBValue(gTransp) / 255.0, &h, &s, &v);
      gHSVTransp[0] = (int)(h * 255.0/360.0);
      gHSVTransp[1] = (int)(s * 255.0);
      gHSVTransp[2] = (int)(v * 255.0);

      }
    else
      cmpPixel = isEquThreshold;
    }
  else
    cmpPixel = isEquBMP;

  HRGN hrgnBitmap = CreateRectRgn(0, 0, sz.cx, sz.cy);

  BOOL bInTransparency = FALSE;  // Already inside a transparent part?
  int start_x = -1;        // Start of transparent part

  // For all rows of the bitmap ...
  for (int y = 0; y < sz.cy; y++)
  {
    // For all pixels of the current row ...
    // (To close any transparent region, we go one pixel beyond the
    // right boundary)
    for (int x = 0; x <= sz.cx; x++)
    {
      BOOL bTransparent = FALSE; // Current pixel transparent?

      // Check for positive transparency within image boundaries
//      if ((x < sz.cx) && (colorTransp == GetPixel(hdc, x, y)))
      if ((x < sz.cx) && cmpPixel(GetPixel(hdc, x, y)))
      {
        bTransparent = TRUE;
      }

      // Does transparency change?
      if (bInTransparency != bTransparent)
      {
        if (bTransparent)
        {
          // Transparency starts. Remember x position.
          bInTransparency = TRUE;
          start_x = x;
        }
        else
        {
          // Transparency ends (at least beyond image boundaries).
          // Create a region for the transparency, one pixel high,
          // beginning at start_x and ending at the current x, and
          // subtract that region from the current bitmap region.
          // The remainding region becomes the current bitmap region.
          HRGN hrgnTransp = ::CreateRectRgn(start_x, y, x, y + 1);
          ::CombineRgn(hrgnBitmap, hrgnBitmap, hrgnTransp, RGN_DIFF);
          ::DeleteObject(hrgnTransp);

          bInTransparency = FALSE;
        }
      }
    }
  }

  return hrgnBitmap;
}
//----------------------------------------------------------------------------
bool isBmpType()
{
  setOfString set(DEF_INFO_FILE);
  LPCTSTR p = set.getString(ID_SKIN);
  int len = _tcslen(p);
  return !_tcsicmp(p + len - 4, _T(".bmp"));
}
//----------------------------------------------------------------------------
bool svSplashWin::resize()
{
  if(!Bkg)
    return false;
  SIZE sz = Bkg->getSize();
  bool useComplexRegion = false;
  if((int)gThreshold >= 0) {
    HRGN hRgn;

    if(gThreshold || isBmpType()) {
      HDC mdc = CreateCompatibleDC(0);

      HGDIOBJ oldObj = SelectObject(mdc, *Bkg);

      hRgn = createRegion(mdc, sz);
      SelectObject(mdc, oldObj);
      DeleteDC(mdc);
      useComplexRegion = true;
      }
    else {
      int rX = sz.cx / 8;
      int rY = sz.cy / 8;
      hRgn = CreateRoundRectRgn(0, 0, sz.cx, sz.cy, rX, rY);//ROUND_BORDER, ROUND_BORDER);
      }

    if(hRgn)
      SetWindowRgn(*this, hRgn, true);
    }
  PRect r(0, 0, sz.cx, sz.cy);
  PRect rSV;
  GetWindowRect(getHwSvisor(), rSV);
  if(rSV.Width()) {
    if(rSV.left > 300 || rSV.top > 200)
      r.Offset(rSV.left + 10, rSV.top + 10);
    else {
      int w = rSV.Width();
      int h = rSV.Height();
      r.Offset((w - sz.cx) / 2 + rSV.left, (h - sz.cy) / 8 + rSV.top);
      }
    }
  else {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    r.Offset((w - sz.cx) / 2, (h - sz.cy) / 8);
    }
  uint flag = SWP_NOCOPYBITS;
  setWindowPos(HWND_TOPMOST, r, flag);

  InvalidateRect(*this, 0, 0);
  return useComplexRegion;
}
//----------------------------------------------------------------------------
inline
bool isEmptyRect(const PRect& r)
{
  return !(r.Width() || r.Height());
}
//----------------------------------------------------------------------------
void svSplashWin::evPaint(HDC hdc)
{
  if(!Bkg)
    return;
  POINT pt = { 0, 0 };
  Bkg->draw(hdc, pt);
  paintEvent(hdc);
}
//----------------------------------------------------------------------------
int svSplashWin::calcLenBuff()
{
  int len = 0;
  int nElem = Info.getElem();
  const int minAdd = 24;
  for(int i = 0; i < nElem; ++i)
    len += minAdd + _tcslen(Info[i].name);
  return len;
}
//----------------------------------------------------------------------------
void svSplashWin::performPaint(HDC hdc)
{
#define USE_MEMDC
#ifdef USE_MEMDC

#define DEF_COLOR_BKG RGB(192, 192, 192)
  PRect r;
  GetClientRect(*this, r);
  HBITMAP hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());

  HDC mdc = CreateCompatibleDC(hdc);
  HGDIOBJ oldObj = SelectObject(mdc, hBmpTmp);

  HBRUSH br = CreateSolidBrush(DEF_COLOR_BKG);
  FillRect(mdc, r, br);
  DeleteObject(HGDIOBJ(br));

  evPaint(mdc);
  if(m_myFire && m_myWater) {
    BITMAPINFOHEADER bmih;

    memset(&bmih, 0, sizeof(bmih));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = r.Width();
    bmih.biHeight = r.Height();
    bmih.biPlanes = 1;
    bmih.biBitCount =  32;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = (bmih.biWidth * 4) * bmih.biHeight;

    bool success = toBool(GetDIBits(mdc, hBmpTmp, 0, bmih.biHeight, Bits1, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS));
    m_myWater->Render((DWORD*)Bits1,(DWORD*)Bits2);
    m_myFire->Render((DWORD*)Bits2, r.Width(), r.Height());

    SetDIBits(mdc, hBmpTmp, 0, bmih.biHeight, Bits2, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS);
    }
  BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdc, 0, 0, SRCCOPY);
  SelectObject(mdc, oldObj);

  DeleteDC(mdc);

  DeleteObject(hBmpTmp);

#else
  evPaint(hdc);
#endif
}
//----------------------------------------------------------------------------
void svSplashWin::paintEvent(HDC hdc)
{
  int len = calcLenBuff();
  HGDIOBJ font = GetStockObject(SYSTEM_FONT);

  HGDIOBJ oldFont = SelectObject(hdc, font);
  COLORREF oldColor = SetTextColor(hdc, iColor);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  UINT flag =  DT_LEFT | DT_NOCLIP | DT_PATH_ELLIPSIS | DT_TOP | DT_WORD_ELLIPSIS;
  LPTSTR buff = 0;
  if(len) {
    buff = new TCHAR[len + 1];
    int nElem = Info.getElem();
    LPTSTR p = buff;
    for(int i = 0; i < nElem; ++i) {
      if(Info[i].loaded > 0)
        wsprintf(p, _T("Loaded %s [%d]\r\n"), Info[i].name, Info[i].id);
      else if(Info[i].loaded < 0)
        wsprintf(p, _T("Failed %s [%d]\r\n"), Info[i].name, Info[i].id);
      else
        wsprintf(p, _T("Loading %s [%d] ...\r\n"), Info[i].name, Info[i].id);
      p = p + _tcslen(p);
      }
    }
  else {
    buff = str_newdup(_T("Waiting ..."));
    flag =  DT_CENTER | DT_NOCLIP | DT_PATH_ELLIPSIS | DT_TOP | DT_WORD_ELLIPSIS;
    }
  DrawText(hdc, buff, -1, infoRect, flag);

  SelectObject(hdc, oldFont);
  SetTextColor(hdc, oldColor);
  SetBkMode(hdc, oldMode);

  delete []buff;
}
//----------------------------------------------------------------------------
void svSplashWin::renderEffect()
{
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
#define EFFECT HeightBlob
void svSplashWin::mouseEffect(int which, const POINTS& pt)
{
  POINT p = { pt.x, pt.y };
  MapWindowPoints(HWND_DESKTOP, *this, &p, 1);
  PRect r;
  GetWindowRect(*this, r);
  p.y = r.Height() - p.y;
  switch(which) {
    case 0:
      m_myWater->EFFECT(p.x, p.y, 5, 50, m_myWater->m_iHpage);
      break;
    case 1:
      m_myWater->EFFECT(p.x, p.y, 40, 300, m_myWater->m_iHpage);
      break;
    }
}
//----------------------------------------------------------------------------
LRESULT svSplashWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_SPLASH_SCREEN == message) {
    switch(LOWORD(wParam)) {
      case WMC_START_TIMER:
        SetTimer(*this, idTimer = 1, Time * 1000, 0);
        break;
      case WMC_END_INIT:
        if(idTimer) {
          KillTimer(*this, idTimer);
          idTimer = 0;
          }
        break;
      case WMC_CLOSE:
        closeWithAnim(hwnd);
        PostQuitMessage(0);
        return true; //result;
      default:
        return processSVisorMsg(hwnd, message, wParam, lParam);
      }
    }
  else {
    switch(message) {

      case WM_TIMER:
        if(idTimerEffect == wParam)
          renderEffect();
        else if(idTimer == wParam)  {
          KillTimer(*this, idTimer);
          PostMessage(getHwSvisor(), WM_SPLASH_SCREEN, MAKEWPARAM(WMC_TIMEOUT_SPLASH, 0), 0);
          closeWithAnim(hwnd);
          PostQuitMessage(0);
          return true;
          }
        break;

      case WM_MOUSEMOVE:
      case WM_NCMOUSEMOVE:
        mouseEffect(0, MAKEPOINTS(lParam));
        break;
      case WM_LBUTTONDOWN:
      case WM_NCLBUTTONDOWN:
        mouseEffect(1, MAKEPOINTS(lParam));
        break;
      case WM_NCHITTEST:
        return HTCAPTION;
      case WM_PRINT:
      case WM_PRINTCLIENT:
        performPaint((HDC)wParam);
        break;

      case WM_ERASEBKGND:
        return 1;
      case WM_PAINT:
        do {
          PAINTSTRUCT ps;
          HDC hdc = BeginPaint(*this, &ps);
          if(!hdc) {
            EndPaint(*this, &ps);
            break;
            }
          PRect r(ps.rcPaint);
          if(isEmptyRect(r)) {
            EndPaint(*this, &ps);
            break;
            }
          performPaint(hdc);
          EndPaint(*this, &ps);
          } while(false);
        return 0;

      case WM_COPYDATA:
        return processSVisorMsg(hwnd, message, wParam, lParam);

      case WM_DESTROY:
        Destroy(hwnd);
        PostQuitMessage(0);
        break;
      }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svSplashWin::addInfo(uint idCode, LPCTSTR p)
{
  int id;
  int loaded = 0;
  if(WMC_LOADING_PRF_0 <= idCode && idCode < WMC_LOADING_PRF_0 + MAX_PERIPH_INFO_SPLASH)
    id = idCode - WMC_LOADING_PRF_0;
  else if(WMC_LOADED_PRF_0 <= idCode && idCode < WMC_LOADED_PRF_0 + MAX_PERIPH_INFO_SPLASH) {
    id = idCode - WMC_LOADED_PRF_0;
    loaded = 1;
    }
  else if(WMC_FAILED_PRF_0 <= idCode && idCode < WMC_FAILED_PRF_0 + MAX_PERIPH_INFO_SPLASH) {
    id = idCode - WMC_FAILED_PRF_0;
    loaded = -1;
    }

  int nElem = Info.getElem();
  bool found = false;
  for(int i = 0; i < nElem; ++i) {
    if(id == Info[i].id) {
      Info[i].loaded = loaded;
      return;
      }
    }
  info in;
  in.id = id;
  _tcscpy_s(in.name, p);
  Info[nElem] = in;
}
//--------------------------------------------------------
#define WMC_MSG_IS_UNICODE 0x80000000
//--------------------------------------------------------
LRESULT svSplashWin::processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_COPYDATA == message) {
    COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;

    uint id = cds->dwData;

    bool is_unicode = toBool(id & WMC_MSG_IS_UNICODE);
    id &= ~WMC_MSG_IS_UNICODE;
  LPTSTR p = 0;
#ifdef UNICODE
    if(!is_unicode) {
      uint len = strlen((LPCSTR)cds->lpData);
      p = new wchar_t[len + 1];
      copyStrZ(p, (LPCSTR)cds->lpData, len);
      }
#else
  if (is_unicode) {
    uint len = wcslen((LPCWSTR)cds->lpData);
    p = new char[len + 1];
    copyStrZ(p, (LPCWSTR)cds->lpData, len);
  }
#endif
  else
    p = str_newdup((LPCTSTR)cds->lpData);
    ReplyMessage(TRUE);

    addInfo(id, p);
    delete []p;
    InvalidateRect(*this, 0, 0);
    return 1;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
