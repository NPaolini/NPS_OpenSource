//-------------------- svmObjTrend.cpp -----------------------
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
#include "svmObjTrend.h"
#include "svmDefObj.h"
#include "svmPropertyTrend.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyTrend staticPT;
//-----------------------------------------------------------
svmObjTrend::svmObjTrend(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oTREND, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjTrend::svmObjTrend(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oTREND, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjTrend::init()
{
  Prop = allocProperty();
  PropertyTrend* po = dynamic_cast<PropertyTrend*>(Prop);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjTrend::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
      PropertyTrend* p = (PropertyTrend*)Prop;
      for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
        if(isValidPrph4OffsetAddr(p->DataPrf[i].perif, ior))
          if(isValid4OffsetAddr(p->DataPrf[i].perif, p->DataPrf[i].addr, exclude))
            p->DataPrf[i].addr += ior.offs;
        }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    PropertyTrend* p = (PropertyTrend*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom == p->DataPrf[i].perif)
        p->DataPrf[i].perif = ior.prphTo;
      }
    }
  if(isSetBitf(eoSubstVar, ior.flag)) {
    PropertyTrend* p = (PropertyTrend*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom2 == p->DataPrf[i].perif && ior.addrFrom == p->DataPrf[i].addr) {
        p->DataPrf[i].perif = ior.prphTo2;
        p->DataPrf[i].addr = ior.addrTo;
        }
      }
    }
}
//----------------------------------------------------------------------------
#define DEF_INFLATE 4
//----------------------------------------------------------------------------
void svmObjTrend::drawFilled(HDC hdc)
{
  if(Property::NO == Prop->style)
    return;

  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(Prop);

  const oTrendColors& c = pt->getColors();

  PRect rect = getRect();
  PPanel(rect, c.baseColor[oTrendColors::cBkg], (PPanel::bStyle)Prop->style).paint(hdc);

}
//----------------------------------------------------------------------------
#define USE_COLOR_DW(dw,c) (!(dw & (1 << oTrendColors::c)))
//----------------------------------------------------------------------------
#define OFFY 5
//----------------------------------------------------------------------------
void svmObjTrend::DrawObject(HDC hdc)
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(Prop);
  if(pt) {
#if 1
    drawFilled(hdc);
    PRect r = getRect();
    r.Inflate(-1, -1);
    paintSampleTrend(hdc, r, pt->getColors(), pt->type2, pt->type1, true);
#else
    drawFilled(hdc);
    drawAxis(hdc);
    drawLines(hdc);
#endif
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjTrend::getTitle() const
{
  return smartPointerConstString(_T("oTrend"), 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjTrend::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return Prop->Rect;
}
//-----------------------------------------------------------
svmObject* svmObjTrend::makeClone()
{
  svmObjTrend* obj = new svmObjTrend(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjTrend::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjTrend::allocProperty() { return new PropertyTrend; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjTrend::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyTrend*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogTrend(this, Prop, getParent()).modal()) {
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
void saveLineColor(P_File& pf, LPTSTR buff, int id, const oTrendColors& c)
{
  const PVect<colorLine>& lines = c.linesColor;
  uint nElem = lines.getElem();
  wsprintf(buff, _T("%d"), id);
  LPTSTR p = buff + _tcslen(buff);
  for(uint i = 0; i < nElem; ++i) {
    const colorLine& cl = lines[i];
    wsprintf(p, _T(",%d,%d,%d,%d"), cl.tickness, GetRValue(cl.Color), GetGValue(cl.Color), GetBValue(cl.Color));
    p += _tcslen(p);
    }
  wsprintf(p, _T("\r\n"));
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadLineColor(uint id, setOfString& set, oTrendColors& c)
{
  LPCTSTR p = set.getString(id);
  PVect<colorLine>& lines = c.linesColor;
  uint ix = 0;
  while(p) {
    int cols[3];
    colorLine& cl = lines[ix];
    _stscanf_s(p, _T("%d,%d,%d,%d"), &cl.tickness, &cols[0], &cols[1], &cols[2]);
    cl.Color = RGB(cols[0], cols[1], cols[2]);
    ++ix;
    p = findNextParamTrim(p, 4);
    }
}
//-----------------------------------------------------------
/*
void saveColor(P_File& pf, LPTSTR buff, int id, const oTrendColors& c)
{
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            GetRValue(c.Color[oTrendColors::cBkg]) * mulBkg,
            GetGValue(c.Color[oTrendColors::cBkg]) * mulBkg,
            GetBValue(c.Color[oTrendColors::cBkg]) * mulBkg,

            GetRValue(c.Color[oTrendColors::cGrid]) * mulGrid,
            GetGValue(c.Color[oTrendColors::cGrid]) * mulGrid,
            GetBValue(c.Color[oTrendColors::cGrid]) * mulGrid,

            GetRValue(c.Color[oTrendColors::cAxe]) * mulAxe,
            GetGValue(c.Color[oTrendColors::cAxe]) * mulAxe,
            GetBValue(c.Color[oTrendColors::cAxe]) * mulAxe,

            GetRValue(c.Color[oTrendColors::cLineShow1]),
            GetGValue(c.Color[oTrendColors::cLineShow1]),
            GetBValue(c.Color[oTrendColors::cLineShow1]),

            GetRValue(c.Color[oTrendColors::cLineShow2]),
            GetGValue(c.Color[oTrendColors::cLineShow2]),
            GetBValue(c.Color[oTrendColors::cLineShow2])
            );
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadColor(uint id, setOfString& set, oTrendColors& c)
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
*/
//-----------------------------------------------------------
int svmObjTrend::getStyle()
{
  int type = 0;
  switch(Prop->style) {
    case Property::DN:
    case Property::DN_FILL:
      type = 0;
      break;
    case Property::UP:
    case Property::UP_FILL:
      type = 1;
      break;
    case Property::BORDER:
    case Property::BORDER_FILL:
      type = 2;
      break;
    case Property::FILL:
    case Property::NO:
      type = 3;
      break;
    }
  return type;
}
//-----------------------------------------------------------
void svmObjTrend::setStyle(int style)
{
  switch(style) {
    case 0:
      Prop->style = Property::DN;
      break;
    case 1:
      Prop->style = Property::UP;
      break;
    case 2:
      Prop->style = Property::BORDER;
      break;
    case 3:
    default:
      Prop->style = Property::NO;
      break;
    }
}
//-----------------------------------------------------------
#define R_G_B(a) GetRValue(a), GetGValue(a), GetBValue(a)
#define R_G_B__(a) R_G_B(bc[a])
//-----------------------------------------------------------
#define AUTO_STEP_BIT (1 << 15)
//-----------------------------------------------------------
#define AUTO_ROWS_BIT (1 << 16)
//-----------------------------------------------------------
static bool extractAutoStep(int& v)
{
  bool set = toBool(v & AUTO_STEP_BIT);
  v &= ~AUTO_STEP_BIT;
  return set;
}
//-----------------------------------------------------------
static bool extractAutoRows(int& v)
{
  bool set = toBool(v & AUTO_ROWS_BIT);
  v &= ~AUTO_ROWS_BIT;
  return set;
}
//-----------------------------------------------------------
static UINT makeAutoStep(UINT v, bool bit)
{
  if(!bit)
    return v;
  return v | AUTO_STEP_BIT;
}
//-----------------------------------------------------------
static UINT makeAutoRows(UINT v, bool bit)
{
  if(!bit)
    return v;
  return v | AUTO_ROWS_BIT;
}
//-----------------------------------------------------------
bool svmObjTrend::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_TREND);
  uint id = moi.calcAndSetExtendId(order);
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = makeAutoStep(getStyle(), pt->autoStepTen);
  style = makeAutoRows(style, pt->autoRows);

  svmObjCount& objCount = getObjCount();

  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idHideShow = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idActive = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idCtrl = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idDateTime = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));

  const oTrendColors& cl = pt->getColors();

//  int idColor = objCount.getGlobalIdCount();

  const COLORREF* bc = cl.baseColor;
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__Y(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__Y(Prop->Rect.Height()),
            style, R_G_B__(oTrendColors::cBkg),
            cl.gridType, R_G_B__(oTrendColors::cGrid),
            R_G_B(pt->foreground),
            R_G_B__(oTrendColors::cLineShow),
            pt->type1, // numero di colonne
            pt->type2, // numero di righe
            pt->idFont + ID_INIT_FONT,
            idHideShow, idActive,
            idCtrl,
            idDateTime,
            pt->trendName
            );

  writeStringChkUnicode(pfCrypt, buff);

  int idColor = moi.getThirdExtendId();
  saveLineColor(pfCrypt, buff, idColor, cl);
  Prop->typeVal = 5;
  if(pt->trendMinMaxFile) {
    int idFileMinMax = objCount.getGlobalIdCount();
    wsprintf(buff, _T("%d,%s\r\n"), idFileMinMax, pt->trendMinMaxFile);
    writeStringChkUnicode(pfCrypt, buff);
    Prop->perif = 0;
    Prop->addr = idFileMinMax;
    }
  saveVar(pfCrypt, id);

  savePrf(pfCrypt, buff, idHideShow, pt->DataPrf[0]);
  savePrf(pfCrypt, buff, idActive, pt->DataPrf[1]);
  savePrf(pfCrypt, buff, idCtrl, pt->DataPrf[2]);
  pt->DataPrf[3].typeVal = 9;
  savePrf(pfCrypt, buff, idDateTime, pt->DataPrf[3]);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
#define aR_G_B(a) &a[0], &a[1], &a[2]
#define setRGB(a) RGB(a[0], a[1], a[2])
//-----------------------------------------------------------
bool svmObjTrend::load(uint id, setOfString& set)
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;
  int style = 0;
  int idHideShow = 0;
  int idActive = 0;
  int idCtrl = 0;
  int idDateTime = 0;
  int c_bkg[3] = { 0 };
  int c_grid[3] = { 0 };
  int c_text[3] = { 0 };
  int c_line[3] = { 0 };

  oTrendColors& setColor = pt->getColors();
  LPCTSTR p = set.getString(id);
  int skip = 0;
  if(p)
    skip = _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style, aR_G_B(c_bkg),
                    &setColor.gridType, aR_G_B(c_grid),
                    aR_G_B(c_text),
                    aR_G_B(c_line),
                    &pt->type1,
                    &pt->type2,
                    &pt->idFont,
                    &idHideShow, &idActive,
                    &idCtrl,
                    &idDateTime
                    );
  p = findNextParamTrim(p, skip);
  if(p)
    pt->trendName = str_newdup(p);
  pt->idFont -= ID_INIT_FONT;
  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  manageObjId moi(id, ID_INIT_VAR_TREND);
  int idColor = moi.getThirdExtendId();
  loadLineColor(idColor, set, setColor);

  setColor.baseColor[oTrendColors::cBkg] = setRGB(c_bkg);
  setColor.baseColor[oTrendColors::cGrid] = setRGB(c_grid);
  setColor.baseColor[oTrendColors::cLineShow] = setRGB(c_line);

  pt->foreground = setRGB(c_text);
  pt->background = setColor.baseColor[oTrendColors::cBkg];

  pt->autoStepTen = extractAutoStep(style);
  pt->autoRows = extractAutoRows(style);
  setStyle(style);

  loadVar(id, set);
  if(!Prop->perif) {
    p = set.getString(Prop->addr);
    if(p) {
      pt->trendMinMaxFile = str_newdup(p);
      Prop->addr = 0;
      }
    }
  loadPrf(idHideShow, set, pt->DataPrf[0]);
  loadPrf(idActive, set, pt->DataPrf[1]);
  loadPrf(idCtrl, set, pt->DataPrf[2]);
  loadPrf(idDateTime, set, pt->DataPrf[3]);

  return true;
}
//-----------------------------------------------------------
void svmObjTrend::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyTrend* pb = dynamic_cast<PropertyTrend*>(Prop);
  LPCTSTR info[] = { _T("bit Show"), _T("bit Active"), _T("bit Control") };
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
  LPCTSTR info2[] = { _T("Data"), _T("Ora"), _T("Intervallo") };
  const dataPrf& vI =  pb->DataPrf[SIZE_A(info)];
  for(uint i = 0; i < SIZE_A(info2); ++i) {
    if(!vI.perif)
      continue;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, info2[i]);
    ia4->prph = vI.perif;
    ia4->addr = vI.addr + i * 2;
    ia4->type = vI.typeVal;
    set.add(ia4);
    }
}

