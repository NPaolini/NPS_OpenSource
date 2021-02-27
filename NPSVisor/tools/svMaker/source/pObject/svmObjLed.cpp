//-------------------- svmObjLed.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "common.h"
#include "p_File.h"
#include "pPanel.h"
#include "pModDialog.h"
#include "svmObjLed.h"
#include "svmDefObj.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
static PropertyLed staticPT;
//-----------------------------------------------------------
svmObjLed::svmObjLed(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oLED, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjLed::svmObjLed(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oLED, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjLed::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  const int dim = 40;
  Prop->Rect.right = Prop->Rect.left + dim * 2;
  Prop->Rect.bottom = Prop->Rect.top + dim;
}
//-----------------------------------------------------------
Property* svmObjLed::allocProperty() { return new PropertyLed; }
//-----------------------------------------------------------
#define MAX_BMP_LED 12
#define nDigit nBits
//-----------------------------------------------------------
void svmObjLed::DrawObject(HDC hdc)
{
//  staticPT.Rect = getRect();

  PPanel panel(getRect(), Prop->background, (PPanel::bStyle)Prop->style);
  panel.draw(hdc);

  PropertyLed* pb = dynamic_cast<PropertyLed*>(Prop);
  const PVect<LPCTSTR>& names = pb->getNames();
  if(names.getElem() >= MAX_BMP_LED) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, names[0]);
    PBitmap bmpDigit(name);
    if(bmpDigit.isValid()) {
      SIZE szDigit = bmpDigit.getSize();

      makeNameImage(name, names[MAX_BMP_LED - 1]);
      PBitmap bmpPoint(name);
      if(bmpPoint.isValid()) {
        SIZE szPoint = bmpPoint.getSize();

        int width = szDigit.cx;

        width *= pb->nDigit;
        width += szPoint.cx;

        PRect allRect(getRect());

        allRect.Inflate(-2, -2);
        double scaleX = allRect.Width();
        scaleX /= width;
        double scaleY = allRect.Height();
        scaleY /= szDigit.cy;

        int integer = pb->nDigit - pb->nDec;
        POINT pt = { allRect.left, allRect.top };

        width = (int)(szDigit.cx * scaleX);

        int i;
        for(i = 0; i < integer; ++i) {
          int ix = i % 10;
          makeNameImage(name, names[ix]);
          PBitmap bmp(name);
          PTraspBitmap tb(getParent(), &bmp, pt);
          tb.setScale(scaleX, scaleY);
          tb.Draw(hdc);
          pt.x += width;
          }
        {
        PTraspBitmap tbPoint(getParent(), &bmpPoint, pt);
        tbPoint.setScale(scaleX, scaleY);
        tbPoint.Draw(hdc);
        }
        pt.x += (LONG)(szPoint.cx * scaleX);

        for(; i < (int)pb->nDigit; ++i) {
          int ix = i % 10;
          makeNameImage(name, names[ix]);
          PBitmap bmp(name);
          PTraspBitmap tb(getParent(), &bmp, pt);
          tb.setScale(scaleX, scaleY);
          tb.Draw(hdc);
          pt.x += width;
          }
        }
      }
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjLed::getTitle() const
{
  PropertyLed* pb = dynamic_cast<PropertyLed*>(Prop);
  const PVect<LPCTSTR>& names = pb->getNames();
  if(names.getElem() >= MAX_BMP_LED)
    return smartPointerConstString(0, 0);
  return smartPointerConstString(_T("Led"), 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjLed::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return Prop->Rect;
}
//-----------------------------------------------------------
svmObject* svmObjLed::makeClone()
{
  svmObjLed* obj = new svmObjLed(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjLed::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int svmObjLed::getIdType() const
{
  return baseClass::getIdType();
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjLed::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyLed*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogLed(this, Prop, getParent()).modal()) {
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
//-----------------------------------------------------------
bool svmObjLed::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_LED);
  uint id = moi.calcAndSetExtendId(order);
  PropertyLed* pb = dynamic_cast<PropertyLed*>(Prop);
  if(!pb)
    return false;

  const PVect<LPCTSTR>& names = pb->getNames();
  int nBmp = names.getElem();
  if(nBmp < MAX_BMP_LED)
    return false;


  TCHAR buff[1000];
  dual* dualBmp = getDualBmp4Btn();
  svmObjCount& objCount = getObjCount();

  PVect<int> idbmp;
  idbmp.setDim(nBmp);
  for(int i = 0; i < nBmp; ++i)
    idbmp[i] = dualBmp->addAndGetId(names[i]);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),
            Prop->nBits,
            Prop->nDec,
            Prop->style,
            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),
            !pb->showZero
            );
  writeStringChkUnicode(pfCrypt, buff);

  wsprintf(buff, _T("%d"), moi.getFirstExtendId());

  for(int i = 0; i < nBmp; ++i) {
    TCHAR t[50];
    wsprintf(t, _T(",%d"), idbmp[i]);
    _tcscat_s(buff, t);
    }
  _tcscat_s(buff, _T("\r\n"));
  writeStringChkUnicode(pfCrypt, buff);

  wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"),
            moi.getSecondExtendId(),
            Prop->perif,
            Prop->addr,
            Prop->typeVal,
            Prop->normaliz
            );
  writeStringChkUnicode(pfCrypt, buff);

//  if(Prop->visPerif)
    saveOnlyVisVar(moi.getThirdExtendId(), pfCrypt);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
extern LPCTSTR removePath(LPCTSTR path);
//-----------------------------------------------------------
bool svmObjLed::load(uint id, setOfString& set)
{
  PropertyLed* pb = dynamic_cast<PropertyLed*>(Prop);
  if(!pb)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int digit = 0;
  int dec = 0;
  int style = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int showZero = 0;
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &digit, &dec,
                    &style,
                    &Rbk, &Gbk, &Bbk,
                    &showZero);
  Prop->nDec = dec;
  Prop->nBits = digit;
  Prop->style = style;
  Prop->background = RGB(Rbk, Gbk, Bbk);
  pb->showZero = toBool(showZero);

  PVect<int> idbmp;
  manageObjId moi(id, ID_INIT_VAR_LED);
  p = set.getString(moi.getFirstExtendId());
  for(int i = 0; p; ++i) {
    idbmp[i] = _ttoi(p);
    p = findNextParam(p, 1);
    }

  PVect<LPCTSTR>& names = pb->getNames();
  dual* dualBmp;
  if(Linked)
    dualBmp = getDualBmp4BtnLinked();
  else
    dualBmp = getDualBmp4Btn();

  int nBmp = idbmp.getElem();
  int j = 0;
  for(int i = 0; i < nBmp; ++i) {
    LPCTSTR name = dualBmp->getTxt(idbmp[i]);
    if(name)
#if 1
      names[j++] = str_newdup(getOnlyName(name, true));
#else
      names[j++] = str_newdup(removePath(name));
#endif
    }

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));


  int perif = 0;
  int addr = 0;
  int typeVal = 0;
  int normaliz = 0;

  p = set.getString(moi.getSecondExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &perif, &addr, &typeVal, &normaliz);

  Prop->perif = perif;
  Prop->addr = addr;
  Prop->typeVal = typeVal;
  Prop->normaliz = normaliz;

  loadOnlyVisVar(set, moi.getThirdExtendId());

  Prop->nBits = digit;
  return true;
}


