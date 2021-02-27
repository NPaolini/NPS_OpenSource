//-------------------- svmObjCam.cpp -----------------------
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
#include "svmObjCam.h"
#include "svmDefObj.h"
#include "svmPropertyCam.h"
#include "svmObjButton.h"
#include "svmObjEdit.h"
#include "svmObjText.h"
#include "svmMainClient.h"
//-----------------------------------------------------------
#define DEF_W_CAM 320
#define DEF_H_CAM 240
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyCam staticPT;
//-----------------------------------------------------------
svmObjCam::svmObjCam(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oCAM,
      x1, y1,
      DATA_NOT_USED == x2 ? DEF_W_CAM + x1 : x2,
      DATA_NOT_USED == y2 ? DEF_H_CAM + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjCam::svmObjCam(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oCAM, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjCam::init()
{
  Prop = allocProperty();
  PropertyCam* po = dynamic_cast<PropertyCam*>(Prop);
  po->setText(_T("Cam"));
  Prop->Rect = getFirstRect();
  Prop->style = Property::UP_FILL;
}
//-----------------------------------------------------------
void svmObjCam::DrawObject(HDC hdc)
{
  PPanel panel(getRect(), Prop->background, (PPanel::bStyle)Prop->style);
  panel.draw(hdc);
//  staticPT.Rect = getRect();
#if 0
  HBRUSH brush = CreateSolidBrush(Prop->background);
  FillRect(hdc, Rect, brush);
  DeleteObject((HGDIOBJ)brush);

//  DrawEdge(hdc, Rect, BDR_SUNKENINNER, BF_RECT);
  DrawEdge(hdc, Rect, BDR_SUNKENOUTER, BF_RECT);
/*
  HBRUSH brush = CreateSolidBrush(Prop->background);
  FillRect(hdc, Rect, brush);
  DeleteObject((HGDIOBJ)brush);
//  FillRect(hdc, Rect, (HBRUSH) (GetStockObject(WHITE_BRUSH)));
  DrawEdge(hdc, Rect, EDGE_SUNKEN, BF_RECT);
*/
#endif
}
//-----------------------------------------------------------
LPCTSTR svmObjCam::getObjName() const
{
  return baseClass::getObjName();
}
//-----------------------------------------------------------
smartPointerConstString svmObjCam::getTitle() const
{
  return smartPointerConstString(dynamic_cast<PropertyCam*>(Prop)->getText(), 0);
}
//-----------------------------------------------------------
Property* svmObjCam::allocProperty() { return new PropertyCam; }
//-----------------------------------------------------------
PRect svmObjCam::getRectTitle(HDC, LPCTSTR /*title*/) const
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
svmObject* svmObjCam::makeClone()
{
  svmObjCam* obj = new svmObjCam(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjCam::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int svmObjCam::getIdType() const
{
//  if((dynamic_cast<PropertyCam*>(Prop))->isSimpleText())
//    return oSIMPLE_TEXT;
  return baseClass::getIdType();
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjCam::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyCam*>(Prop))->cloneMinusProperty(staticPT);
      }
    else {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      }
    }
  if(!oldProp)
    oldProp = allocProperty();
  *oldProp = *Prop;
  DWORD Bits = (DWORD)-1;
  DWORD* pBits = onCreate ? &Bits : 0;
  if(IDOK == svmDialogCam(this, Prop, getParent(), pBits).modal()) {
    if(Prop->getFullCloneOnCreate())
      staticPT = *Prop;
    else
      staticPT.cloneMinusProperty(*Prop);
    }
  else if(onCreate)
    return svmObjHistory::Abort;
  if(onCreate ) {
    makeBtn(Bits); // creazione pulsanti abbinati
    return baseClass::dialogProperty(onCreate);
    }
  else
    return svmObjHistory::Change;
}
//-----------------------------------------------------------
#define MAX_CAM_BTN 9
//-----------------------------------------------------------
#define W_BTN 60
#define H_BTN 40
#define W_FIELD 80
#define H_FIELD 24
//-----------------------------------------------------------
void svmObjCam::makeBtn(DWORD Bits)
{
  svmObjButton* btn[MAX_CAM_BTN];
  PRect r(0, 0, W_BTN, H_BTN);
  r.MoveTo(Prop->Rect.left, Prop->Rect.bottom + 2);
  PWin* par = getParent();
  PRect rm;
  GetClientRect(*par, rm);
  if(r.bottom > rm.bottom)
    r.Offset(0, rm.bottom - r.bottom);
  svmMainClient* mc = dynamic_cast<svmMainClient*>(par);
  if(!mc)
    return;
  LPCTSTR released[] = {
    _T("Stop"),
    _T("Setup"),
    _T("Preview"),
    _T("Capture"),
    _T("Capture\\nfile"),
    _T("Start"),
    _T("Media\\nPlayer"),
    _T("Connect"),
    _T("Pause"),
    };
  LPCTSTR pressed[] = {
    _T("Stop"),
    _T("Setup"),
    _T("Preview"),
    _T("Capture"),
    _T("Capture\\nfile"),
    _T("Start"),
    _T("Media\\nPlayer"),
    _T("Disconnect"),
    _T("Pause"),
    };
  DWORD bits = Bits;
  for(uint i = 0; i < SIZE_A(btn); ++i, Bits >>= 1) {
    btn[i] = 0;
    if(Bits & 1) {
      btn[i] = new svmObjButton(mc, mc->getIncrementGlobalId(), r);
      if(!btn[i])
        mc->decrementGlobalId();
      else {
        r.Offset(W_BTN + 1, 0);
        btn[i]->makeDef4Cam(Prop->perif, Prop->addr, i, released[i], pressed[i]);
        mc->addNewObject(btn[i]);
        }
      }
    }
  r.bottom = r.top + H_FIELD;
  r.right = r.left + W_FIELD;
  r.MoveTo(Prop->Rect.left, Prop->Rect.top - r.Height() - 1);
  if(bits & (1 << MAX_CAM_BTN)) {
    svmObjEdit* ed = new svmObjEdit(mc, mc->getIncrementGlobalId(), r);
    if(!ed)
      mc->decrementGlobalId();
    else {
      r.Offset(W_FIELD + 1, 0);
      ed->makeDef4Cam(Prop->perif, Prop->addr + 1);
      mc->addNewObject(ed);
      }
    }
  if(bits & (1 << (MAX_CAM_BTN + 1))) {
    svmObjText* txt = new svmObjText(mc, mc->getIncrementGlobalId(), r);
    if(!txt)
      mc->decrementGlobalId();
    else {
//      r.Offset(W_FIELD + 1, 0);
      txt->makeDef4Cam(Prop->perif, Prop->addr + 3);
      mc->addNewObject(txt);
      }
    }
}
//-----------------------------------------------------------
bool svmObjCam::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  int id = ID_INIT_VAR_CAM;
  id += order;
  PropertyCam* pt = dynamic_cast<PropertyCam*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];

  svmObjCount& objCount = getObjCount();

  LPCTSTR txt = pt->getText();
//formato ->id,x,y,w,h,perif,addr,objName

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%s\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),
            Prop->perif,
            Prop->addr,
            txt);
  writeStringChkUnicode(pfCrypt, buff);

  if(Prop->type1) {
    wsprintf(buff, _T("%d,%d\r\n"), id + ADD_INIT_VAR, Prop->type1);
    writeStringChkUnicode(pfCrypt, buff);
    }
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjCam::load(uint id, setOfString& set)
{
  PropertyCam* pt = dynamic_cast<PropertyCam*>(Prop);
  if(!pt)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int prf = 0;
  int addr = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &prf, &addr
                    );


  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  Prop->perif = prf;
  Prop->addr = addr;

  p = findNextParam(p, 6);

  if(p) {
    TCHAR t[_MAX_PATH];
    translateFromCRNL(t, p);
    pt->setText(t);
    }
  id += ADD_INIT_VAR;
  p = set.getString(id);
  if(p)
    Prop->type1 = _ttoi(p);
  return true;
}


