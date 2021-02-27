//-------------------- svmObjSimpleBmp.cpp ------------------
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
#include "svmObjSimpleBmp.h"
#include "svmDefObj.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
static PropertySimpleBmp staticPT;
//-----------------------------------------------------------
svmObjSimpleBmp::svmObjSimpleBmp(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oBKG_BITMAP, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjSimpleBmp::svmObjSimpleBmp(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oBKG_BITMAP, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjSimpleBmp::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  const int dim = 40;
  Prop->Rect.right = Prop->Rect.left + dim * 2;
  Prop->Rect.bottom = Prop->Rect.top + dim;
}
//-----------------------------------------------------------
Property* svmObjSimpleBmp::allocProperty() { return new PropertySimpleBmp; }
//-----------------------------------------------------------
void svmObjSimpleBmp::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d, zOrder=%d"), getObjName(), getId(), zOrder);
  addTipsVis(tips, size);
}
//-----------------------------------------------------------
void svmObjSimpleBmp::DrawObject(HDC hdc)
{
//  staticPT.Rect = getRect();

  PropertySimpleBmp* pb = dynamic_cast<PropertySimpleBmp*>(Prop);
  if(pb && pb->filename) {
    svmMainClient* wMain = dynamic_cast<svmMainClient*>(getParent());
    if(wMain) {
      int scaleX = 0;
      int scaleY = 0;
      PRect r = getRect();
      if(pb->szBmp.cx) {
        double d = r.Width();
        scaleX = (int)(d / (double)pb->szBmp.cx * (double)1000);
        }
      if(pb->szBmp.cy) {
        double d = r.Height();
        scaleY = (int)(d / (double)pb->szBmp.cy * (double)1000);
        }
      TCHAR buff[_MAX_PATH];
      TCHAR name[_MAX_PATH];
      makeRelNameImage(name, pb->filename, false);
      wsprintf(buff, _T("%d,%d,%d,%d,%d,%s"),
          REV__X(r.left),
          REV__Y(r.top),
          REV__X(scaleX),
          REV__Y(scaleY),
          Prop->style & Property::TRANSP ? 1000 : 0,
          name);

      wMain->drawBmp(hdc, buff);
      return;
      }
    }
  PPanel panel(getRect(), GetSysColor(COLOR_BTNFACE), PPanel::UP_FILL);
  panel.draw(hdc);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmObjSimpleBmp::DrawTitle(HDC hdc)
{
  baseClass::DrawTitle(hdc);

  PropertySimpleBmp* pb = dynamic_cast<PropertySimpleBmp*>(Prop);
  if(!pb || !pb->filename) {
    PRect r = getRect();
    int h = r.Height() / 3;
    if(h > R__Y(20))
      h = R__Y(20);
    HFONT hf = D_FONT(h, 0, fBOLD, _T("arial"));
    HGDIOBJ oldFont = SelectObject(hdc, hf);


    int oldMode = SetBkMode(hdc, TRANSPARENT);
    int oldColor = SetTextColor(hdc, 0);

    DrawText(hdc, _T("Semplice BMP"), -1, r, DT_CENTER | DT_WORDBREAK | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hf);

    SetTextColor(hdc, oldColor);
    SetBkMode(hdc, oldMode);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjSimpleBmp::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjSimpleBmp::getRectTitle(HDC, LPCTSTR /*title*/) const
{
#if 1
  return Prop->Rect;
#else
  PRect r(Prop->Rect);
  int w = r.Width() / 3;
  r.Inflate(-w, 0);
  return r;
#endif
}
//-----------------------------------------------------------
svmObject* svmObjSimpleBmp::makeClone()
{
  svmObjSimpleBmp* obj = new svmObjSimpleBmp(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjSimpleBmp::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int svmObjSimpleBmp::getIdType() const
{
  return baseClass::getIdType();
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjSimpleBmp::dialogProperty(bool onCreate)
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
    if(IDOK == svmDialogSimpleBmp(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
bool svmObjSimpleBmp::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_BMP);
  uint id = moi.calcAndSetExtendId(order);
  PropertySimpleBmp* pb = dynamic_cast<PropertySimpleBmp*>(Prop);
  if(!pb)
    return false;
  if(!pb->filename)
    return false;

  PRect r = getRect();
  SIZE sz = pb->szBmp;
  if(!sz.cx || !sz.cy)
    return false;
  double scaleX = REV__Xf(r.Width());
  double scaleY = REV__Xf(r.Height());
  if(isMetafile(pb->filename)) {
    scaleX = -(scaleX + 0.45);
    scaleY = -(scaleY + 0.45);
    }
  else {
    scaleX /= (double)sz.cx;
    scaleX *= (double)1000;
    scaleX += 0.45;
    scaleY /= (double)sz.cy;
    scaleY *= (double)1000;
    scaleY += 0.45;
    }
  int flag = 0;
  if(Prop->style & Property::TRANSP)
    flag = 1000;
  TCHAR name[_MAX_PATH];
  TCHAR buff[500];
  makeRelNameImage(name, pb->filename, true);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%s\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            (int)scaleX,
            (int)scaleY,
            flag, name);
  writeStringChkUnicode(pfCrypt, buff);
//  if(Prop->visPerif)
    saveOnlyVisVar(pfCrypt, id, Prop->type1);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
extern LPCTSTR removePath(LPCTSTR path);
//-----------------------------------------------------------
bool svmObjSimpleBmp::load(uint id, setOfString& set)
{
  PropertySimpleBmp* pb = dynamic_cast<PropertySimpleBmp*>(Prop);
  if(!pb)
    return false;

  int x = 0;
  int y = 0;
  int scaleX = 0;
  int scaleY = 0;
  int flag = 0;

  LPCTSTR p = set.getString(id);
  if(p) {
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &scaleX, &scaleY, &flag);
    p = findNextParam(p, 5);
    }
  if(!scaleX)
    scaleX = 1000;
  if(!scaleY)
    scaleY = 1000;

  Prop->style = 0;
  if(flag >= 1000) {
    Prop->style = Property::TRANSP;
    flag -= 1000;
    }
  Prop->type1 = flag;
#if 1
   pb->filename = str_newdup(getOnlyName(p, true));
#else
   pb->filename = str_newdup(removePath(p));
#endif
  int w = 60;
  int h = 30;

  if(*pb->filename) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, pb->filename);
    PBitmap bmp(name);
    if(bmp.isValid()) {
      pb->szBmp = bmp.getSize();
      if(scaleX < 0)
        w = -scaleX;
      else {
        double tScale = scaleX;
        tScale /= 1000;
        w = (int)(tScale * pb->szBmp.cx + 0.45);
        }
      if(scaleY < 0)
        h = -scaleY;
      else {
        double tScale = scaleY;
        tScale /= 1000;
        h = (int)(tScale * pb->szBmp.cy + 0.45);
        }
      }
    }

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  loadOnlyVisVar(id, set, &Prop->type1);
  return true;
}


