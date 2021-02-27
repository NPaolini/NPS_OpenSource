//-------------------- svmObjXScope.cpp -----------------------
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
#include "svmObjXScope.h"
#include "svmDefObj.h"
#include "svmPropertyXScope.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyXScope staticPT;
//-----------------------------------------------------------
svmObjXScope::svmObjXScope(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oXSCOPE, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjXScope::svmObjXScope(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oXSCOPE, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjXScope::init()
{
  Prop = allocProperty();
  PropertyXScope* po = dynamic_cast<PropertyXScope*>(Prop);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjXScope::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
      PropertyXScope* p = (PropertyXScope*)Prop;
      for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
        if(isValidPrph4OffsetAddr(p->DataPrf[i].perif, ior))
          if(isValid4OffsetAddr(p->DataPrf[i].perif, p->DataPrf[i].addr, exclude))
            p->DataPrf[i].addr += ior.offs;
        }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    PropertyXScope* p = (PropertyXScope*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom == p->DataPrf[i].perif)
        p->DataPrf[i].perif = ior.prphTo;
      }
    }
  if(isSetBitf(eoSubstVar, ior.flag)) {
    PropertyXScope* p = (PropertyXScope*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom2 == p->DataPrf[i].perif && ior.addrFrom == p->DataPrf[i].addr) {
        p->DataPrf[i].perif = ior.prphTo2;
        p->DataPrf[i].addr = ior.addrTo;
        }
      }
    }
}
//----------------------------------------------------------------------------
Property* svmObjXScope::allocProperty() { return new PropertyXScope; }
//----------------------------------------------------------------------------
#define DEF_INFLATE 4
//----------------------------------------------------------------------------
void svmObjXScope::drawFilled(HDC hdc)
{
  if(Property::NO == Prop->style)
    return;

  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);

  const xScopeColors& c = pt->getColors();

  PRect rect = getRect();
  PPanel(rect, c.Color[xScopeColors::cBkg], (PPanel::bStyle)Prop->style).paint(hdc);

}
//----------------------------------------------------------------------------
#define MID_WIDTH_ARROW 3
#define HEIGHT_ARROW 8
//----------------------------------------------------------------------------
void arrowUp(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - MID_WIDTH_ARROW, y + HEIGHT_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x + MID_WIDTH_ARROW, y + HEIGHT_ARROW);
  LineTo(hdc, x - MID_WIDTH_ARROW, y + HEIGHT_ARROW);
}
//----------------------------------------------------------------------------
void arrowRight(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x - HEIGHT_ARROW, y + MID_WIDTH_ARROW);
  LineTo(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW);
}
//----------------------------------------------------------------------------
#define USE_COLOR_DW(dw,c) (!(dw & (1 << xScopeColors::c)))
//----------------------------------------------------------------------------
#define OFFY 5
//----------------------------------------------------------------------------
void svmObjXScope::drawAxis(HDC hdc)
{
  PRect r = getRect();
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);

  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
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
void draw_Lines(HDC hdc, PRect r, const xScopeColors& c)
{
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);

  HPEN Pen1 = CreatePen(PS_SOLID, 1, c.Color[xScopeColors::cLineShow1]);
  HPEN Pen2 = CreatePen(PS_SOLID, 1, c.Color[xScopeColors::cLineShow2]);
  HGDIOBJ oldPen = SelectObject(hdc, Pen1);

  int width = r.Width() / 4;
  srand(width);
  MoveToEx(hdc, r.left, (r.bottom + r.top) / 2, 0);
  for(int i = 4, x = r.left + 4; i < width; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, Pen2);
  int width2 = r.Width() / 2;
  for(int i = width, x = width * 2 + r.left; i < width2; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, oldPen);
  DeleteObject(Pen1);
  DeleteObject(Pen2);
}
//-----------------------------------------------------------
void svmObjXScope::drawLines(HDC hdc)
{
#if 1
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
  const xScopeColors& c = pt->getColors();
  draw_Lines(hdc, getRect(), c);
#else
  PRect r = getRect();
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);

  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
  const xScopeColors& c = pt->getColors();

  HPEN Pen1 = CreatePen(PS_SOLID, 1, c.Color[xScopeColors::cLineShow1]);
  HPEN Pen2 = CreatePen(PS_SOLID, 1, c.Color[xScopeColors::cLineShow2]);
  HGDIOBJ oldPen = SelectObject(hdc, Pen1);

  int width = r.Width() / 4;
  srand(width);
  MoveToEx(hdc, r.left, (r.bottom + r.top) / 2, 0);
  for(int i = 4, x = r.left + 4; i < width; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, Pen2);
  int width2 = r.Width() / 2;
  for(int i = width, x = width * 2 + r.left; i < width2; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, oldPen);
  DeleteObject(Pen1);
  DeleteObject(Pen2);
#endif
}
//-----------------------------------------------------------
void svmObjXScope::DrawObject(HDC hdc)
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
  if(pt) {
    drawFilled(hdc);
    drawAxis(hdc);
    drawLines(hdc);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjXScope::getTitle() const
{
  return smartPointerConstString(_T("xScope"), 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjXScope::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return Prop->Rect;
}
//-----------------------------------------------------------
svmObject* svmObjXScope::makeClone()
{
  svmObjXScope* obj = new svmObjXScope(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjXScope::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjXScope::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyXScope*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogXScope(this, Prop, getParent()).modal()) {
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
void saveColor(P_File& pf, LPTSTR buff, int id, const xScopeColors& c)
{
  int mulBkg =  (c.notUseBit & 1) ? -1 : 1;
  int mulGrid = (c.notUseBit & 2) ? -1 : 1;
  int mulAxe =  (c.notUseBit & 4) ? -1 : 1;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            GetRValue(c.Color[xScopeColors::cBkg]) * mulBkg,
            GetGValue(c.Color[xScopeColors::cBkg]) * mulBkg,
            GetBValue(c.Color[xScopeColors::cBkg]) * mulBkg,

            GetRValue(c.Color[xScopeColors::cGrid]) * mulGrid,
            GetGValue(c.Color[xScopeColors::cGrid]) * mulGrid,
            GetBValue(c.Color[xScopeColors::cGrid]) * mulGrid,

            GetRValue(c.Color[xScopeColors::cAxe]) * mulAxe,
            GetGValue(c.Color[xScopeColors::cAxe]) * mulAxe,
            GetBValue(c.Color[xScopeColors::cAxe]) * mulAxe,

            GetRValue(c.Color[xScopeColors::cLineShow1]),
            GetGValue(c.Color[xScopeColors::cLineShow1]),
            GetBValue(c.Color[xScopeColors::cLineShow1]),

            GetRValue(c.Color[xScopeColors::cLineShow2]),
            GetGValue(c.Color[xScopeColors::cLineShow2]),
            GetBValue(c.Color[xScopeColors::cLineShow2])
            );
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadColor(uint id, setOfString& set, xScopeColors& c)
{
  LPCTSTR p = set.getString(id);
  int cols[5][3];
  if(p) {
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            &cols[0][0], &cols[0][1], &cols[0][2],
            &cols[1][0], &cols[1][1], &cols[1][2],
            &cols[2][0], &cols[2][1], &cols[2][2],
            &cols[3][0], &cols[3][1], &cols[3][2],
            &cols[4][0], &cols[4][1], &cols[4][2]
            );
    for(int i = 0; i < 5; ++i) {
      bool notUse = false;
      for(uint j = 0; j < 3; ++j) {
        if(cols[i][j] < 0) {
          notUse = true;
          cols[i][j] = -cols[i][j];
          }
        }
      c.Color[i] = RGB(cols[i][0], cols[i][1], cols[i][2]);
      if(notUse)
        c.notUseBit |= 1 << i;
      }
    }
}
//-----------------------------------------------------------
int svmObjXScope::getStyle()
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
void svmObjXScope::setStyle(int style)
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
bool svmObjXScope::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_SCOPE);
  uint id = moi.calcAndSetExtendId(order);
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = getStyle();

  svmObjCount& objCount = getObjCount();

  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idNumBlock = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMaxNumData = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMin = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMax = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idEnble = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idInitX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idCurrX = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));

  const xScopeColors& set = pt->getColors();

  int idColor = objCount.getGlobalIdCount();

  int typeX = pt->useVariableX ? 1 | (pt->relativeBlock << 1) | (pt->zeroOffset << 2) : 0;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__Y(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__Y(Prop->Rect.Height()),
            style, idColor,
            pt->type1, // numero di righe
            idNumBlock, idMaxNumData,
            idMin,
            idMax,
            idEnble,
            pt->type2,  // addrReadyPos
            pt->uniqueData,
            typeX, idInitX, idCurrX
            );

  writeStringChkUnicode(pfCrypt, buff);

  saveColor(pfCrypt, buff, idColor, set);
  saveVar(pfCrypt, id);

  savePrf(pfCrypt, buff, idEnble, pt->DataPrf[0]);
  if(typeX & 1) {
    savePrf(pfCrypt, buff, idInitX, pt->DataPrf[1]);
    savePrf(pfCrypt, buff, idCurrX, pt->DataPrf[2]);
    }
  savePrf(pfCrypt, buff, idNumBlock, pt->DataPrf[3]);
  savePrf(pfCrypt, buff, idMaxNumData, pt->DataPrf[4]);
  savePrf(pfCrypt, buff, idMin, pt->DataPrf[5]);
  savePrf(pfCrypt, buff, idMax, pt->DataPrf[6]);

  return true;
}
//-----------------------------------------------------------
bool svmObjXScope::load(uint id, setOfString& set)
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;
  int style = 0;
  int idNumBlock = 0;
  int idMaxNumData = 0;
  int idMin = 0;
  int idMax = 0;
  int idEnble = 0;
  int idColor = 0;
  int uniqueData = 0;
  int typeX = 0;
  int idInitX = 0;
  int idCurrX = 0;


  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style, &idColor, &pt->type1,
                    &idNumBlock, &idMaxNumData,
                    &idMin, &idMax, &idEnble,
                    &pt->type2,
                    &uniqueData,
                    &typeX, &idInitX, &idCurrX
                    );
  pt->uniqueData = toBool(uniqueData);
  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  setStyle(style);

  loadVar(id, set);
  pt->useVariableX = toBool(typeX & 1);
  pt->relativeBlock = toBool(typeX & 2);
  pt->zeroOffset = toBool(typeX & 4);

  loadPrf(idEnble, set, pt->DataPrf[0]);
  if(pt->useVariableX) {
    loadPrf(idInitX, set, pt->DataPrf[1]);
    loadPrf(idCurrX, set, pt->DataPrf[2]);
    }
  loadPrf(idNumBlock, set, pt->DataPrf[3]);
  loadPrf(idMaxNumData, set, pt->DataPrf[4]);
  loadPrf(idMin, set, pt->DataPrf[5]);
  loadPrf(idMax, set, pt->DataPrf[6]);

  xScopeColors& setColor = pt->getColors();

  loadColor(idColor, set, setColor);

  return true;
}
//-----------------------------------------------------------
void svmObjXScope::addInfoAd4Other(manageInfoAd4& set)
{
    // idEnbleRead,idInitX,idCurrX,idNumBlock,idMaxNumData,idMinVal,idMaxVal
  PropertyXScope* pb = dynamic_cast<PropertyXScope*>(Prop);
  LPCTSTR info[] = { _T("Abilita lettura"), _T("Inizio dati X"), _T("Posizione corrente X"),
        _T("Num Blocchi"), _T("Max Num Dati"), _T("Valore min"), _T("Valore max") };
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
