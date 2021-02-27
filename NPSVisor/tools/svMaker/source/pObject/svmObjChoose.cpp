//-------------------- svmObjChoose.cpp -----------------------
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
#include "svmObjChoose.h"
#include "svmDefObj.h"
#include "svmPropertyChoose.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
bool isObjChoose(svmObject* t) { return toBool(dynamic_cast<svmObjChoose*>(t)); }
svmObject* allocObjChooseByText(svmObject* choose, int id) { return new svmObjChoose(choose, id); }
//-----------------------------------------------------------
static PropertyChoose staticPT;
//-----------------------------------------------------------
svmObjChoose::svmObjChoose(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oCHOOSE, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjChoose::svmObjChoose(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oCHOOSE, r)
{
  init();
}
//-----------------------------------------------------------
svmObjChoose::svmObjChoose(svmObject* other, int id) :
  svmObject(other->getParent(), id, oCHOOSE, other->getRect())
{
  Prop = new PropertyChoose;
  *Prop = *other->getProp();
  Prop->style = Property::UP_FILL;
}
//-----------------------------------------------------------
void svmObjChoose::init()
{
  Prop = new PropertyChoose;
  PropertyChoose* po = dynamic_cast<PropertyChoose*>(Prop);
  setOfPText& set = po->getSet();
  set[0] = new chooseText(_T("Scelta"));
  Prop->Rect = getFirstRect();
  Prop->style = Property::UP_FILL;
}
//-----------------------------------------------------------
#define MAX_ARROW_SIZE 24
//-----------------------------------------------------------
void svmObjChoose::DrawObject(HDC hdc)
{
  PRect r(getRect());
  r.Inflate(-1, -1);
  int h = r.Height();
  if(h > MAX_ARROW_SIZE)
    h = MAX_ARROW_SIZE;
  r.right -= h;
  PPanel panel(r, Prop->background, (PPanel::bStyle)Prop->style);
  panel.draw(hdc);
  r.right += h;
  r.bottom = r.top + h;
  r.left = r.right - h;
  r.Inflate(-2, -2);
  DrawFrameControl(hdc, r, DFC_SCROLL, DFCS_SCROLLDOWN);
}
//-----------------------------------------------------------
smartPointerConstString svmObjChoose::getTitle() const
{
  return getStringByLangSimple(dynamic_cast<PropertyChoose*>(Prop)->getText());
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjChoose::getRectTitle(HDC, LPCTSTR /*title*/) const
{
#if 1
  PRect r(Prop->Rect);
  int h = r.Height();
  if(h > MAX_ARROW_SIZE)
    h = MAX_ARROW_SIZE;
  r.right -= h;
  return r;
#else
  PRect r(Prop->Rect);
  r.Inflate(-2, 0);
  return r;
#endif
}
//-----------------------------------------------------------
svmObject* svmObjChoose::makeClone()
{
  svmObjChoose* obj = new svmObjChoose(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjChoose::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjChoose::allocProperty() { return new PropertyChoose; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjChoose::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyChoose*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogChoose(this, Prop, getParent()).modal()) {
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
bool svmObjChoose::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_CHOOSE);
  uint id = moi.calcAndSetExtendId(order);
  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = getStyle();

  int align = Prop->alignText | Property::aMid;

  int idColor = 0;

  svmObjCount& objCount = getObjCount();

  const setOfPText& set = pt->getSet();
  int nElem = set.getElem();
  if(!nElem)
    return false;

  int idMsg = objCount.getGlobalIdCount(nElem + 1);
  for(int i = 0; i < nElem; ++i) {
    const chooseText ct = *set[i];
    wsprintf(buff, _T("%d,%s\r\n"), idMsg + i, ct.getText());
    writeStringChkUnicode(pfClear, pfCrypt, buff);
    }

  // essendo testo variabile il normalizzatore deve essere negativo e al
  // posto dei decimali viene inserito l'id del primo testo
  pt->normaliz = -1;
  pt->nDec = idMsg;

  const setOfPTextColor& setC = pt->getSetColor();
  nElem = setC.getElem();

  if(nElem > 0 && pt->usingColor()) {
    idColor = objCount.getGlobalIdCount(nElem + 1);
    for(int i = 0; i < nElem; ++i) {
      const textAndColor* tac = setC[i];

      wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"),
                  idColor + i,
                  GetRValue(tac->fgColor),
                  GetGValue(tac->fgColor),
                  GetBValue(tac->fgColor),

                  GetRValue(tac->bgColor),
                  GetGValue(tac->bgColor),
                  GetBValue(tac->bgColor));
      writeStringChkUnicode(pfCrypt, buff);
      }
    }
  uint useAltText = 0;
  alternateTextSet& at = pt->getAltObj();

  if(at.getFileName() || at.getPrph()) {
    useAltText = objCount.getGlobalIdCount(1);
    at.save(pfCrypt, useAltText);
    }
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),

            Prop->idFont + ID_INIT_FONT,

            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),

            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),

            style,

            align,
            idColor,
            pt->usingCustomSend(),
            useAltText
            );
  writeStringChkUnicode(pfCrypt, buff);

  saveVar(pfCrypt, id);

  wsprintf(buff, _T("%d"), id + 900);
  TCHAR t[100];
  for(int i = 0; i < MAX_LB_COLORS; ++i) {
    wsprintf(t, _T(",%d,%d,%d"),
            GetRValue(pt->lbColors[i]),
            GetGValue(pt->lbColors[i]),
            GetBValue(pt->lbColors[i])
            );
    _tcscat_s(buff, t);
    }
  _tcscat_s(buff, _T("\r\n"));
  writeStringChkUnicode(pfCrypt, buff);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjChoose::load(uint id, setOfString& set)
{

  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = ID_INIT_FONT;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int style = 0;
  int align = 0;
  int idColor = 0;
  int useCustomSend = 0;
  int useAltText = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &style, &align, &idColor,
                    &useCustomSend, &useAltText);
  pt->setUseCustomSend(toBool(useCustomSend));

  if(useAltText) {
    alternateTextSet& at = pt->getAltObj();
    at.load(set, useAltText);
    }

  idfont -= ID_INIT_FONT;

  PPanel::bStyle type = style == 0 ? PPanel::TRANSP  :
                        style == 1 ? PPanel::UP_FILL :
                        style == 2 ? PPanel::DN_FILL :
                        style == 3 ? PPanel:: FILL   :

                        style == 4 ? PPanel::UP_TRANSP :
                        style == 5 ? PPanel::DN_TRANSP :
                        style == 6 ? PPanel::BORDER_FILL :
                                     PPanel::BORDER_TRANSP;
  Prop->foreground = RGB(Rfg, Gfg, Bfg);
  Prop->background = RGB(Rbk, Gbk, Bbk);
  Prop->style = type;
  Prop->idFont = idfont;

  Prop->alignText = align | Property::aMid;

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  loadVar(id, set);

  setOfPTextColor& sTC = pt->getSetColor();
  flushPV(sTC);
  pt->setUseColor(toBool(idColor));

  setOfPText& sT = pt->getSet();
  flushPV(sT);

  int idMsg = Prop->nDec;
  int i;
  int j = 0;
  for(i = 0; ; ++i) {
    p = set.getString(idMsg + i);
    if(!p)
      break;
    TCHAR buff[500];
    translateFromCRNL(buff, p);
    sT[i] = new chooseText(buff);
    if(idColor) {
      p = set.getString(idColor + i);
      if(p) {
        _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk);
        textAndColor* tac = new textAndColor(0, RGB(Rfg, Gfg, Bfg), RGB(Rbk, Gbk, Bbk), 0);
        sTC[j++] = tac;
        }
      }
    }
  if(!i)
    sT[0] = new chooseText(_T("Scelta"));

  p = set.getString(id + 900);
  if(p) {
    for(int i = 0; i < MAX_LB_COLORS; ++i) {
      _stscanf_s(p, _T("%d,%d,%d"), &Rfg, &Gfg, &Bfg);
      pt->lbColors[i] = RGB(Rfg, Gfg, Bfg);
      p = findNextParam(p, 3);
      if(!p)
        break;
      }
    }

  return true;
}


