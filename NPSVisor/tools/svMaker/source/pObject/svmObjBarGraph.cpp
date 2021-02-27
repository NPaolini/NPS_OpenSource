//------------------- svmObjBarGraph.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjBarGraph.h"
#include "svmDefObj.h"
#include "svmPropertyBarGraph.h"
#include "p_File.h"
#include "common.h"
//-----------------------------------------------------------
static PropertyBarGraph staticPT;
//-----------------------------------------------------------
svmObjBarGraph::svmObjBarGraph(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oBARGRAPH, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjBarGraph::svmObjBarGraph(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oBARGRAPH, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjBarGraph::init()
{
  Prop = new PropertyBarGraph;
  Prop->background = GetSysColor(COLOR_3DSHADOW);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjBarGraph::setOffsetAddr(const infoOffsetReplace& ior)
{
  PropertyBarGraph* p = (PropertyBarGraph*)Prop;

  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isValidPrph4OffsetAddr(p->vI[1].perif, ior))
      if(isValid4OffsetAddr(p->vI[1].perif, p->vI[1].addr, isSetBitf(eoExcludePrph1, ior.flag)))
        p->vI[1].addr += ior.offs;
    if(isValidPrph4OffsetAddr(Prop->visPerif, ior))
      if(isValid4OffsetAddr(Prop->visPerif, Prop->visAddr, isSetBitf(eoExcludePrph1, ior.flag)) && isSetBitf(eoAlsoVisibility, ior.flag)) {
        Prop->visAddr += ior.offs;
        Prop->visOffset += ior.bitOffs;
        }
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      if(isValidPrph4OffsetAddr(p->vI[0].perif, ior))
        if(isValid4OffsetAddr(p->vI[0].perif, p->vI[0].addr, isSetBitf(eoExcludePrph1, ior.flag)))
          p->vI[0].addr += ior.offs;
      if(isValidPrph4OffsetAddr(p->vI[2].perif, ior))
        if(isValid4OffsetAddr(p->vI[2].perif, p->vI[2].addr, isSetBitf(eoExcludePrph1, ior.flag)))
          p->vI[2].addr += ior.offs;
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    for(uint i = 0; i < SIZE_A(p->vI); ++i)
      if(ior.prphFrom == p->vI[i].perif)
        p->vI[i].perif = ior.prphTo;
    if(isSetBitf(eoAlsoPrphVisibility, ior.flag) && ior.prphFrom == Prop->visPerif)
      Prop->visPerif = ior.prphTo;
    }

  if(isSetBitf(eoSubstVar, ior.flag)) {
    for(uint i = 0; i < SIZE_A(p->vI); ++i) {
      if(ior.prphFrom2 == p->vI[i].perif && ior.addrFrom == p->vI[i].addr) {
        p->vI[i].perif = ior.prphTo2;
        p->vI[i].addr = ior.addrTo;
        }
      }
    if(ior.prphFrom2 == Prop->visPerif && ior.addrFrom == Prop->visAddr) {
      Prop->visPerif = ior.prphTo2;
      Prop->visAddr = ior.addrTo;
      }
    }
}
//-----------------------------------------------------------
#define BITS_VAR_BASE ((1 << CHANGED_PRF) | (1 << CHANGED_ADDR) | (1 << CHANGED_VALUETYPE) | \
       (1 << CHANGED_NBITS) | (1 << CHANGED_OFFSET) | (1 << CHANGED_NORMALIZ))
//-----------------------------------------------------------
void svmObjBarGraph::setCommonProperty(const Property* prop, DWORD bits, LPCTSTR simpleText)
{
  baseClass::setCommonProperty(prop, bits, simpleText);
  if(BITS_VAR_BASE & bits) {
    PropertyBarGraph* p = (PropertyBarGraph*)Prop;
    if((1 << CHANGED_PRF) & bits)
      p->vI[1].perif = Prop->perif;
    if((1 << CHANGED_ADDR) & bits)
      p->vI[1].addr = Prop->addr;
    if((1 << CHANGED_VALUETYPE) & bits)
      p->vI[1].typeVal =  Prop->typeVal;
    if((1 << CHANGED_NORMALIZ) & bits)
      p->vI[1].normaliz = Prop->normaliz;
    if((1 << CHANGED_NBITS) & bits)
      p->vI[1].nBit = Prop->nBits;
    if((1 << CHANGED_OFFSET) & bits)
      p->vI[1].offs = Prop->offset;
    }
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjBarGraph::dialogProperty(bool onCreate)
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
    if(IDOK == svmDialogBarGraph(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
smartPointerConstString svmObjBarGraph::getTitle() const
{
  bool vert = toBool(Prop->type1 & 1);
  return vert ? smartPointerConstString(0, 0) : smartPointerConstString(_T("BarGraph"), 0);
}
//-----------------------------------------------------------
static void makeStrConst(const PropertyBarGraph::varInfo& vi, LPTSTR buff)
{
  TCHAR t[128];
  makeConstValue(t, SIZE_A(t), vi.addr, PRPH_4_CONST, vi.typeVal);
  wsprintf(buff, _T("[Costante, Valore=%s]\r\n--"), t);
}
//-----------------------------------------------------------
static void makeStrVar(const PropertyBarGraph::varInfo& vi, LPTSTR buff)
{
  wsprintf(buff, _T("Perif=%d, Addr=%d, Type=%s\r\nBits=%d, Offset=%d, Norm=%d\r\n--"),
    vi.perif, vi.addr, getTypeString(vi.typeVal), vi.nBit, vi.offs, vi.normaliz);
}
//-----------------------------------------------------------
void svmObjBarGraph::fillTips(LPTSTR tips, int size)
{
  PropertyBarGraph* p = dynamic_cast<PropertyBarGraph*>(Prop);
  if(!p)
    baseClass::fillTips(tips, size);
  else {
    TCHAR strValue[256];
    TCHAR strMax[256];
    TCHAR strMin[256];
    if(PRPH_4_CONST == p->vI[0].perif)
      makeStrConst(p->vI[0], strValue);
    else
      makeStrVar(p->vI[0], strValue);

    if(PRPH_4_CONST == p->vI[1].perif)
      makeStrConst(p->vI[1], strMax);
    else
      makeStrVar(p->vI[1], strMax);

    if(PRPH_4_CONST == p->vI[2].perif)
      makeStrConst(p->vI[2], strMin);
    else
      makeStrVar(p->vI[2], strMin);

    wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nVal->%s\r\nMax->%s\r\nMin->%s\r\n Move=%s"),
          getObjName(), getId(), zOrder, strValue, strMax, strMin, Prop->MoveInfo.isEnabled() ? _T("ON") : _T("OFF"));
    addTipsVis(tips, size);
    }
}
//-----------------------------------------------------------
void svmObjBarGraph::DrawTitle(HDC hdc)
{
  baseClass::DrawTitle(hdc);
  bool vert = toBool(Prop->type1 & 1);
  if(vert) {
    int oldMode = SetBkMode(hdc, TRANSPARENT);
    int oldColor = SetTextColor(hdc, getProp()->foreground);
    const int heightFont = 14;
    LPCTSTR title = _T("BarGraph");
    HFONT font = D_FONT_ORIENT(R__Y(heightFont), 0, 900, 0, _T("arial"));
    HGDIOBJ oldFont =  SelectObject(hdc, font);
    PRect r(getRect());
//    int t = r.Width();
//    r.right = r.left + r.Height();
//    r.bottom = r.top + t;
    int x = r.left;
//    int x = (r.right + r.left) / 2 - R__Y(heightFont / 2);
    int y = (r.bottom + r.top) / 2;
    uint oldAlign = SetTextAlign(hdc, TA_CENTER | TA_TOP);
    ExtTextOut(hdc, x, y, ETO_CLIPPED, r, title, _tcslen(title), 0);
    SetTextAlign(hdc, oldAlign);

    SelectObject(hdc, oldFont);
    DeleteObject(font);

    SetTextColor(hdc, oldColor);
    SetBkMode(hdc, oldMode);
    }
}
//-----------------------------------------------------------
void svmObjBarGraph::DrawObject(HDC hdc)
{
  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(Prop);
  const setOfValueColor& c = pb->getValueColors();
  COLORREF cBar = RGB(0x0, 0xff, 0x0);
  if(c.getElem())
    cBar = c[0].Color;
  bool horz = !toBool(Prop->type1 & 1);
  bool to_right = !toBool(Prop->type1 & 2);

  PRect r(getRect());

  int perc = 55;
  if(pb->centerZero & 1) {
    PGraph bar(r, 0, PPanel::BORDER, cBar, horz, to_right);
    bar.setPerc(0);
    bar.draw(hdc);

    double v = horz ? r.Width() : r.Height();
    double rg = pb->valueTest[1] - pb->valueTest[0];
    if(rg > 0) {
      int val = (int)(v / rg * pb->valueTest[0]);
      perc = 100 - int((100.0 * pb->valueTest[1]) / rg);
      if(perc >= 100 || perc <= 0)
        perc = 55;
      if(horz) {
        if(to_right)
          r.Offset(val, 0);
        else
          r.Offset(-val, 0);
        }
      else {
        if(to_right)
          r.Offset(0, -val);
        else
          r.Offset(0, val);
        }
      }
    }

//  if(!horz) {
//    int t = r.Width();
//    r.right = r.left + r.Height();
//    r.bottom = r.top + t;
//    }
  if(*pb->iBmp[0].img || *pb->iBmp[1].img) {
    PBitmap* bkg = 0;
    if(*pb->iBmp[0].img) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, pb->iBmp[0].img);
      bkg = new PBitmap(name);
      if(!bkg->isValid()) {
        delete bkg;
        bkg = 0;
        }
      }
    PBitmap* bar = 0;
    if(*pb->iBmp[1].img) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, pb->iBmp[1].img);
      bar = new PBitmap(name);
      if(!bar->isValid()) {
        delete bar;
        bar = 0;
        }
      }

    PGraphBmp::bmpstyle flagbkg = (PGraphBmp::bmpstyle)pb->iBmp[0].getFlag();
    PGraphBmp::bmpstyle flagbar = (PGraphBmp::bmpstyle)pb->iBmp[1].getFlag();
    PGraphBmp barBmp(r, Prop->background, (PPanel::bStyle)Prop->style, cBar, bar, flagbar, bkg, flagbkg, horz, to_right);
    barBmp.setPerc(perc);
    barBmp.draw(hdc);
    delete bar;
    delete bkg;
    }
  else {
    PGraph bar(r, Prop->background, (PPanel::bStyle)Prop->style, cBar, horz, to_right);
    bar.setPerc(perc);
    bar.draw(hdc);
    }
//  staticPT.Rect = getRect();
}
//-----------------------------------------------------------
PRect svmObjBarGraph::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
//  bool vert = toBool(Prop->type1 & 1);
//  if(vert) {
//    int t = r.Width();
//    r.right = r.left + r.Height();
//    r.bottom = r.top + t;
//    }
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjBarGraph::makeClone()
{
  svmObjBarGraph* obj = new svmObjBarGraph(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjBarGraph::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
/*
> 0 = nessun bordo
> 1 = bordato nero
> 2 = stile Up
> 3 = stile Down
> 4 = nessun bordo e nessun ridisegno per lo sfondo (può essere utile per
      accoppiare due bargraph, il primo normale ed il secondo con questo stile)
*/
//-----------------------------------------------------------
namespace styleObjBarGraph {
enum stylePanel {
    noBorder,
    border,
    up,
    dn,
    noEraseBkg
    };
}
//-----------------------------------------------------------
uint svmObjBarGraph::getStyle() const
{
  int style = 0;
  if(Prop->style & Property::FILL) {
    if(Prop->style & Property::BORDER)
      style = styleObjBarGraph::border;
    else if(Prop->style & Property::UP)
      style = styleObjBarGraph::up;
    else if(Prop->style & Property::DN)
      style = styleObjBarGraph::dn;
    }

  else
    style = styleObjBarGraph::noEraseBkg;

  return style;
}
//-----------------------------------------------------------
void svmObjBarGraph::setStyle(uint style)
{
  Prop->style = 0;
  switch(style) {
    case styleObjBarGraph::noBorder:
      Prop->style |= Property::FILL;
      break;
    case styleObjBarGraph::border:
      Prop->style |= Property::BORDER_FILL;
      break;
    case styleObjBarGraph::up:
      Prop->style |= Property::UP_FILL;
      break;
    case styleObjBarGraph::dn:
      Prop->style |= Property::DN_FILL;
      break;
    case styleObjBarGraph::noEraseBkg:
      Prop->style |= Property::NO;
      break;
    }
}
//-----------------------------------------------------------
bool svmObjBarGraph::saveColors(P_File& pf, uint id, const setOfValueColor& c)
{
  int nElem = c.getElem();
  TCHAR buff[1000];
  wsprintf(buff, _T("%d"), id);
  for(int i = 0; i < nElem; ++i) {
    TCHAR t[100];
    wsprintf(t, _T(",%d,%d,%d,%d"),
          (int)c[i].value,
          GetRValue(c[i].Color),
          GetGValue(c[i].Color),
          GetBValue(c[i].Color));
    _tcscat_s(buff, SIZE_A(buff), t);
    }
  _tcscat_s(buff, SIZE_A(buff), _T("\r\n"));
  return toBool(writeStringChkUnicode(pf, buff));
}
//-----------------------------------------------------------
void svmObjBarGraph::loadColors(uint id, setOfString& set, setOfValueColor& c)
{
  LPCTSTR p = set.getString(id);
  if(!p)
    return;
  int nElem = 0;
  do {
    int val = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &val, &r, &g, &b);
    c[nElem] = valueAndColor(RGB(r, g, b), (float)val);
    ++nElem;
    p = findNextParam(p, 4);
    } while(p);
}
//-----------------------------------------------------------
bool svmObjBarGraph::saveVarInfo(P_File& pf, uint firstAdd, uint secondAdd, const PropertyBarGraph::varInfo& vI)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n%d,%d,%d\r\n"),
              firstAdd,
              vI.perif,
              vI.addr,
              vI.typeVal,
              vI.normaliz,
              4,
              secondAdd,
              vI.nBit,
              vI.offs
              );
  return toBool(writeStringChkUnicode(pf, buff));
}
//-----------------------------------------------------------
void svmObjBarGraph::loadVarInfo(uint id, setOfString& set, PropertyBarGraph::varInfo& vI)
{
  int perif = 0;
  int addr = 0;
  int typeVal = 0;
  int normaliz = 0;

  manageObjId moi(id);
  LPCTSTR p = set.getString(moi.getFirstExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &perif, &addr, &typeVal, &normaliz);

  vI.perif = perif;
  vI.addr = addr;
  vI.typeVal = typeVal;
  vI.normaliz = normaliz;

  int nBit = 0;
  int offs = 0;
  p = set.getString(moi.getSecondExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d"), &nBit, &offs);

  vI.nBit = nBit;
  vI.offs = offs;
}
//-----------------------------------------------------------
#define MIDDLE_SUB 400
//-----------------------------------------------------------
static
int getIdBmpFlag(const PropertyBarGraph::infoBmp& ib, int& flag)
{
  dual* dualBmp = getDualBmp4Btn();
  int idbmp = 0;
  flag = 0;
  if(*ib.img) {
    idbmp = dualBmp->addAndGetId(ib.img);
    flag = ib.getFlag();
    }
  return idbmp;
}
//-----------------------------------------------------------
bool svmObjBarGraph::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_BARGRAPH);
  uint id = moi.calcAndSetExtendId(order);

//  int id = ID_INIT_BARGRAPH;
//  id += order;

  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(Prop);
  if(!pb)
    return false;

  TCHAR buff[1000];
  int style = getStyle();

  bool vert = toBool(Prop->type1 & 1);
  bool to_left = toBool(Prop->type1 & 2);
  if(vert)
    style += 1000;
  if(to_left)
    style += 100;

  svmObjCount& objCount = getObjCount();
  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idMax = objCount.getIdCount(oEDIT);
  idMax = moi2.calcAndSetExtendId(idMax);

  saveVarInfo(pfCrypt, moi2.getFirstExtendId(), moi2.getSecondExtendId(), pb->vI[1]);

  int idVal = objCount.getIdCount(oEDIT);
  idVal = moi2.calcAndSetExtendId(idVal);

  saveVarInfo(pfCrypt, moi2.getFirstExtendId(), moi2.getSecondExtendId(), pb->vI[0]);

  int idMin = objCount.getIdCount(oEDIT);
  idMin = moi2.calcAndSetExtendId(idMin);

  saveVarInfo(pfCrypt, moi2.getFirstExtendId(), moi2.getSecondExtendId(), pb->vI[2]);

  int idAlternColor = 0;
  const setOfValueColor& c = pb->getValueColors();
  if(c.getElem()) {
    idAlternColor = objCount.getGlobalIdCount();
    saveColors(pfCrypt, idAlternColor, c);
    }

  int x = Prop->Rect.left;
  int y = Prop->Rect.top;
  int w = Prop->Rect.Width();
  int h = Prop->Rect.Height();

  if(vert) {
    int t = w;
    w = h;
    h = t;
    }

  dual* dualBmp = getDualBmp4Btn();
  int flagbkg = 0;
  int idbmpbkg = getIdBmpFlag(pb->iBmp[0], flagbkg);

  int flagbar = 0;
  int idbmpbar = getIdBmpFlag(pb->iBmp[1], flagbar);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(x),
            REV__X(y),
            REV__X(w),
            REV__X(h),

            style,
            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),
            idMax, idVal, idAlternColor,
            pb->negValue ? -1 : 0,
            idbmpbar, flagbar,
            idbmpbkg, flagbkg,
            idMin, pb->centerZero, pb->valueTest[0], pb->valueTest[1]
            );

  writeStringChkUnicode(pfCrypt, buff);
  saveOnlyVisVar(moi.getFirstExtendId(), pfCrypt);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
static
void setIdBmpFlag(PropertyBarGraph::infoBmp& ib, int idbmp, int flag)
{
  dual* dualBmp = getDualBmp4Btn();
  LPCTSTR name = dualBmp->getTxt(idbmp);
  if(name) {
    _tcscpy_s(ib.img, getOnlyName(name, true));
    ib.setFlag(flag);
    }
}
//-----------------------------------------------------------
bool svmObjBarGraph::load(uint id, setOfString& set)
{
  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(Prop);
  if(!pb)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int style = 0;
  int idMax = 0;
  int idVal = 0;
  int idAlternColor = 0;
  int trunc = 0;

  int flagbkg = 0;
  int idbmpbkg = 0;

  int flagbar = 0;
  int idbmpbar = 0;

  int idMin = 0;
  int centerZero = 0;
  LPCTSTR p = set.getString(id);
  manageObjId moi(id, ID_INIT_BARGRAPH);
  loadOnlyVisVar(set, moi.getFirstExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style,
                    &Rbk, &Gbk, &Bbk,
                    &idMax, &idVal,
                    &idAlternColor,
                    &trunc,
                    &idbmpbar, &flagbar,
                    &idbmpbkg, &flagbkg,
                    &idMin, &centerZero, &pb->valueTest[0], &pb->valueTest[1]
                    );

  pb->centerZero = centerZero;
  setIdBmpFlag(pb->iBmp[0], idbmpbkg, flagbkg);
  setIdBmpFlag(pb->iBmp[1], idbmpbar, flagbar);

  pb->negValue = -1 == trunc ? true : false;
  Prop->type1 = 0;
  if(style >= 1000) {
    Prop->type1 |= 1;
    style -= 1000;
    int t = w;
    w = h;
    h = t;
    }
  if(style >= 100) {
    Prop->type1 |= 2;
    style -= 100;
    }

  setStyle(style);

  Prop->background = RGB(Rbk, Gbk, Bbk);

  loadVarInfo(idMax, set, pb->vI[1]);
  loadVarInfo(idVal, set, pb->vI[0]);
  loadVarInfo(idMin, set, pb->vI[2]);

  if(idAlternColor) {
    setOfValueColor& c = pb->getValueColors();
    loadColors(idAlternColor, set, c);
    }

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  return true;
}
//-----------------------------------------------------------
void svmObjBarGraph::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(Prop);
  LPCTSTR info[] = { _T("Max valore"), _T("Min valore") };
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const PropertyBarGraph::varInfo& vI = pb->vI[i + 1];
    if(!vI.perif)
      continue;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, info[i]);
    ia4->prph = vI.perif;
    ia4->addr = vI.addr;
    ia4->type = vI.typeVal;
    ia4->nBit = vI.nBit;
    ia4->offs = vI.offs;
    set.add(ia4);
    }
}
