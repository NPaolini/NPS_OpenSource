//-------------------- pOwnBtnImageStd.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pdialog.h"
#include "pOwnBtnImageStd.h"
#include "PTraspBitmap.h"
//-----------------------------------------------------------
void gThemeChanged()
{
  manageXP_Theme::themeChanged();
}
//-----------------------------------------------------------
manageXP_Theme manageXP_Theme::dummy;
HMODULE manageXP_Theme::hModThemes;
bool manageXP_Theme::success;
//-----------------------------------------------------------
#define STAT_PROC_DEF(a) ptr_##a manageXP_Theme::a
//-----------------------------------------------------------
STAT_PROC_DEF(OpenThemeData);
STAT_PROC_DEF(DrawThemeBackground);
STAT_PROC_DEF(CloseThemeData);
STAT_PROC_DEF(DrawThemeText);
STAT_PROC_DEF(GetThemeBackgroundContentRect);

STAT_PROC_DEF(IsThemeBackgroundPartiallyTransparent);
STAT_PROC_DEF(DrawThemeParentBackground);
STAT_PROC_DEF(DrawThemeEdge);
STAT_PROC_DEF(GetThemeColor);
STAT_PROC_DEF(GetThemeSysColor);
//------------------------------------------------------------------
template <typename T>
bool getProcAddress(HMODULE hdll, LPCSTR procName, T& proc)
{
  FARPROC f = GetProcAddress(hdll, procName + 1);
  if(!f)
    f = GetProcAddress(hdll, procName);
  if(!f)
    return false;
  proc = (T)f;
  return true;
}
//------------------------------------------------------------------
#define get_proc(hDLL, a) getProcAddress<ptr_##a>(hDLL, "_" #a, a)
//------------------------------------------------------------------
#define GET_PROC(a) if(!get_proc(hModThemes, a)) break
//------------------------------------------------------------------
void manageXP_Theme::themeChanged()
{
  closeAll();
  hModThemes = LoadLibrary(_T("UXTHEME.DLL"));
  while(hModThemes) {
    GET_PROC(OpenThemeData);
    GET_PROC(DrawThemeBackground);
    GET_PROC(CloseThemeData);
    GET_PROC(DrawThemeText);
    GET_PROC(GetThemeBackgroundContentRect);

    GET_PROC(IsThemeBackgroundPartiallyTransparent);
    GET_PROC(DrawThemeParentBackground);
    GET_PROC(DrawThemeEdge);
    GET_PROC(GetThemeColor);
    GET_PROC(GetThemeSysColor);

    success = true;
    return;
    }
  FreeLibrary(hModThemes);
  hModThemes = 0;
}
//------------------------------------------------------------------
#define ZERO_PROC(a) a = 0
//------------------------------------------------------------------
void manageXP_Theme::closeAll()
{
  if(hModThemes) {
    FreeLibrary(hModThemes);
    hModThemes = 0;
    }
  ZERO_PROC(OpenThemeData);
  ZERO_PROC(DrawThemeBackground);
  ZERO_PROC(CloseThemeData);
  ZERO_PROC(DrawThemeText);
  ZERO_PROC(GetThemeBackgroundContentRect);

  ZERO_PROC(IsThemeBackgroundPartiallyTransparent);
  ZERO_PROC(DrawThemeParentBackground);
  ZERO_PROC(DrawThemeEdge);
  ZERO_PROC(GetThemeColor);
  ZERO_PROC(GetThemeSysColor);

  success = false;
}
//------------------------------------------------------------------
extern
void trasformBmp(HDC hdc, PBitmap* bmp, float mH, float mS, float mV, bool grayed);
extern
double scaleImage(const SIZE& winSz, SIZE& bmpSz);
//-----------------------------------------------------------
POwnBtnImageStd::~POwnBtnImageStd()
{
  if(isThemed())
    manageXP_Theme::CloseThemeData(hTheme);

  destroy();
  if(autoDeleteBmp)
    delete Bmp;
}
//-----------------------------------------------------------
bool POwnBtnImageStd::create()
{
  if(!baseClass::create())
    return false;
//  PDialog* d = dynamic_cast<PDialog*>(getParent());
  loadTheme();
  return true;
}
//-----------------------------------------------------------
PBitmap* POwnBtnImageStd::replaceBmp(PBitmap* other, uint pos)
{
  PBitmap* b = Bmp;
  Bmp = other;
  return b;
}
//-----------------------------------------------------------
void POwnBtnImageStd::loadTheme()
{
  themeChanged();
  if(isThemed())
    enableCapture();
}
//-----------------------------------------------------------
void POwnBtnImageStd::themeChanged()
{
  if(isThemed())
    manageXP_Theme::CloseThemeData(hTheme);
  hTheme = 0;
  if(enabledTheme() && manageXP_Theme::isValid())
    hTheme = manageXP_Theme::OpenThemeData(*this, L"Button");

}
//-----------------------------------------------------------
void POwnBtnImageStd::invalidateIf()
{
  if(!isThemed())
    baseClass::invalidateIf();
}
//-----------------------------------------------------------
void POwnBtnImageStd::mouseEnter(const POINT& pt, uint flag)
{
  if(isThemed()) {
    mouseOverButton = true;
    InvalidateRect(*this, 0, 0);
    }
  baseClass::mouseEnter(pt, flag);
}
//-----------------------------------------------------------
void POwnBtnImageStd::mouseLeave(const POINT& pt, uint flag)
{
  if(isThemed()) {
    mouseOverButton = false;
    InvalidateRect(*this, 0, 0);
    }
  baseClass::mouseLeave(pt, flag);
}
//-----------------------------------------------------------
inline bool isBitsEqu(DWORD check, DWORD bits)
{
  return (check & bits) == check;
}
//-----------------------------------------------------------
bool POwnBtnImageStd::drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis)
{
  if(dis->itemID == -1)
    return baseClass::drawItem(idCtrl, dis);

  HDC hdc = dis->hDC;
  if(!hdc)
    return baseClass::drawItem(idCtrl, dis);

  if(!isThemed())
    return baseClass::drawItem(idCtrl, dis);

  DWORD state;
  howDraw style = released;

  if(ODS_DISABLED & dis->itemState) {
    state = PBS_DISABLED;
    style = disabled;
    }
  else if(ODS_SELECTED & dis->itemState) {
    state = PBS_PRESSED;
    style = pushed;
    }
  else {
    if(mouseOverButton)
      state = PBS_HOT;
    else if(ODS_FOCUS & dis->itemState)
      state = PBS_DEFAULTED;
    else
      state = PBS_NORMAL;
    }

  PRect r(dis->rcItem);
  PRect r2(r);
  ++r2.bottom;
  ++r2.right;
  if(Flat) {
    if(!Over && !(ODS_FOCUS & dis->itemState))
      manageXP_Theme::DrawThemeParentBackground(*this, hdc, r2);
    else {
      if(manageXP_Theme::IsThemeBackgroundPartiallyTransparent(hTheme, BP_PUSHBUTTON, state))
        manageXP_Theme::DrawThemeParentBackground(*this, hdc, r2);
      manageXP_Theme::DrawThemeBackground(hTheme, hdc, BP_PUSHBUTTON, state, r2, NULL);
      }
    }
  else {
    if(manageXP_Theme::IsThemeBackgroundPartiallyTransparent(hTheme, BP_PUSHBUTTON, state))
      manageXP_Theme::DrawThemeParentBackground(*this, hdc, r2);
    manageXP_Theme::DrawThemeBackground(hTheme, hdc, BP_PUSHBUTTON, state, r2, NULL);
    }
  if(isBitsEqu(ODS_SELECTED, dis->itemState)) {
    COLORREF c;
    if(mouseOverButton) {
      manageXP_Theme::GetThemeColor(hTheme, BP_PUSHBUTTON, PBS_HOT, TMT_ACCENTCOLORHINT, &c);
      drawFocusThemed(hdc, c, r2);
      }
    else if(isBitsEqu(ODS_FOCUS, dis->itemState)) {
      manageXP_Theme::GetThemeColor(hTheme, BP_PUSHBUTTON, PBS_DEFAULTED, TMT_BORDERCOLORHINT, &c);
      drawFocusThemed(hdc, c, r2);
      }
    }
  r.Inflate(-2, -2);
  drawCustom(hdc, r, style);

  drawText(hdc, r, style);


  return true;
}
//-----------------------------------------------------------
void POwnBtnImageStd::drawFocusThemed(HDC hdc, COLORREF c, const PRect& rect)
{
  PRect r(rect);// = getRectDrawButton(false);
  r.Inflate(-3, -2);
  --r.left;
  --r.bottom;
    POINT pts[] = {
      { r.left, r.top },
      { r.right, r.top },
      { r.right, r.bottom },
      { r.left, r.bottom },
      { r.left, r.top }
      };

  HPEN pen = CreatePen(PS_SOLID, 1, c);
  HGDIOBJ old = SelectObject(hdc, pen);
  Polyline(hdc, pts, SIZE_A(pts));
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
void POwnBtnImageStd::adjustRectText(const PRect& rect, PRect& calc)
{
  int diffW = rect.Width() - calc.Width();
  int diffH = rect.Height() - calc.Height();
#if 1
  if(diffH || diffW)
    calc.Offset(diffW / 2, diffH / 2);
#else
  if(diffH || diffW) {
    switch(flagPos) {
      case POwnBtnImageStd::wLeft:
        calc.Offset(diffW, diffH / 2);
        break;
      case POwnBtnImageStd::wTop:
        calc.Offset(diffW / 2, diffH / 2);
        break;
      case POwnBtnImageStd::wRight:
        calc.Offset(0, diffH / 2);
        break;
      case POwnBtnImageStd::wBottom:
        calc.Offset(diffW / 2, diffH / 2);
        break;
      }
    }
#endif
}
//-----------------------------------------------------------
void POwnBtnImageStd::drawText(HDC hdc, const PRect& rect, POwnBtn::howDraw style)
{
  if(getCaption() && Bmp && wCenter != flagPos) {
    PRect r(rect);
    r.Inflate(-2, -2);
#if 0
    SIZE szF = getSizeFont();
    SIZE szB = Bmp->getSize();

    switch(flagPos) {
      case POwnBtnImageStd::wLeft:
        r.left += szB.cx;
        break;
      case POwnBtnImageStd::wTop:
        r.top = r.bottom - szF.cy / SIMUL_DEC_FONT;
        break;
      case POwnBtnImageStd::wRight:
        r.right -= szB.cx;
        break;
      case POwnBtnImageStd::wBottom:
        r.bottom = r.top + szF.cy / SIMUL_DEC_FONT;
        break;
      }
#else
    SIZE sz = Bmp->getSize();
    if(scaleImg) {
      PRect r = getRectDrawButton(true);
      makeRectImageButton(r);
      SIZE winSz = { r.Width(), r.Height() };
      scaleImage(winSz, sz);
      }
    switch(flagPos) {
      case wLeft:
        r.left += sz.cx;
        break;
      case wTop:
        r.top += sz.cy;
        break;
      case wRight:
        r.right -= sz.cx;
        break;
      case wBottom:
        r.bottom -= sz.cy;
        break;
      }
#endif
    baseClass::drawText(hdc, r, style);
    }
  else
    baseClass::drawText(hdc, rect, style);
}
//-----------------------------------------------------------
void POwnBtnImageStd::drawCustom(HDC hdc, const PRect& rect, POwnBtn::howDraw style)
{
  if(Bmp) {
    SIZE sz = Bmp->getSize();

    double scale = 1.0;
    switch(scaleImg) {
      case POwnBtnImageStd::eisStretched:
        break;
      case POwnBtnImageStd::eisScaled:
        PRect r = getRectDrawButton(true);
        makeRectImageButton(r);
        SIZE winSz = { r.Width(), r.Height() };
        scale = scaleImage(winSz, sz);
        break;
      }

    POINT pt = { 0, 0 };
    if((!getCaption() || !*getCaption()) && eisStretched != scaleImg) {
      pt.x = (rect.left + rect.right - sz.cx) / 2;
      pt.y = (rect.top + rect.bottom - sz.cy) / 2;
      }
    else {
      PRect r(rect);
      r.Inflate(-2, -2);
      switch(flagPos) {
        case wLeft:
          pt.x = r.left + 1;
          pt.y = (r.top + r.bottom - sz.cy) / 2;
          break;
        case wTop:
          pt.x = (r.left + r.right - sz.cx) / 2;
          pt.y = r.top + 1;
          break;
        case wRight:
          pt.x = r.right - sz.cx - 1;
          pt.y = (r.top + r.bottom - sz.cy) / 2;
          break;
        case wBottom:
          pt.x = (r.left + r.right - sz.cx) / 2;
          pt.y = r.bottom - sz.cy - 1;
          break;
        case wCenter:
          if(eisStretched == scaleImg) {
            pt.x = r.left;
            pt.y = r.top;
            }
          else {
            pt.x = (rect.left + rect.right - sz.cx) / 2;
            pt.y = (rect.top + rect.bottom - sz.cy) / 2;
            }
          break;
        }
      }
    PBitmap* bmp = Bmp;

    if(style == pushed) {
      ++pt.x;
      ++pt.y;
      }
    else if(POwnBtn::disabled == style) {
      // disegnare l'immagine in grayed
      bmp = new PBitmap(*Bmp);
      const float mH = 1.2f;
      const float mS = 0.2f;
      const float mV = 3;
      trasformBmp(hdc, bmp, mH, mS, mV, true);
      }

    PTraspBitmap trsp(this, bmp, pt);
    if(POwnBtnImageStd::eisStretched == scaleImg) {
      PRect r(rect);
      r.Inflate(-2, -2);
      double scaleX = r.Width();
      scaleX /= (double)sz.cx;
      double scaleY = r.Height();
      scaleY /= (double)sz.cy;
      trsp.setScale(scaleX, scaleY);
      }
    else if(scale != 1.0)
      trsp.setScale(scale);
    trsp.Draw(hdc);
    if(POwnBtn::disabled == style)
      delete bmp;
    }
}
//-----------------------------------------------------------
void POwnBtnImageStd::makeRectImageButton(PRect& r)
{
  r = getRectDrawButton(true);
  if(!getCaption() || !*getCaption() || wCenter == flagPos)
    return;
  switch(flagPos) {
    case wLeft:
    case wRight:
      r.right /= 2;
//      r.right = r.left + r.Width() / 2;
      break;
    case wTop:
    case wBottom:
      r.bottom /= 2;
//      r.bottom = r.top + r.Height() / 2;
      break;
    }
}
//-----------------------------------------------------------
PRect POwnBtnImageStd::getRectDrawButton(bool withFocus) const
{
  PRect r;
  GetWindowRect(*this, r);
  r.MoveTo(0, 0);
  return r;
}
//-----------------------------------------------------------
void POwnBtnImageStd::drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT /*dis*/)
{
  PRect r = getRectDrawButton(false);
  r.Inflate(-3, -3);
  DrawFocusRect(hdc, r);
}
//-----------------------------------------------------------
LRESULT POwnBtnImageStd::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NCPAINT:
      if(isThemed())
        break;
      return 0;

    case WM_ERASEBKGND:
      if(isThemed())
        break;
      return 1;
    case WM_KILLFOCUS:
      if(isThemed()) {
        mouseOverButton = false;
        if(!Flat)
          InvalidateRect(*this, 0, 0);
        }
      break;
    case WM_DESTROY:
      if(isThemed())
        manageXP_Theme::CloseThemeData(hTheme);
      hTheme = 0;
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------

