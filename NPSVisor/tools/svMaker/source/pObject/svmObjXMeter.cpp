//-------------------- svmObjXMeter.cpp -----------------------
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
#include "svmObjXMeter.h"
#include "svmDefObj.h"
#include "svmPropertyXMeter.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyXMeter staticPT;
//-----------------------------------------------------------
svmObjXMeter::svmObjXMeter(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oXMETER, x1, y1, x1 + R__X(100), y1 + R__Y(20))
{
  init();
}
//-----------------------------------------------------------
svmObjXMeter::svmObjXMeter(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oXMETER, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjXMeter::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
void svmObjXMeter::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
      PropertyXMeter* p = (PropertyXMeter*)Prop;
      for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
        if(isValidPrph4OffsetAddr(p->DataPrf[i].perif, ior))
          if(isValid4OffsetAddr(p->DataPrf[i].perif, p->DataPrf[i].addr, exclude))
            p->DataPrf[i].addr += ior.offs;
        }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    PropertyXMeter* p = (PropertyXMeter*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom == p->DataPrf[i].perif)
        p->DataPrf[i].perif = ior.prphTo;
      }
    }
  if(isSetBitf(eoSubstVar, ior.flag)) {
    PropertyXMeter* p = (PropertyXMeter*)Prop;
    for(uint i = 0; i < SIZE_A(p->DataPrf); ++i) {
      if(ior.prphFrom2 == p->DataPrf[i].perif && ior.addrFrom == p->DataPrf[i].addr) {
        p->DataPrf[i].perif = ior.prphTo2;
        p->DataPrf[i].addr = ior.addrTo;
        }
      }
    }
}
//----------------------------------------------------------------------------
Property* svmObjXMeter::allocProperty() { return new PropertyXMeter; }
//-----------------------------------------------------------
void svmObjXMeter::DrawObject(HDC hdc)
{
  PropertyXMeter* pxm = dynamic_cast<PropertyXMeter*>(Prop);
  if(pxm) {
    LPCTSTR pname = pxm->getName();
    if(pname) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, pname);
      PBitmap bmp(name);
      if(bmp.isValid()) {
        PRect r = getRect();
        SIZE sz = bmp.getSize();
        double scaleX = r.Width();
        scaleX /= (double)sz.cx;
        double scaleY = r.Height();
        scaleY /= (double)sz.cy;

        POINT pt = { r.left, r.top };

        pt.x = REV__X(pt.x);
        pt.y = REV__Y(pt.y);

        pt.x = R__X(pt.x);
        pt.y = R__Y(pt.y);

        PTraspBitmap tb(getParent(), &bmp, pt);
        tb.setScale(scaleX, scaleY);
        tb.Draw(hdc);
        return;
        }
      }
    }
  PPanel panel(getRect(), GetSysColor(COLOR_BTNFACE), PPanel::UP_FILL);
  panel.draw(hdc);
}
//-----------------------------------------------------------
#define DELTA_C 5
//-----------------------------------------------------------
void svmObjXMeter::drawHandleUI(HDC hdc)
{
  PropertyXMeter* pb = dynamic_cast<PropertyXMeter*>(Prop);
  if(pb) {
    int offsX = pb->center ? Prop->Rect.Width() / 2 : pb->offsX;
    int offsY = pb->center ? Prop->Rect.Height() / 2 : pb->offsY;
    int radius = Prop->Rect.Width() - offsX;
    if(radius < 0)
      radius = -radius;

    POINT pt;
    pt.x = Prop->Rect.right - radius;
    pt.y = Prop->Rect.bottom - offsY;

    if(Inside == whichPoint || Extern == whichPoint) {
      pt.x += RectDragging.left - RectDragging.right;
      pt.y += RectDragging.top - RectDragging.bottom;
      }
    else {
      pt.x += RectDragging.left - Prop->Rect.left;
      pt.y += RectDragging.top - Prop->Rect.top;
      }

    int oldRop = SetROP2(hdc, R2_XORPEN);
    HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 255, 255));
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    getDataProject().calcToScreen(pt);

    MoveToEx(hdc, pt.x, pt.y, 0);
    int initAngle = pb->type1;
    int sweep = pb->type2;
    if(pb->style & 1) {
      sweep = -sweep;
      }
//      initAngle -= pb->type2;
    AngleArc(hdc, pt.x, pt.y, radius, (float)initAngle, (float)sweep);
    DeleteObject(SelectObject(hdc, oldPen));
    pen = CreatePen(PS_SOLID, 1, getXoredColor());
    oldPen = SelectObject(hdc, pen);
    LineTo(hdc, pt.x, pt.y);

    DeleteObject(SelectObject(hdc, oldPen));
    SetROP2(hdc, oldRop);
    }
  baseClass::drawHandleUI(hdc);
  }
//-----------------------------------------------------------
smartPointerConstString svmObjXMeter::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjXMeter::getRectTitle(HDC, LPCTSTR /*title*/) const
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
svmObject* svmObjXMeter::makeClone()
{
  svmObjXMeter* obj = new svmObjXMeter(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjXMeter::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjXMeter::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyXMeter*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogXMeter(this, Prop, getParent()).modal()) {
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
bool svmObjXMeter::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_XMETER);
  uint id = moi.calcAndSetExtendId(order);
  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];

  svmObjCount& objCount = getObjCount();
  manageObjId moi2(0, ID_INIT_VAR_EDI);
  int idMin = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
  int idMax = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));

  dual* dualBmp = getDualBmp4Btn();
  int idbmp = 0;
  if(pt->getName())
    idbmp = dualBmp->addAndGetId(pt->getName());

//id,x,y,w,h,idImage,offsX,offsY,initAngle,ampiezza,clockwise,granularity,idMin,idMax,
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),

            idbmp,

            pt->offsX, pt->offsY,

            pt->type1, pt->type2,
            pt->style,
            pt->granularity,

            idMin,
            idMax,
            pt->active, pt->updateAlways, pt->center, pt->moveVert
            );

  writeStringChkUnicode(pfCrypt, buff);

//  dataPrf tmp(Prop->perif, Prop->addr, Prop->typeVal, Prop->normaliz);

//  savePrf(pfCript, buff, idCurr, tmp);
  savePrf(pfCrypt, buff, idMin, pt->DataPrf[0]);
  savePrf(pfCrypt, buff, idMax, pt->DataPrf[1]);

  saveVar(pfCrypt, id);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
extern LPCTSTR removePath(LPCTSTR path);
//-----------------------------------------------------------
bool svmObjXMeter::load(uint id, setOfString& set)
{
  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;
  int idbmp = 0;
  int style2 = 0;
  int idMin = 0;
  int idMax = 0;
  int idCurr = 0;
  int idColor = 0;
  int active = 0;
  int updateAlways = 0;
  int center = 0;
  int moveVert = 1;

//id,x,y,w,h,idImage,offsX,offsY,initAngle,ampiezza,clockwise,granularity,idMin,idMax
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idbmp,
                    &pt->offsX, &pt->offsY,

                    &pt->type1, &pt->type2,
                    &pt->style,
                    &pt->granularity,

                    &idMin, &idMax, &active,
                    &updateAlways, &center, &moveVert
                    );

  pt->active = active;
  pt->updateAlways = updateAlways;
  pt->center = center;
  pt->moveVert = moveVert;
  dual* dualBmp;
  if(Linked)
    dualBmp = getDualBmp4BtnLinked();
  else
    dualBmp = getDualBmp4Btn();
  pt->active = toBool(active);
  LPCTSTR name = dualBmp->getTxt(idbmp);
  if(name)
#if 1
    pt->setName(getOnlyName(name, true));
#else
    pt->setName(removePath(name));
#endif
  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  loadVar(id, set);

  loadPrf(idMin, set, pt->DataPrf[0]);
  loadPrf(idMax, set, pt->DataPrf[1]);


  return true;
}
//-----------------------------------------------------------
void svmObjXMeter::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyXMeter* pb = dynamic_cast<PropertyXMeter*>(Prop);
  LPCTSTR info[] = { _T("Valore min"), _T("Valore max") };
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
