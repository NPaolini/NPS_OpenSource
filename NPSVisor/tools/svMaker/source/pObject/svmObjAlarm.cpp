//------------------- svmObjAlarm.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjAlarm.h"
#include "svmDefObj.h"
#include "svmPropertyAlarm.h"
#include "p_File.h"
//-----------------------------------------------------------
static PropertyAlarm staticPT;
//-----------------------------------------------------------
svmObjAlarm::svmObjAlarm(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oLB_ALARM, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjAlarm::svmObjAlarm(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oLB_ALARM, r)
{
  init();
}
//-----------------------------------------------------------
struct line_color {
      COLORREF fg_n;
      COLORREF bg_n;
      COLORREF fg_s;
      COLORREF bg_s;
};
static line_color defColors[] =
{
  { RGB(255,255,0), RGB(200,50,50), RGB(255,255,0), RGB(255,0,0) },
  { RGB(255,255,92), RGB(220,90,90), RGB(255,255,92), RGB(255,0,0) },
  { RGB(50,50,0), RGB(255,255,127), RGB(50,50,50), RGB(255,255,0) },
  { RGB(0,0,0), RGB(235,235,127), RGB(0,0,0), RGB(235,235,0) },
  { RGB(0,0,0), RGB(200,200,200), RGB(0,0,0), RGB(220,220,220) },
};
//-----------------------------------------------------------
static void copyColor(PropertyAlarm::lineColor& t, const line_color& s)
{
  t.fg_n = s.fg_n;
  t.bg_n = s.bg_n;
  t.fg_s = s.fg_s;
  t.bg_s = s.bg_s;
}
//-----------------------------------------------------------
void svmObjAlarm::init()
{
  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(allocProperty());
  for(uint i = 0; i < SIZE_A(pa->lColor); ++i)
    copyColor(pa->lColor[i], defColors[i]);
  pa->DataPrf[0].typeVal = 4;
  pa->DataPrf[1].typeVal = 4;
  pa->DataPrf[2].typeVal = 14;
  pa->DataPrf[2].normaliz = 20;
  Prop = pa;

  Prop->Rect = getFirstRect();
  Prop->Rect.right = Prop->Rect.left + R__X(300);
  Prop->Rect.bottom = Prop->Rect.top + R__Y(150);
}
//-----------------------------------------------------------
Property* svmObjAlarm::allocProperty() { return new PropertyAlarm; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjAlarm::dialogProperty(bool onCreate)
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
    if(IDOK == svmDialogAlarm(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void svmObjAlarm::DrawObject(HDC hdc)
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

  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(Prop);

  COLORREF bn = pa->lColor[MAX_LINE_COLOR - 1].bg_n;

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
static void myRect(HDC hdc, const PRect& r)
{
  POINT pt[] = {
    { r.left, r.top },
    { r.right - 1, r.top },
    { r.right - 1, r.bottom },
    { r.left, r.bottom },
    { r.left, r.top },
    };
Polyline(hdc, pt, SIZE_A(pt));
}
//-----------------------------------------------------------
void svmObjAlarm::DrawTitle(HDC hdc)
{
  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(Prop);

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

  COLORREF fn = pa->lColor[MAX_LINE_COLOR - 1].fg_n;
  COLORREF bn = pa->lColor[MAX_LINE_COLOR - 1].bg_n;
  COLORREF fs = pa->lColor[MAX_LINE_COLOR - 1].fg_s;
  COLORREF bs = pa->lColor[MAX_LINE_COLOR - 1].bg_s;

  COLORREF oldBkg = SetBkColor(hdc, bn);
  COLORREF oldFg = SetTextColor(hdc, fn);
  r.bottom -= GetSystemMetrics(SM_CYBORDER) * 3;
  ExtTextOut(hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, r, 0, 0, 0);

  r.bottom = r.top + height;

  SetBkColor(hdc, pa->lColor[0].bg_s);
  SetTextColor(hdc, pa->lColor[0].fg_s);
  LPCTSTR txt = getStrAlarm(0);
  ExtTextOut(hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, r, txt, _tcslen(txt), 0);
  DrawFocusRect(hdc, r);
  r.Offset(0, height);

  uint ix = 0;
  while(r.bottom < bottom && ix < MAX_LINE_COLOR) {
    SetBkColor(hdc, pa->lColor[ix].bg_n);
    SetTextColor(hdc, pa->lColor[ix].fg_n);
    LPCTSTR txt = getStrAlarm(ix);
    ExtTextOut(hdc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, r, txt, _tcslen(txt), 0);
    myRect(hdc, r);
    r.Offset(0, height);
    ++ix;
//    ix %= MAX_LINE_COLOR;
    }
  SetBkColor(hdc, oldBkg);
  SetTextColor(hdc, oldFg);
  SelectObject(hdc, oldFont);
  baseClass::DrawTitle(hdc);
}
//-----------------------------------------------------------
smartPointerConstString svmObjAlarm::getTitle() const
{
  return smartPointerConstString(0, 0);//"List Box";
}
//-----------------------------------------------------------
PRect svmObjAlarm::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
//  r.Inflate(-2, -4);
//  r.right -= GetSystemMetrics(SM_CXBORDER) * 5 + GetSystemMetrics(SM_CXVSCROLL);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjAlarm::makeClone()
{
  svmObjAlarm* obj = new svmObjAlarm(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjAlarm::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmObjAlarm::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
//  int id = ID_INIT_VAR_ALARM;
//  id += order;

  manageObjId moi(0, ID_INIT_VAR_ALARM);
  uint id = moi.calcAndSetExtendId(order);

  TCHAR buff[1000];
  int style = 0;
  if(Prop->style & Property::TRANSP)
    style |= 1;
  if(Prop->negative)
    style |= 2;

  PRect r(Prop->Rect);
  r.Inflate(-R__X(2), -R__Y(2));

  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(Prop);

  svmObjCount& objCount = getObjCount();
  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idFilter = objCount.getIdCount(oEDIT);
  idFilter = moi2.calcBaseExtendId(idFilter);

  int idFilterGrp = objCount.getIdCount(oEDIT);
  idFilterGrp = moi2.calcBaseExtendId(idFilterGrp);

  int idFilterTxt = objCount.getIdCount(oEDIT);
  idFilterTxt = moi2.calcBaseExtendId(idFilterTxt);

//  int idFilter = objCount.getIdCount(oEDIT) + ID_INIT_VAR_EDI;
//  int idFilterGrp = objCount.getIdCount(oEDIT) + ID_INIT_VAR_EDI;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            id,
            REV__X(r.left),
            REV__X(r.top),
            REV__X(r.Width()),
            REV__X(r.Height()),
            Prop->idFont + ID_INIT_FONT,
            order + 1,
            pa->PA_fix_filter,
            idFilter,
            pa->PA_def_show
            );

  writeStringChkUnicode(pfCrypt, buff);
  LPTSTR p = buff;
  for(uint i = 0; i < SIZE_A(pa->percWidth); ++i) {
    wsprintf(p, _T(",%d"), pa->percWidth[i]);
    p += _tcslen(p);
    }
  writeStringChkUnicode(pfCrypt, buff);

  p = buff;
  for(uint i = 0; i < SIZE_A(pa->columnPos); ++i) {
    wsprintf(p, _T(",%d"), pa->columnPos[i]);
    p += _tcslen(p);
    }
  wsprintf(p, _T(",%d,%d\r\n"), idFilterGrp, idFilterTxt);
  writeStringChkUnicode(pfCrypt, buff);

  pa->DataPrf[0].typeVal = 4;
  pa->typeVal = 4;
  savePrf(pfCrypt, buff, idFilter, pa->DataPrf[0]);

  pa->DataPrf[1].typeVal = 4;
  savePrf(pfCrypt, buff, idFilterGrp, pa->DataPrf[1]);

  pa->DataPrf[2].typeVal = 14;
  savePrf(pfCrypt, buff, idFilterTxt, pa->DataPrf[2]);

  saveVar(pfCrypt, id);

  wsprintf(buff, _T("%d"), moi.getThirdExtendId());
  p = buff;
  for(uint i = 0; i < MAX_LINE_COLOR; ++i) {
    p += _tcslen(p);
    PropertyAlarm::lineColor& lc = pa->lColor[i];
    wsprintf(p, _T(",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            GetRValue(lc.fg_n),
            GetGValue(lc.fg_n),
            GetBValue(lc.fg_n),

            GetRValue(lc.bg_n),
            GetGValue(lc.bg_n),
            GetBValue(lc.bg_n),

            GetRValue(lc.fg_s),
            GetGValue(lc.fg_s),
            GetBValue(lc.fg_s),

            GetRValue(lc.bg_s),
            GetGValue(lc.bg_s),
            GetBValue(lc.bg_s)
            );
    }
  p += _tcslen(p);
  wsprintf(p, _T("\r\n"));

  writeStringChkUnicode(pfCrypt, buff);
//  if(Prop->visPerif)
    saveOnlyVisVar(moi.getSecondExtendId(), pfCrypt);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjAlarm::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int filterFix = 0;
  int idFilter = 0;
  int idFilterGrp = 0;
  int whichshow = 0;
  int idFont = ID_INIT_FONT;
  int idObj = 0;
  int idFilterTxt = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h, &idFont,
                    &idObj, &filterFix,
                    &idFilter, &whichshow
                    );
  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(Prop);
  p = findNextParamTrim(p, 9);
  for(uint i = 0; p && i < SIZE_A(pa->percWidth); ++i) {
    pa->percWidth[i] = _ttoi(p);
    p = findNextParamTrim(p);
    }

  for(uint i = 0; p && i < SIZE_A(pa->columnPos); ++i) {
    pa->columnPos[i] = _ttoi(p);
    p = findNextParamTrim(p);
    }
  if(p) {
    idFilterGrp = _ttoi(p);
    p = findNextParamTrim(p);
    if(p) 
      idFilterTxt = _ttoi(p);
    }
  Prop->idFont = idFont - ID_INIT_FONT;

  pa->PA_fix_filter = filterFix;
  pa->PA_def_show = whichshow;

  loadVar(id, set);
  loadPrf(idFilter, set, pa->DataPrf[0]);
  pa->DataPrf[0].typeVal = 4;
  loadPrf(idFilterGrp, set, pa->DataPrf[1]);
  pa->DataPrf[1].typeVal = 4;
  pa->typeVal = 4;

  loadPrf(idFilterTxt, set, pa->DataPrf[2]);
  pa->DataPrf[2].typeVal = 14;

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

  manageObjId moi(id, ID_INIT_VAR_ALARM);
  p = set.getString(moi.getThirdExtendId());
  if(p) {
    for(uint i = 0; i < MAX_LINE_COLOR && p; ++i) {
      PropertyAlarm::lineColor& lc = pa->lColor[i];
      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &r1, &g1, &b1,
                    &r2, &g2, &b2,
                    &r3, &g3, &b3,
                    &r4, &g4, &b4
                    );
      p = findNextParamTrim(p, 12);
      lc.fg_n = RGB(r1, g1, b1);
      lc.bg_n = RGB(r2, g2, b2);
      lc.fg_s = RGB(r3, g3, b3);
      lc.bg_s = RGB(r4, g4, b4);
      }
    }
  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  Prop->Rect.Inflate(R__X(2), R__Y(2));

  loadOnlyVisVar(set, moi.getSecondExtendId());
  return true;
}
//-----------------------------------------------------------
void svmObjAlarm::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(Prop);
  LPCTSTR info[] = { _T("Filtro"), _T("Filtro gruppi"), _T("Filtro testo") };
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const dataPrf& dp = pa->DataPrf[i];
    if(!dp.perif)
      continue;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, info[i]);
    ia4->prph = dp.perif;
    ia4->addr = dp.addr;
    ia4->type = dp.typeVal;
    ia4->nBit = 0;
    ia4->offs = 0;
    set.add(ia4);
    }
}
