//-------------------- pOwnBtn.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <math.h>
#include "pOwnBtn.h"
#include "pEdit.h"
#include "pTextPanel.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
#define EDGE_BTN_X (rectColor.tickness)
#define EDGE_BTN_Y EDGE_BTN_X
//-----------------------------------------------------------
#define DIM_BORDER GetSystemMetrics(SM_CXEDGE)
//-----------------------------------------------------------
void POwnBtn::invalidateIf()
{
//  PRect r;
//  GetUpdateRect(*getParent(), r, false);
  PRect r2 = getRect();
  // sono controlli inutili, costano di più dell'invalidare direttamente
//  if(r.Intersec(r2))
    InvalidateRect(*getParent(), r2, 0);
}
//-----------------------------------------------------------
int POwnBtn::verifyKey()
{
/**/
  if(GetKeyState(VK_RETURN)& 0x8000)
    return VK_RETURN;
  if(GetKeyState(VK_DOWN)& 0x8000)
    return VK_DOWN;
  if(GetKeyState(VK_UP)& 0x8000)
    return VK_UP;
/**/
  return baseClass::verifyKey();
}
//-----------------------------------------------------------
bool POwnBtn::evKeyDown(WPARAM& key)
{
  if(VK_DOWN == key) {
    up_click(VK_DOWN);
    down_click(VK_TAB);
    return true;
    }
  if(VK_UP == key) {
    up_click(VK_UP);
    down_click(VK_SHIFT);
    down_click(VK_TAB);
    return true;
    }
  if(VK_RETURN == key) {
    down_click(VK_SPACE);
    invalidateIf();
    return true;
    }
  if(VK_SPACE == key)
    invalidateIf();
  return false;
}
//-----------------------------------------------------------
bool POwnBtn::evKeyUp(WPARAM& key)
{
  if(VK_DOWN == key) {
    up_click(VK_TAB);
    return true;
    }
  if(VK_UP == key) {
    up_click(VK_TAB);
    up_click(VK_SHIFT);
    return true;
    }
  if(VK_RETURN == key) {
    up_click(VK_SPACE);
    invalidateIf();
    return true;
    }
  if(VK_SPACE == key)
    invalidateIf();
  return false;
}
//-----------------------------------------------------------
bool POwnBtn::evChar(WPARAM& key)
{
  if(VK_RETURN == key) {
//    click(VK_SPACE);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
//----------------------------------------------------------------------------
bool POwnBtn::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message) {
    if(VK_RETURN == msg.wParam)
      msg.wParam = VK_SPACE;
    }
  else if(WM_KEYUP == msg.message) {
    if(VK_RETURN == msg.wParam)
      msg.wParam = VK_SPACE;
    }

  return baseClass::preProcessMsg(msg);
}
//-----------------------------------------------------------
LRESULT POwnBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      invalidateIf();
      break;
    case WM_KILLFOCUS:
      if(Flat)
        InvalidateParent();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
bool POwnBtn::drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis)
{
  if(dis->itemID == -1)
    return baseClass::drawItem(idCtrl, dis);

  HDC hdc = dis->hDC;
  if(!hdc)
    return baseClass::drawItem(idCtrl, dis);

  if(ODS_FOCUS & dis->itemState)
    drawFocusBefore(hdc, dis);

  if(ODS_DISABLED & dis->itemState)
    drawDisabled(hdc, dis);
  else if(ODS_SELECTED & dis->itemState)
    drawSelected(hdc, dis);
  else
    drawUnselected(hdc, dis);

  if(ODS_FOCUS & dis->itemState)
    drawFocusAfter(hdc, dis);

  endDrawItem(hdc, dis);

  return true;
}
//-----------------------------------------------------------
#define COLOR_DISABLED GetSysColor(COLOR_BTNFACE)
//-----------------------------------------------------------
void POwnBtn::drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis)
{
  howDraw style = POwnBtn::disabled;
  if(ODS_SELECTED & dis->itemState)
    style = POwnBtn::pushedDisabled;
  drawButton(hdc, style, false);
}
//-----------------------------------------------------------
void POwnBtn::drawSelected(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  drawButton(hdc, POwnBtn::pushed, toBool(ODS_FOCUS & lpDIS->itemState));
}
//-----------------------------------------------------------
void POwnBtn::drawUnselected(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  drawButton(hdc, POwnBtn::released, toBool(ODS_FOCUS & lpDIS->itemState));
}
//-----------------------------------------------------------
// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//    if s == 0, then h = -1 (undefined)
void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
{
  float vmin;
  float vmax;
  float delta;
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
//-----------------------------------------------------------
void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
  int i;
  float f, p, q, t;
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
//-----------------------------------------------------------
COLORREF trasform(COLORREF col, float mH, float mS, float mV, bool grayed)
{
  float h;   // tonalità
  float s;   // saturazione
  float v;   // luminosità
  float r = GetRValue(col);
  r /= 255.0;
  float g = GetGValue(col);
  g /= 255.0;
  float b = GetBValue(col);
  b /= 255.0;
  RGBtoHSV(r, g, b, &h, &s, &v);

  if(grayed && v == 0) {
    s = 0.2f;
    v = 0.2f;
    h = 0;
    }
  v *= mV;
  s *= mS;
  h *= mH;

  if(v > 1.0)
    v = 1.0;
  if(s > 1.0)
    s = 1.0;
  if(h > 360.0)
    h = 360.0;

  HSVtoRGB(&r, &g, &b, h, s, v);
  uint ir = (uint)(r * 255.0 + 0.5);
  uint ig = (uint)(g * 255.0 + 0.5);
  uint ib = (uint)(b * 255.0 + 0.5);
  return RGB(min(ir, 255), min(ig, 255), min(ib, 255));
}
//-----------------------------------------------------------
// This is the simplest Edge detector
// Pixel(x0,y0) = (abs(Pixel(x0,y0)-Pixel(x+1,y+1)) + abs(Pixel(x+1,y0)-Pixel(x0,y+1)))/2
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnBtn::drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT /*dis*/)
{
  int midSize = rectColor.tickness;
  midSize += midSize / 2;
//  int midSize = rectColor.tickness / 2 + 1;
  PRect r;
  GetWindowRect(*this, r);
  SIZE sz = { r.Width() -  midSize - 1, r.Height() -  midSize - 1 };

  POINT pts[] = {
      { midSize, midSize },
      { sz.cx, midSize },
      { sz.cx, sz.cy },
      { midSize, sz.cy },
      { midSize, midSize }
      };
  HPEN pen = CreatePen(PS_SOLID, rectColor.tickness, rectColor.focus);
  HGDIOBJ old = SelectObject(hdc, pen);
  Polyline(hdc, pts, SIZE_A(pts));
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
#define CGRAY 180
//#define COLOR_TXT_DISABLED RGB(0xff, 0xff, 0xff)
//#define COLOR_TXT_DISABLED RGB(CGRAY, CGRAY, CGRAY)
#define COLOR_TXT_DISABLED GetSysColor(COLOR_GRAYTEXT)
//-----------------------------------------------------------
void POwnBtn::drawText(HDC hdc, const PRect& rect, POwnBtn::howDraw style)
{
  LPCTSTR title = getCaption();
  if(title) {

    int old = SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ font = getFont();
    if(!font)
      font = GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ oldFont = SelectObject(hdc, font);

    PRect calc = rect;
    DrawText(hdc, title, -1, calc, DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

    adjustRectText(rect, calc);

    int offs = style == pushed ? 1 : -1;
    calc.Offset(offs, offs);

    COLORREF oldColor;
    if(style == disabled)
      oldColor = SetTextColor(hdc, COLOR_TXT_DISABLED);
    else
      oldColor = SetTextColor(hdc, rectColor.txt);

    SetTextAlign(hdc, TA_LEFT | TA_TOP);
    UINT format = DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS;// | DT_END_ELLIPSIS;
    DrawText(hdc, title, -1, calc, format);

    SetTextColor(hdc, oldColor);
    SelectObject(hdc, oldFont);
    if(!getFont())
      DeleteObject(font);

    SetBkMode(hdc, old);
    }
}
//-----------------------------------------------------------
void POwnBtn::adjustRectText(const PRect& rect, PRect& calc)
{
  if(calc.right > rect.right)
    calc.right = rect.right;

  int diffW = rect.Width() - calc.Width();
  int diffH = rect.Height() - calc.Height();
  if(diffH || diffW)
    calc.Offset(diffW / 2, diffH / 2);
}
//-----------------------------------------------------------
PRect POwnBtn::getRectDrawButton(bool withFocus) const
{
  PRect r;
  GetWindowRect(*this, r);
  PRect rect(EDGE_BTN_X, EDGE_BTN_Y, r.Width() - EDGE_BTN_X, r.Height() - EDGE_BTN_Y);
  if(withFocus)
    rect.Inflate(-(int)rectColor.tickness, -(int)rectColor.tickness);
  return rect;
}
//-----------------------------------------------------------
void POwnBtn::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  if(Flat) {
    Over = true;
    InvalidateRect(*this, 0, false);
    }
}
//-----------------------------------------------------------
void POwnBtn::InvalidateParent()
{
  Over = false;
  PRect r;
  GetWindowRect(*this, r);
  MapWindowPoints(HWND_DESKTOP, *getParent(), (LPPOINT)(LPRECT)r, 2);
  InvalidateRect(*getParent(), r, true);
}
//-----------------------------------------------------------
void POwnBtn::mouseLeave(const POINT& pt, uint flag)
{
  baseClass::mouseLeave(pt, flag);
  if(Flat)
    InvalidateParent();
}
//-----------------------------------------------------------
void POwnBtn::setFlat(bool set)
{
  Flat = set;
  enableCapture(Flat);
  Over = false;
}
//-----------------------------------------------------------
void POwnBtn::drawButton(HDC hdc, POwnBtn::howDraw style, bool withFocus)
{
  PRect Rect = getRectDrawButton(withFocus);
  if((COLORREF)-1 == rectColor.bkg) {
    if(Over || !Flat && drawingEdge) {
      uint type = (style == pushed || style == pushedDisabled) ? EDGE_SUNKEN : EDGE_RAISED;
      DrawEdge(hdc, Rect, type, BF_RECT);
      }
    }
  else if(!Flat || Over) {
    HBRUSH br = CreateSolidBrush(rectColor.bkg);
    uint type = (style == pushed || style == pushedDisabled) ? (DFCS_BUTTONPUSH | DFCS_PUSHED) : DFCS_BUTTONPUSH;
    DrawFrameControl(hdc, Rect, DFC_BUTTON, type);
    PRect r(Rect);
    r.Inflate(-2, -2);
//    if(withFocus)
//      r.Inflate(-(int)rectColor.tickness, -(int)rectColor.tickness);
    FillRect(hdc, r, br);
    DeleteObject(HGDIOBJ(br));
    }
  drawCustom(hdc, Rect, style);

  drawText(hdc, Rect, style);
}
//-----------------------------------------------------------
