//-------------------- svmObjSlider.cpp -----------------------
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
#include "svmObjSlider.h"
#include "svmDefObj.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertySlider staticPT;
//-----------------------------------------------------------
svmObjSlider::svmObjSlider(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oSLIDER, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjSlider::svmObjSlider(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oSLIDER, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjSlider::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjSlider::DrawObject(HDC hdc)
{
  PropertySlider* ps = dynamic_cast<PropertySlider*>(Prop);
  if(ps) {
    LPCTSTR pname = ps->img;
    if(*pname) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, pname);
      PBitmap bmp(name);
      if(bmp.isValid()) {
        PRect r = getRect();
        SIZE sz = bmp.getSize();
        POINT pt = { r.left, r.top };
        double scale;
        if(ps->vert) {
          scale = r.Width();
          scale /= (double)sz.cy;
          if(ps->mirror)
            PBitmap::rotateInside(&bmp, Gdiplus::Rotate270FlipY);
          else
            PBitmap::rotateInside(&bmp, Gdiplus::Rotate270FlipNone);
          pt.y = r.bottom - ROUND_REAL(sz.cx * scale);
          }
        else {
          scale = r.Height();
          scale /= (double)sz.cy;
          if(ps->mirror)
            PBitmap::rotateInside(&bmp, Gdiplus::RotateNoneFlipX);
          }

        pt.x = REV__X(pt.x);
        pt.y = REV__Y(pt.y);

        pt.x = R__X(pt.x);
        pt.y = R__Y(pt.y);
        PTraspBitmap tb(getParent(), &bmp, pt);
        tb.setScale(scale, scale);
        tb.Draw(hdc);
        return;
        }
      }
    }
  PPanel panel(getRect(), GetSysColor(COLOR_BTNFACE), PPanel::UP_FILL);
  panel.draw(hdc);
}
//-----------------------------------------------------------
smartPointerConstString svmObjSlider::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjSlider::getRectTitle(HDC, LPCTSTR /*title*/) const
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
svmObject* svmObjSlider::makeClone()
{
  svmObjSlider* obj = new svmObjSlider(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjSlider::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjSlider::allocProperty() { return new PropertySlider; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjSlider::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertySlider*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogSlider(this, Prop, getParent()).modal()) {
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
bool svmObjSlider::saveVarInfo(P_File& pf, uint firstAdd, uint secondAdd, const PropertySlider::varInfo& vI)
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
void svmObjSlider::loadVarInfo(uint id, setOfString& set, PropertySlider::varInfo& vI)
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
static
int getIdBmpFlag(LPCTSTR img)
{
  dual* dualBmp = getDualBmp4Btn();
  int idbmp = 0;
  if(img)
    idbmp = dualBmp->addAndGetId(img);
  return idbmp;
}
//-----------------------------------------------------------
bool svmObjSlider::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_SLIDER);
  uint id = moi.calcAndSetExtendId(order);

  PropertySlider* pb = dynamic_cast<PropertySlider*>(Prop);
  if(!pb)
    return false;

  TCHAR buff[1000];

  svmObjCount& objCount = getObjCount();
  manageObjId moi2(0, ID_INIT_VAR_EDI);

  int idMin = objCount.getIdCount(oEDIT);
  idMin = moi2.calcAndSetExtendId(idMin);
  saveVarInfo(pfCrypt, moi2.getFirstExtendId(), moi2.getSecondExtendId(), pb->vI[0]);

  int idMax = objCount.getIdCount(oEDIT);
  idMax = moi2.calcAndSetExtendId(idMax);
  saveVarInfo(pfCrypt, moi2.getFirstExtendId(), moi2.getSecondExtendId(), pb->vI[1]);

  int x = Prop->Rect.left;
  int y = Prop->Rect.top;
  int w = Prop->Rect.Width();
  int h = Prop->Rect.Height();

  if(pb->vert) {
    int t = w;
    w = h;
    h = t;
    }

  int idbmp = getIdBmpFlag(pb->img);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(x),
            REV__X(y),
            REV__X(w),
            REV__X(h),

            idMax, idMin, idbmp, pb->H_W, pb->fullImage, pb->vert, pb->mirror, pb->reverse,
                    pb->readOnly, pb->updateAlways
            );

  writeStringChkUnicode(pfCrypt, buff);
  saveVar(pfCrypt, id);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
static
void setIdBmpFlag(LPTSTR img, uint dim, int idbmp)
{
  dual* dualBmp = getDualBmp4Btn();
  LPCTSTR name = dualBmp->getTxt(idbmp);
  if(name)
    _tcscpy_s(img, dim, getOnlyName(name, true));
}
//-----------------------------------------------------------
#define SET_V(v) pt->v = v
//-----------------------------------------------------------
bool svmObjSlider::load(uint id, setOfString& set)
{
  PropertySlider* pt = dynamic_cast<PropertySlider*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int idMax = 0;
  int idMin = 0;
  int idBmp = 0;
  int H_W = 0;
  int fullImage = 0;
  int vert = 0;
  int mirror = 0;
  int reverse = 0;
  int readOnly = 0;
  int updateAlways = 0;

  LPCTSTR p = set.getString(id);
  manageObjId moi(id, ID_INIT_VAR_SLIDER);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idMax, &idMin, &idBmp, &H_W,
                    &fullImage, &vert, &mirror, &reverse,
                    &readOnly, &updateAlways
                    );

  SET_V(H_W);
  SET_V(fullImage);
  SET_V(vert);
  SET_V(mirror);
  SET_V(reverse);
  SET_V(readOnly);
  SET_V(updateAlways);

  setIdBmpFlag(pt->img, SIZE_A(pt->img), idBmp);
  if(vert) {
    int t = w;
    w = h;
    h = t;
    }

  loadVarInfo(idMin, set, pt->vI[0]);
  loadVarInfo(idMax, set, pt->vI[1]);

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  loadVar(id, set);
  return true;
}
//-----------------------------------------------------------
void svmObjSlider::addInfoAd4Other(manageInfoAd4& set)
{
  PropertySlider* pb = dynamic_cast<PropertySlider*>(Prop);
  LPCTSTR info[] = { _T("Min valore"), _T("Max valore") };
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const PropertySlider::varInfo& vI = pb->vI[i];
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
//-----------------------------------------------------------
