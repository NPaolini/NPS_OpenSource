//-------------------- svmObjDiam.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
#include "pPanel.h"
#include "pModDialog.h"
#include "svmObjDiam.h"
#include "svmDefObj.h"
#include "svmPropertyDiam.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyDiam staticPT;
//-----------------------------------------------------------
svmObjDiam::svmObjDiam(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oDIAM, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjDiam::svmObjDiam(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oDIAM, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjDiam::init()
{
  Prop = new PropertyDiam;
  PropertyDiam* po = dynamic_cast<PropertyDiam*>(Prop);
  setOfDiamColors& set = po->getSet();
  diamColors dc = set[0];
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjDiam::setOffsetAddr(const infoOffsetReplace& ior)
{
  PropertyDiam* p = (PropertyDiam*)Prop;
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isValidPrph4OffsetAddr(p->DataPrf[1].perif, ior))
      if(isValid4OffsetAddr(p->DataPrf[1].perif, p->DataPrf[1].addr, isSetBitf(eoExcludePrph1, ior.flag)))
        p->DataPrf[1].addr += ior.offs;
    if(isValidPrph4OffsetAddr(Prop->visPerif, ior))
      if(isValid4OffsetAddr(Prop->visPerif, Prop->visAddr, isSetBitf(eoExcludePrph1, ior.flag)) && isSetBitf(eoAlsoVisibility, ior.flag)) {
        Prop->visAddr += ior.offs;
        Prop->visOffset += ior.bitOffs;
        }
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      if(isValidPrph4OffsetAddr(p->DataPrf[0].perif, ior))
        if(isValid4OffsetAddr(p->DataPrf[0].perif, p->DataPrf[0].addr, isSetBitf(eoExcludePrph1, ior.flag)))
          p->DataPrf[0].addr += ior.offs;
      if(isValidPrph4OffsetAddr(Prop->perif, ior))
        if(isValid4OffsetAddr(Prop->perif, Prop->addr, isSetBitf(eoExcludePrph1, ior.flag))) {
          Prop->addr += ior.offs;
          Prop->offset += ior.bitOffs;
          }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    if(ior.prphFrom == Prop->perif)
      Prop->perif = ior.prphTo;
    if(ior.prphFrom == p->DataPrf[0].perif)
      p->DataPrf[0].perif = ior.prphTo;
    if(ior.prphFrom == p->DataPrf[1].perif)
      p->DataPrf[1].perif = ior.prphTo;
    if(isSetBitf(eoAlsoPrphVisibility, ior.flag) && ior.prphFrom == Prop->visPerif)
      Prop->visPerif = ior.prphTo;
    }
  if(isSetBitf(eoSubstVar, ior.flag)) {
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom2 == p->DataPrf[i].perif && ior.addrFrom == p->DataPrf[i].addr) {
        p->DataPrf[i].perif = ior.prphTo2;
        p->DataPrf[i].addr = ior.addrTo;
        }
      }
    if(ior.prphFrom2 == Prop->visPerif && ior.addrFrom == Prop->visAddr) {
      Prop->visPerif = ior.prphTo2;
      Prop->visAddr = ior.addrTo;
      }
    }
}
//----------------------------------------------------------------------------
static void squareRect(PRect& r)
{
  int width = r.Width();
  int height = r.Height();

  if(width > height) {
    r.left += (width - height) / 2;
    r.right = r.left + height;
    }
  else if(width < height) {
    r.top += (height - width) / 2;
    r.bottom = r.top + width;
    }
}
//----------------------------------------------------------------------------
#define DEF_INFLATE 2
//----------------------------------------------------------------------------
static void calcRect(const PRect& full, PRect& crown, PRect& pivot)
{
  crown = full;
  pivot = full;
//  squareRect(crown);
//  squareRect(pivot);
  int width = crown.Width();
  width /= 10;
  if(width < DEF_INFLATE * 2)
    width = DEF_INFLATE * 2;
  crown.Inflate(-width, -width);

  width = pivot.Width() / 2;
  width -= width / 5;
  if(width < DEF_INFLATE)
    width = DEF_INFLATE;
  pivot.Inflate(-width, -width);
}
//----------------------------------------------------------------------------
void svmObjDiam::drawWithCycle(HDC hdc)
{
  PRect rCurr;
  PRect rPivot;
  calcRect(getRect(), rCurr, rPivot);

  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  const setOfDiamColors& set = pt->getSet() ;
  const diamColors& c = set[0];

  HPEN penCurr = CreatePen(PS_SOLID, 1, c.Color[diamColors::cCROWN]);

  HGDIOBJ oldPen = SelectObject(hdc, penCurr);

// non ha senso il disegno nella direzione opposta
#if 1
  int dim = (rCurr.Width() - rPivot.Width()) / 2;
  for(int i = 0; i < dim; ++i)
    Arc(hdc, rPivot.left - i, rPivot.top - i, rPivot.right + i, rPivot.bottom + i, 0, 0, 0, 0);
#else
  if(PropertyDiam::INVERT & pt->type1) {
    PRect t = rCurr;
    rCurr = rPivot;
    rPivot = t;
    }
  int dim = (rCurr.Width() - rPivot.Width()) / 2;
  if(dim < 0)
    for(int i = dim; i < 0; ++i)
      Arc(hdc, rPivot.left - i, rPivot.top - i, rPivot.right + i, rPivot.bottom + i, 0, 0, 0, 0);
  else
    for(int i = 0; i < dim; ++i)
      Arc(hdc, rPivot.left - i, rPivot.top - i, rPivot.right + i, rPivot.bottom + i, 0, 0, 0, 0);
#endif
  SelectObject(hdc, oldPen);
  DeleteObject(penCurr);
}
//----------------------------------------------------------------------------
void svmObjDiam::drawFilled(HDC hdc)
{
  PRect rCurr;
  PRect rPivot;
  calcRect(getRect(), rCurr, rPivot);

  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  const setOfDiamColors& set = pt->getSet() ;
  const diamColors& c = set[0];

  HBRUSH brushCurr = CreateSolidBrush(c.Color[diamColors::cCROWN]);
  HPEN penCurr = CreatePen(PS_SOLID, 1, c.Color[diamColors::cCROWN]);

  HGDIOBJ oldBrush = SelectObject(hdc, brushCurr);
  HGDIOBJ oldPen = SelectObject(hdc, penCurr);

  Ellipse(hdc, rCurr.left, rCurr.top, rCurr.right, rCurr.bottom);

  HBRUSH brushPivot = CreateSolidBrush(c.Color[diamColors::cPIVOT]);
  HPEN penPivot = CreatePen(PS_SOLID, 1, c.Color[diamColors::cPIVOT]);

  SelectObject(hdc, brushPivot);
  SelectObject(hdc, penPivot);

  Ellipse(hdc, rPivot.left, rPivot.top, rPivot.right, rPivot.bottom);

  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);

  DeleteObject(penPivot);
  DeleteObject(brushPivot);
  DeleteObject(penCurr);
  DeleteObject(brushCurr);
}
//----------------------------------------------------------------------------
void svmObjDiam::drawMaxVal(HDC hdc)
{
  HPEN penMax = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff)); //Colors[currColor].type[cCROWN]);
  uint oldMode = SetBkMode(hdc, OPAQUE);
  COLORREF oldColor = SetBkColor(hdc, RGB(0, 0, 0));
  HGDIOBJ oldPen = SelectObject(hdc, penMax);
  PRect minRect(getRect());
//  squareRect(minRect);
  minRect.Inflate(-DEF_INFLATE, -DEF_INFLATE);
  Arc(hdc, minRect.left, minRect.top, minRect.right, minRect.bottom, 0, 0, 0, 0);
  SetBkColor(hdc, oldColor);
  SetBkMode(hdc, oldMode);
  SelectObject(hdc, oldPen);
  DeleteObject(penMax);
}
//----------------------------------------------------------------------------
void svmObjDiam::drawAxis(HDC hdc)
{
  PRect r = getRect();
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  const setOfDiamColors& set = pt->getSet() ;
  const diamColors& c = set[0];

  HPEN penAxis = CreatePen(PS_DASHDOT, 1, c.Color[diamColors::cAXIS]);
  HGDIOBJ oldPen = SelectObject(hdc, penAxis);

  int width = r.Width();
  MoveToEx(hdc, r.left + width / 2, r.top, 0);
  LineTo(hdc, r.left + width / 2, r.bottom);

  int height = r.Height();
  MoveToEx(hdc, r.left, r.top + height / 2, 0);
  LineTo(hdc, r.right, r.top + height / 2);

  SelectObject(hdc, oldPen);
  SetBkMode(hdc, oldMode);

  DeleteObject(penAxis);
}
//-----------------------------------------------------------
void svmObjDiam::DrawObject(HDC hdc)
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  if(pt) {
    const setOfDiamColors& set = pt->getSet() ;
    const diamColors& c = set[0];
    PPanel panel(getRect(), c.Color[diamColors::cBKG], (PPanel::bStyle)Prop->style);
    panel.draw(hdc);
//    staticPT.Rect = getRect();

    bool transp = toBool(pt->style & Property::TRANSP);

    transp &= toBool(PropertyDiam::TRANSP_PIVOT & pt->type1);
    if(transp)
      drawWithCycle(hdc);
    else
      drawFilled(hdc);

    if(PropertyDiam::DRAW_MAX & pt->type1)
      drawMaxVal(hdc);

    if(PropertyDiam::AXIS & pt->type1)
      drawAxis(hdc);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjDiam::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjDiam::getRectTitle(HDC, LPCTSTR /*title*/) const
{
#if 1
  return Prop->Rect;
#else
  PRect r(Prop->Rect);
  r.Inflate(-2, 0);
  return r;
#endif
}
//-----------------------------------------------------------
svmObject* svmObjDiam::makeClone()
{
  svmObjDiam* obj = new svmObjDiam(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjDiam::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjDiam::allocProperty() { return new PropertyDiam; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjDiam::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyDiam*>(Prop))->cloneMinusProperty(staticPT);
      }
    else {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      }
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogDiam(this, Prop, getParent()).modal()) {
      Prop->Rect.bottom = Prop->Rect.top + Prop->Rect.Width();
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void saveColor(P_File& pf, LPTSTR buff, int id, const diamColors& c)
{
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            GetRValue(c.Color[diamColors::cCROWN]),
            GetGValue(c.Color[diamColors::cCROWN]),
            GetBValue(c.Color[diamColors::cCROWN]),

            GetRValue(c.Color[diamColors::cPIVOT]),
            GetGValue(c.Color[diamColors::cPIVOT]),
            GetBValue(c.Color[diamColors::cPIVOT]),

            GetRValue(c.Color[diamColors::cAXIS]),
            GetGValue(c.Color[diamColors::cAXIS]),
            GetBValue(c.Color[diamColors::cAXIS]),

            GetRValue(c.Color[diamColors::cBKG]),
            GetGValue(c.Color[diamColors::cBKG]),
            GetBValue(c.Color[diamColors::cBKG])
            );
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadColor(uint id, setOfString& set, diamColors& c)
{
  LPCTSTR p = set.getString(id);
  int cols[4][3];
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            &cols[0][0], &cols[0][1], &cols[0][2],
            &cols[1][0], &cols[1][1], &cols[1][2],
            &cols[2][0], &cols[2][1], &cols[2][2],
            &cols[3][0], &cols[3][1], &cols[3][2]
            );
  for(int i = 0; i < 4; ++i)
    c.Color[i] = RGB(cols[i][0], cols[i][1], cols[i][2]);
}
//-----------------------------------------------------------
int svmObjDiam::getStyle1()
{
  int style = 0;
  if(Property::BORDER & Prop->style)
    style = 1;
  else if(Property::UP & Prop->style)
    style = 2;
  else if(Property::DN & Prop->style)
    style = 3;

  if(Property::FILL & Prop->style)
    style += 8;
  else if(Property::TRANSP & Prop->style)
    style += 4;
  return style;
}
//-----------------------------------------------------------
void svmObjDiam::setStyle1(int style)
{
  Prop->style = 0;
  if(style >= 8) {
    style -= 8;
    Prop->style |= Property::FILL;
    }

  if(style >= 4) {
    style -= 4;
    Prop->style |= Property::TRANSP;
    }

  switch(style) {
    case 1:
      Prop->style |= Property::BORDER;
      break;
    case 2:
      Prop->style |= Property::UP;
      break;
    case 3:
      Prop->style |= Property::DN;
      break;
    }
}
//-----------------------------------------------------------
int svmObjDiam::getStyle2()
{
  return Prop->type1;
}
//-----------------------------------------------------------
void svmObjDiam::setStyle2(int style)
{
  Prop->type1 = style;
}
//-----------------------------------------------------------
bool svmObjDiam::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_DIAM);
  uint id = moi.calcAndSetExtendId(order);

  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style1 = getStyle1();
  int style2 = getStyle2();

  svmObjCount& objCount = getObjCount();
  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idMin = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMax = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idCurr = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));

  int idColor = 0;

  const setOfDiamColors& set = pt->getSet() ;
  int nElem = set.getElem();

  if(1 < nElem && pt->usingColor()) {
    moi2.calcAndSetExtendId(objCount.getIdCount(oEDIT));
    idColor = moi2.getFirstExtendId();
    }
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Width()),
//            REV__X(Prop->Rect.Height()),

            style1,
            style2,

            idMin,
            idMax,
            idCurr,
            idColor
            );

  writeStringChkUnicode(pfCrypt, buff);

  dataPrf tmp(Prop->perif, Prop->addr, Prop->typeVal, Prop->normaliz);

  savePrf(pfCrypt, buff, idCurr, tmp);
  savePrf(pfCrypt, buff, idMax, pt->DataPrf[0]);
  savePrf(pfCrypt, buff, idMin, pt->DataPrf[1]);
  if(idColor)
    savePrf(pfCrypt, buff, idColor, pt->DataPrf[2]);

  if(!idColor)
    saveColor(pfCrypt, buff, moi.getFirstExtendId(), set[0]);

  else {
    int idInit = objCount.getGlobalIdCount(nElem + 1);
    wsprintf(buff, _T("%d,%d,%d\r\n"), moi.getFirstExtendId(), nElem, idInit);
    writeStringChkUnicode(pfCrypt, buff);

    for(int i = 0; i < nElem; ++i)
      saveColor(pfCrypt, buff, idInit + i, set[i]);
    }
//  if(Prop->visPerif)
    saveOnlyVisVar(moi.getSecondExtendId(), pfCrypt);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjDiam::load(uint id, setOfString& set)
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;
  int style1 = 0;
  int style2 = 0;
  int idMin = 0;
  int idMax = 0;
  int idCurr = 0;
  int idColor = 0;


  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style1, &style2,
                    &idMin, &idMax, &idCurr,
                    &idColor
                    );

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  setStyle1(style1);
  setStyle2(style2);

  dataPrf tmp;
  loadPrf(idCurr, set, tmp);

  Prop->perif = tmp.perif;
  Prop->addr = tmp.addr;
  Prop->typeVal = tmp.typeVal;
  Prop->normaliz = tmp.normaliz;

  loadPrf(idMax, set, pt->DataPrf[0]);
  loadPrf(idMin, set, pt->DataPrf[1]);

  setOfDiamColors& setColor = pt->getSet() ;
  pt->setUseColor(toBool(idColor));

  manageObjId moi(id, ID_INIT_VAR_DIAM);
  if(!idColor)
    loadColor(moi.getFirstExtendId(), set, setColor[0]);

  else {
    loadPrf(idColor, set, pt->DataPrf[2]);
    int nElem = 0;
    int idInit = 0;
    p = set.getString(moi.getFirstExtendId());
    if(p) {
      _stscanf_s(p, _T("%d,%d"), &nElem, &idInit);
      for(int i = 0; i < nElem; ++i)
        loadColor(idInit + i, set, setColor[i]);
      }
    }
  loadOnlyVisVar(set, moi.getSecondExtendId());

  return true;
}
//-----------------------------------------------------------
void svmObjDiam::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyDiam* pb = dynamic_cast<PropertyDiam*>(Prop);
  LPCTSTR info[] = { _T("Valore max"), _T("Valore min"), _T("Colori") };
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const dataPrf& vI =  pb->DataPrf[i];
    if(!vI.perif)
      continue;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, info[i]);
    ia4->prph = vI.perif;
    ia4->addr = vI.addr;
    ia4->type = vI.typeVal;
    set.add(ia4);
    }
}
