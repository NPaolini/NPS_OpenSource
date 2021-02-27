//-------------------- svmObjPlotXY.cpp -----------------------
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
#include "svmObjPlotXY.h"
#include "svmDefObj.h"
#include "svmPropertyPlotXY.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyPlotXY staticPT;
//-----------------------------------------------------------
svmObjPlotXY::svmObjPlotXY(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oPLOT_XY, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjPlotXY::svmObjPlotXY(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oPLOT_XY, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjPlotXY::init()
{
  Prop = allocProperty();
  PropertyPlotXY* po = dynamic_cast<PropertyPlotXY*>(Prop);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjPlotXY::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
      PropertyPlotXY* p = (PropertyPlotXY*)Prop;
      for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
        if(isValidPrph4OffsetAddr(p->DataPrf[i].perif, ior))
          if(isValid4OffsetAddr(p->DataPrf[i].perif, p->DataPrf[i].addr, exclude))
            p->DataPrf[i].addr += ior.offs;
        }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    PropertyPlotXY* p = (PropertyPlotXY*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom == p->DataPrf[i].perif)
        p->DataPrf[i].perif = ior.prphTo;
      }
    }
  if(isSetBitf(eoSubstVar, ior.flag)) {
    PropertyPlotXY* p = (PropertyPlotXY*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom2 == p->DataPrf[i].perif && ior.addrFrom == p->DataPrf[i].addr) {
        p->DataPrf[i].perif = ior.prphTo2;
        p->DataPrf[i].addr = ior.addrTo;
        }
      }
    }
}
//----------------------------------------------------------------------------
Property* svmObjPlotXY::allocProperty() { return new PropertyPlotXY; }
//----------------------------------------------------------------------------
#define DEF_INFLATE 4
//----------------------------------------------------------------------------
void svmObjPlotXY::drawFilled(HDC hdc)
{
  if(Property::NO == Prop->style)
    return;

  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);

  const xScopeColors& c = pt->getColors();

  PRect rect = getRect();
  PPanel(rect, c.Color[xScopeColors::cBkg], (PPanel::bStyle)Prop->style).paint(hdc);

}
//----------------------------------------------------------------------------
#define MID_WIDTH_ARROW 3
#define HEIGHT_ARROW 8
//----------------------------------------------------------------------------
extern void arrowUp(HDC hdc, int x, int y);
extern void arrowRight(HDC hdc, int x, int y);
//----------------------------------------------------------------------------
#define USE_COLOR_DW(dw,c) (!(dw & (1 << xScopeColors::c)))
//----------------------------------------------------------------------------
#define OFFY 5
//----------------------------------------------------------------------------
void svmObjPlotXY::drawAxis(HDC hdc)
{
  PRect r = getRect();
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);

  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);
  const xScopeColors& c = pt->getColors();
  COLORREF oldColor;

  DWORD bitUseColor =  pt->getColors().notUseBit;
  if(USE_COLOR_DW(bitUseColor, cBkg))
    oldColor = SetBkColor(hdc, c.Color[xScopeColors::cBkg]);

  if(USE_COLOR_DW(bitUseColor, cGrid)) {
    HPEN Pen2 = CreatePen(PS_DOT, 1, c.Color[xScopeColors::cGrid]);
    HGDIOBJ oldPen = SelectObject(hdc, Pen2);

    int numCol = 10;
    double stepx = r.Width() / (double)numCol;
    double j = stepx;
    for(int i = 0; i < numCol; ++i, j += stepx) {
      long x = (long)j + r.left;
      MoveToEx(hdc, x, r.top, 0);
      LineTo(hdc, x, r.bottom);
      }

    if(pt->type1) {
      double stepy = r.Height() / (double)pt->type1;
      j = stepy;
      for(int i = 0; i < (int)pt->type1 - 1; ++i, j += stepy) {
        long y = (LONG)j + r.top;
        MoveToEx(hdc, r.left, y, 0);
        LineTo(hdc, r.right, y);
        }
      }
    SelectObject(hdc, oldPen);
    DeleteObject(Pen2);
    }

  if(USE_COLOR_DW(bitUseColor, cAxe)) {
    HPEN Pen = CreatePen(PS_SOLID, 2, c.Color[xScopeColors::cAxe]);

    HGDIOBJ oldPen = SelectObject(hdc, Pen);

    int y =  r.bottom;
    MoveToEx(hdc, r.left, y, 0);
    LineTo(hdc, r.right, y);

    MoveToEx(hdc, r.left, r.top, 0);
    LineTo(hdc, r.left, r.bottom);

    arrowUp(hdc, r.left, r.top);
    arrowRight(hdc, r.right, y);

    SelectObject(hdc, oldPen);
    DeleteObject(Pen);
    }
  if(USE_COLOR_DW(bitUseColor, cBkg))
    SetBkColor(hdc, oldColor);
}
//-----------------------------------------------------------
extern void draw_Lines(HDC hdc, PRect r, const xScopeColors& c);
//-----------------------------------------------------------
void svmObjPlotXY::drawLines(HDC hdc)
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);
  const xScopeColors& c = pt->getColors();
  draw_Lines(hdc, getRect(), c);
}
//-----------------------------------------------------------
void svmObjPlotXY::DrawObject(HDC hdc)
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);
  if(pt) {
    drawFilled(hdc);
    drawAxis(hdc);
    drawLines(hdc);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjPlotXY::getTitle() const
{
  return smartPointerConstString(_T("Plot XY"), 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjPlotXY::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return Prop->Rect;
}
//-----------------------------------------------------------
svmObject* svmObjPlotXY::makeClone()
{
  svmObjPlotXY* obj = new svmObjPlotXY(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjPlotXY::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjPlotXY::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyPlotXY*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogPlotXY(this, Prop, getParent()).modal()) {
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
extern void saveColor(P_File& pf, LPTSTR buff, int id, const xScopeColors& c);
extern void loadColor(uint id, setOfString& set, xScopeColors& c);
//-----------------------------------------------------------
int svmObjPlotXY::getStyle()
{
  int type;
  switch(Prop->style) {
    case Property::UP:
      type = 0;
      break;
    case Property::DN:
      type = 1;
      break;
    case Property::UP_FILL:
      type = 2;
      break;
    case Property::DN_FILL:
      type = 3;
      break;
    case Property::BORDER:
      type = 4;
      break;
    case Property::BORDER_FILL:
      type = 5;
      break;
    case Property::FILL:
      type = 6;
      break;
    default:
    case Property::NO:
      type = 7;
      break;
    }
  return type;
}
//-----------------------------------------------------------
void svmObjPlotXY::setStyle(int style)
{
  switch(style) {
    case 0:
      Prop->style = Property::UP;
      break;
    case 1:
      Prop->style = Property::DN;
      break;
    case 2:
      Prop->style = Property::UP_FILL;
      break;
    case 3:
      Prop->style = Property::DN_FILL;
      break;
    case 4:
      Prop->style = Property::BORDER;
      break;
    case 5:
      Prop->style = Property::BORDER_FILL;
      break;
    case 6:
      Prop->style = Property::FILL;
      break;
    default:
      Prop->style = Property::NO;
      break;
    }
}
//-----------------------------------------------------------
bool svmObjPlotXY::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_PLOT_XY);
  uint id = moi.calcAndSetExtendId(order);
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = getStyle();

  svmObjCount& objCount = getObjCount();

  manageObjId moi2(0, ID_INIT_VAR_EDI);
//  int idNumBlock = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMaxNumData = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMinY = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMaxY = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idEnble = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idInitX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMinX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMaxX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idCurrX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));

  const xScopeColors& set = pt->getColors();

  int idColor = objCount.getGlobalIdCount();

  int typeX = (pt->relativeBlock_Y << 0) | (pt->relativeBlock_X << 1);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__Y(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__Y(Prop->Rect.Height()),
            style, idColor,
            pt->type1, // numero di righe
            pt->type2, // numero di colonne
            idMaxNumData,
            idMinY, idMaxY,
            idInitX, idMinX, idMaxX,
            idCurrX,
            idEnble,
            typeX
            );

  writeStringChkUnicode(pfCrypt, buff);

  saveColor(pfCrypt, buff, idColor, set);
  saveVar(pfCrypt, id);

  savePrf(pfCrypt, buff, idMinY, pt->DataPrf[0]);
  savePrf(pfCrypt, buff, idMaxY, pt->DataPrf[1]);
  savePrf(pfCrypt, buff, idMinX, pt->DataPrf[2]);
  savePrf(pfCrypt, buff, idMaxX, pt->DataPrf[3]);
  savePrf(pfCrypt, buff, idMaxNumData, pt->DataPrf[4]);
  savePrf(pfCrypt, buff, idInitX, pt->DataPrf[5]);
  savePrf(pfCrypt, buff, idCurrX, pt->DataPrf[6]);
  savePrf(pfCrypt, buff, idEnble, pt->DataPrf[7]);
//  savePrf(pfCrypt, buff, idNumBlock, pt->DataPrf[8]);

  return true;
}
//-----------------------------------------------------------
bool svmObjPlotXY::load(uint id, setOfString& set)
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;
  int style = 0;
//  int idNumBlock = 0;
  int idMaxNumData = 0;
  int idMinY = 0;
  int idMaxY = 0;
  int idMinX = 0;
  int idMaxX = 0;
  int idEnble = 0;
  int idColor = 0;
  int typeX = 0;
  int idInitX = 0;
  int idCurrX = 0;


  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style, &idColor, &pt->type1, &pt->type2,
                    &idMaxNumData,
                    &idMinY, &idMaxY,
                    &idInitX, &idMinX, &idMaxX,
                    &idCurrX,
                    &idEnble,
                    &typeX
                    );

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  setStyle(style);

  loadVar(id, set);
  pt->relativeBlock_Y = toBool(typeX & 1);
  pt->relativeBlock_X = toBool(typeX & 2);

  loadPrf(idMinY, set, pt->DataPrf[0]);
  loadPrf(idMaxY, set, pt->DataPrf[1]);
  loadPrf(idMinX, set, pt->DataPrf[2]);
  loadPrf(idMaxX, set, pt->DataPrf[3]);
  loadPrf(idMaxNumData, set, pt->DataPrf[4]);
  loadPrf(idInitX, set, pt->DataPrf[5]);
  loadPrf(idCurrX, set, pt->DataPrf[6]);
  loadPrf(idEnble, set, pt->DataPrf[7]);
//  loadPrf(idNumBlock, set, pt->DataPrf[8]);

  xScopeColors& setColor = pt->getColors();

  loadColor(idColor, set, setColor);

  return true;
}
//-----------------------------------------------------------
void svmObjPlotXY::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyPlotXY* pb = dynamic_cast<PropertyPlotXY*>(Prop);
  LPCTSTR info[] = { _T("Valore max Y"),
          _T("Inizio dati X"), _T("Valore min X"), _T("Valore max X"),
          _T("Max Num Dati"), _T("Valore min Y"), _T("Posizione corrente"),
          _T("Abilita lettura") /*, _T("Num Blocchi") */ };

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
