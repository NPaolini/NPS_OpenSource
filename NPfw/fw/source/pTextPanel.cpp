//-------------------- pTextPanel.CPP ---------------------------
#include "precHeader.h"

#include "pTextPanel.h"
#include "p_Util.h"


#define DELTA_RECT 2
#define DELTA_3D 1
//#define DELTA_3D 5
#define _Y(a) a

#define BLACK RGB(0, 0, 0)
#define WHITE RGB(0xff, 0xff, 0xff)
//-----------------------------------------------------------
PTextPanel::PTextPanel(LPCTSTR str, int x, int y, HFONT font,
    COLORREF txt, PPanel::bStyle type, COLORREF bkg, b3DStyle style3d) :
    PPanel(PRect(x, y, x, y), bkg, type), TxtColor(txt), Style3D(style3d),
    Align(TA_LEFT),vAlign(DT_TOP),
    Str(str_newdup(str)), Font(font)
{ }
//-----------------------------------------------------------
PTextPanel::~PTextPanel()
{
  delete []Str;
}
//-----------------------------------------------------------
void PTextPanel::clone(const PTextPanel& other)
{
  if(&other != this) {
    PPanel::clone(other);
    delete []Str;
    Str = str_newdup(other.Str);
    CLONE(TxtColor);
    CLONE(Font);
    CLONE(Style3D);
    CLONE(Align);
    CLONE(vAlign);
    }
}
//-----------------------------------------------------------
void PTextPanel::drawText(HDC hdc, LPCTSTR txt)
{
  LPTSTR old = Str;
  if(txt)
    Str = (LPTSTR)txt;
  draw(hdc);
  Str = old;
}
//-----------------------------------------------------------
void PTextPanel::setText(int value)
{
  TCHAR t[64];
  wsprintf(t, _T("%d"), value);
  setText(t);
}
//-----------------------------------------------------------
bool PTextPanel::setText(LPCTSTR newString)
{
  if(Str && newString) {
    if(_tcscmp(Str, newString)) {
      delete []Str;
      Str = str_newdup(newString);
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PTextAutoPanel::PTextAutoPanel(LPCTSTR str, int x, int y,
                HFONT font, COLORREF txt, PPanel::bStyle type, COLORREF bkg,
                b3DStyle style3d) :
    PTextPanel(str, x, y, font, txt, type, bkg, style3d),
    calculated(false), X(x), Y(y)
{
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PTextAutoPanel::draw(HDC hdc)
{
  if(isHided())
    return;
  int x = X;
  int y = Y;
  HFONT oldF = 0;
  if(getFont())
    oldF = (HFONT) SelectObject(hdc, getFont());
  if(getStyle() != PPanel::NO) {
    if(!calculated) {
      SIZE sz;
      int len = _tcslen(getText());
      if(!GetTextExtentPoint32(hdc, getText(), len, &sz)) {
        if(getFont())
          SelectObject(hdc, oldF);
        return;
        }
      sz.cx += len;
      PRect rect(x - DELTA_RECT, _Y(y - DELTA_RECT),
                sz.cx + x + DELTA_RECT, _Y(sz.cy + y + DELTA_RECT));

      if((TA_RIGHT & getAlign()) == TA_RIGHT)
        rect.Offset(-sz.cx, 0);
      else if((TA_CENTER & getAlign()) == TA_CENTER)
        rect.Offset(-sz.cx / 2, 0);

      setRect(rect);
      calculated = true;
      }
    if((TA_RIGHT & getAlign()) == TA_RIGHT)
      x -= getRect().Width() / 2;
    else if((TA_LEFT & getAlign()) == TA_LEFT)
      x += getRect().Width() / 2;
    PPanel::draw(hdc);
    SetTextAlign(hdc, TA_CENTER | TA_TOP);
    }
  else {
//    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, getAlign() | TA_TOP);
    }
  SetBkMode(hdc, TRANSPARENT);
  COLORREF old;
  if((PTextPanel::White &  get3DStyle()) == PTextPanel::White) {
    old = SetTextColor(hdc, WHITE);
    TextOut(hdc, x - DELTA_3D, _Y(y - DELTA_3D), getText(), (int)_tcslen(getText()));
    }
  if((PTextPanel::Black &  get3DStyle()) == PTextPanel::Black) {
    old = SetTextColor(hdc, BLACK);
    TextOut(hdc, x + DELTA_3D, _Y(y + DELTA_3D), getText(), (int)_tcslen(getText()));
    }
  SetTextColor(hdc, getTextColor());
  TextOut(hdc, x, _Y(y), getText(), (int)_tcslen(getText()));
  SetTextColor(hdc, old);
  if(getFont())
    SelectObject(hdc, oldF);
}
//-----------------------------------------------------------
bool PTextAutoPanel::setText(LPCTSTR newString)
{
  bool changed = PTextPanel::setText(newString);
  if(changed)
    calculated = false;
  return changed;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PTextFixedPanel::PTextFixedPanel(LPCTSTR str, const PRect& rect,
      HFONT font, COLORREF txt, PPanel::bStyle type,
      COLORREF bkg, b3DStyle style3d) :
      PTextPanel(str, rect.left, rect.top, font, txt, type, bkg, style3d)
{
  setRect(rect);
}
//-----------------------------------------------------------
static void adjustRectText(PRect& rect, const PRect& calc, int vAlign)
{
  switch(vAlign) {
    case DT_TOP:
    default:
      break;
    case DT_BOTTOM:
      rect.top = rect.bottom - calc.Height();
      break;
    case DT_VCENTER:
      rect.top = rect.bottom - (rect.Height() - calc.Height()) / 2 - calc.Height();
      break;
    }
}
//-----------------------------------------------------------
void PTextFixedPanel::draw(HDC hdc)
{
  if(isHided())
    return;
  HFONT oldF = 0;
  if(getFont())
    oldF = (HFONT) SelectObject(hdc, getFont());

  int oldMode;// = -1;
  if(getStyle() != PPanel::NO)
    PPanel::draw(hdc);
//  else
    oldMode = SetBkMode(hdc, TRANSPARENT);
  RECT trect = { getRect().left,
                 getRect().top,
                 getRect().right - DELTA_3D,
                 getRect().bottom - DELTA_3D
              };
  DPtoLP(hdc, reinterpret_cast<POINT*>(&trect), 2);
  POINT pt = { DELTA_3D, DELTA_3D };
  DPtoLP(hdc, &pt, 1);

  PRect rect(trect);
  rect.Inflate(-DELTA_RECT, -DELTA_3D);
  SetTextAlign(hdc, TA_LEFT | TA_TOP);
  UINT format;
  if((getAlign() & TA_CENTER) == TA_CENTER)
    format = DT_CENTER;
  else if((getAlign() & TA_RIGHT) == TA_RIGHT)
    format = DT_RIGHT;
  else
    format = DT_LEFT;
  format |= DT_WORDBREAK | DT_EXPANDTABS;

  PRect calc = rect;
  DrawText(hdc, getText(), -1, calc, format | DT_CALCRECT);
  adjustRectText(rect, calc, getVAlign());

  COLORREF oldColor = (COLORREF)-1;
  if((PTextPanel::Black &  get3DStyle()) == PTextPanel::Black) {
    rect.Offset(pt.x, pt.y);
    rect.Offset(pt.x, pt.y);
    oldColor = SetTextColor(hdc, BLACK);
    DrawText(hdc, getText(), -1, rect, format);
    rect.Offset(-pt.x, -pt.y);
    }
  if((PTextPanel::White &  get3DStyle()) == PTextPanel::White) {
    rect.Offset(-pt.x,-pt.y);
    COLORREF oldColor2 = SetTextColor(hdc, WHITE);
    if((COLORREF)-1 == oldColor)
      oldColor = oldColor2;
    DrawText(hdc, getText(), -1, rect, format);
    rect.Offset(pt.x, pt.y);
    }
  COLORREF oldColor2 = SetTextColor(hdc, getTextColor());
  if((COLORREF)-1 == oldColor)
    oldColor = oldColor2;
  DrawText(hdc, getText(), -1, rect, format);
  SetTextColor(hdc, oldColor);
  if(getFont())
    SelectObject(hdc, oldF);

  if(-1 != oldMode)
    SetBkMode(hdc, oldMode);
}
