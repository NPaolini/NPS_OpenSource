//------------------- svmObjListBox.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjListBox.h"
#include "svmDefObj.h"
#include "svmPropertyListBox.h"
#include "p_File.h"
//-----------------------------------------------------------
static Property staticPT;
//-----------------------------------------------------------
svmObjListBox::svmObjListBox(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oLISTBOX, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjListBox::svmObjListBox(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oLISTBOX, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjListBox::init()
{
  Prop = new Property;
  Prop->foreground = GetSysColor(COLOR_WINDOWTEXT);
  Prop->background = GetSysColor(COLOR_WINDOW);
  Prop->type1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
  Prop->type2 = GetSysColor(COLOR_HIGHLIGHT);
  Prop->Rect = getFirstRect();
  Prop->Rect.right = Prop->Rect.left + 150;
  Prop->Rect.bottom = Prop->Rect.top + 100;
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjListBox::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      *Prop = staticPT;
      }
    else
      staticPT = *Prop;
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogListBox(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void svmObjListBox::DrawObject(HDC hdc)
{
//  PPanel panel(getRect(), Prop->background, Prop->style);
//  panel.draw(hdc);

//  staticPT.Rect = getRect();

  PRect r(getRect());

  HPEN hpen = CreatePen(PS_SOLID, R__X(2), RGB(0xff, 0, 0));
  HGDIOBJ old = SelectObject(hdc, hpen);
  POINT pt[] = {
    { r.left, r.top },
    { r.right, r.top },
    { r.right, r.bottom },
    { r.left, r.bottom },
    { r.left, r.top },
    };
  Polyline(hdc, pt, SIZE_A(pt));
  DeleteObject(SelectObject(hdc, old));

  r.Inflate(-R__X(2), -R__Y(2));

  COLORREF bn = Prop->background;
  if(!(Prop->style & Property::TRANSP))
    bn = GetSysColor(COLOR_WINDOW);
  HBRUSH brush = CreateSolidBrush(bn);
  FillRect(hdc, r, brush);
  DeleteObject((HGDIOBJ)brush);

  DrawEdge(hdc, r, EDGE_SUNKEN, BF_RECT);
  r.left = r.right - GetSystemMetrics(SM_CXVSCROLL);
  r.Offset(-2, 0);

  r.Inflate(0, -4);
  brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  FillRect(hdc, r, brush);
  DeleteObject((HGDIOBJ)brush);
  r.Inflate(0, 4);
  r.Offset(0, 2);

  r.bottom = r.top + r.Width();
  DrawFrameControl(hdc, r, DFC_SCROLL, DFCS_SCROLLUP | DFCS_INACTIVE);
  r.Offset(0, getRect().Height() - R__Y(2) - r.Height() - 4);
  DrawFrameControl(hdc, r, DFC_SCROLL, DFCS_SCROLLDOWN | DFCS_INACTIVE);
  r.Offset(-r.Width(), 0);
  r.top = getRect().top + 2;
  DrawEdge(hdc, r, EDGE_SUNKEN, BF_RIGHT);

}
//-----------------------------------------------------------
void svmObjListBox::DrawTitle(HDC hdc)
{
  COLORREF fn = Prop->foreground;
  COLORREF bn = Prop->background;

  COLORREF fs = Prop->type1;
  COLORREF bs = Prop->type2;

  if(!(Prop->style & Property::TRANSP)) {
    fn = GetSysColor(COLOR_WINDOWTEXT);
    bn = GetSysColor(COLOR_WINDOW);
    fs = GetSysColor(COLOR_HIGHLIGHTTEXT);
    bs = GetSysColor(COLOR_HIGHLIGHT);
    }

  PRect r(getRect());

  r.Inflate(-R__X(2), -R__Y(2));

  HFONT fnt = Prop->getFont(Linked);
  HGDIOBJ oldFont = SelectObject(hdc, fnt);

  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);

  int height = tm.tmHeight + R__Y(2);

  int bottom = r.bottom - GetSystemMetrics(SM_CYBORDER);
  r.Offset(GetSystemMetrics(SM_CXBORDER) * 2, GetSystemMetrics(SM_CYBORDER) * 2);
  r.right -= GetSystemMetrics(SM_CXBORDER) * 6 + GetSystemMetrics(SM_CXVSCROLL);
  r.bottom = r.top + height;
//  r.bottom = r.top + R__Y(16);
  LPCTSTR txt = _T("Esempio Listbox");


  COLORREF oldBkg = SetBkColor(hdc, bs);
  COLORREF oldFg = SetTextColor(hdc, fs);
  ExtTextOut(hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, r, txt, _tcslen(txt), 0);
  r.Offset(0, height);
//  r.Offset(0, R__Y(16));
  SetBkColor(hdc, bn);
  SetTextColor(hdc, fn);
  while(r.bottom < bottom) {
    ExtTextOut(hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, r, txt, _tcslen(txt), 0);
    r.Offset(0, height);
    }
  SetBkColor(hdc, oldBkg);
  SetTextColor(hdc, oldFg);
  SelectObject(hdc, oldFont);
  baseClass::DrawTitle(hdc);
}
//-----------------------------------------------------------
smartPointerConstString svmObjListBox::getTitle() const
{
  return smartPointerConstString(0, 0);//"List Box";
}
//-----------------------------------------------------------
PRect svmObjListBox::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
//  r.Inflate(-2, -4);
//  r.right -= GetSystemMetrics(SM_CXBORDER) * 5 + GetSystemMetrics(SM_CXVSCROLL);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjListBox::makeClone()
{
  svmObjListBox* obj = new svmObjListBox(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjListBox::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmObjListBox::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_LBOX);
  uint id = moi.calcAndSetExtendId(order);

  TCHAR buff[1000];
  int style = 0;
  if(Prop->style & Property::TRANSP)
    style |= 1;
  if(Prop->negative)
    style |= 2;

  PRect r(Prop->Rect);
  r.Inflate(-R__X(2), -R__Y(2));

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(r.left),
            REV__X(r.top),
            REV__X(r.Width()),
            REV__X(r.Height()),
            style,
            Prop->idFont + ID_INIT_FONT);
  writeStringChkUnicode(pfCrypt, buff);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            moi.getFirstExtendId(),
            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),

            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),

            GetRValue(Prop->type1),
            GetGValue(Prop->type1),
            GetBValue(Prop->type1),

            GetRValue(Prop->type2),
            GetGValue(Prop->type2),
            GetBValue(Prop->type2)

            );

  writeStringChkUnicode(pfCrypt, buff);
//  if(Prop->visPerif)
    saveOnlyVisVar(moi.getSecondExtendId(), pfCrypt);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjListBox::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int style = 0;
  int idFont = ID_INIT_FONT;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &x, &y, &w, &h, &style, &idFont);

  Prop->style = style & 1 ? Property::TRANSP : Property::NO;
  Prop->negative = toBool(style & 2);
  Prop->idFont = idFont - ID_INIT_FONT;

  int r1 = 0;
  int g1 = 0;
  int b1 = 0;

  int r2 = 220;
  int g2 = 220;
  int b2 = 220;

  int r3 = 0;
  int g3 = 0;
  int b3 = 0;

  int r4 = 220;
  int g4 = 220;
  int b4 = 220;

  manageObjId moi(id, ID_INIT_VAR_LBOX);
  p = set.getString(moi.getFirstExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &r1, &g1, &b1,
                    &r2, &g2, &b2,
                    &r3, &g3, &b3,
                    &r4, &g4, &b4
                    );

  Prop->foreground = RGB(r1, g1, b1);
  Prop->background = RGB(r2, g2, b2);
  Prop->type1 = RGB(r3, g3, b3);
  Prop->type2 = RGB(r4, g4, b4);

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  Prop->Rect.Inflate(R__X(2), R__Y(2));

  loadOnlyVisVar(set, moi.getSecondExtendId());
  return true;
}
