//------------------- svmObjPanel.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjPanel.h"
#include "svmDefObj.h"
#include "svmPropertyPanel.h"
#include "p_File.h"
//-----------------------------------------------------------
svmObjPanel::svmObjPanel(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oPANEL, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjPanel::svmObjPanel(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oPANEL, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjPanel::init()
{
  Prop = new Property;
  Prop->background = GetSysColor(COLOR_3DSHADOW);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
//static Property staticPP;
//-----------------------------------------------------------
void svmObjPanel::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d, zOrder=%d"), getObjName(), getId(), zOrder);
  addTipsVis(tips, size);
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjPanel::dialogProperty(bool onCreate)
{
  static Property staticPP;
  if(onCreate) {
    if(staticPP.Rect.Width()) {
      staticPP.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      *Prop = staticPP;
      }
    else
      staticPP = *Prop;
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogPanel(this, Prop, getParent()).modal()) {
      if(PPanel::NO == Prop->style || PPanel::TRANSP == Prop->style)
        Prop->style = 1;
      staticPP = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void svmObjPanel::DrawObject(HDC hdc)
{
  PPanel panel(getRect(), Prop->background, (PPanel::bStyle)Prop->style);
  panel.setAllBorder(Prop->Border);
  panel.draw(hdc);
//  staticPP.Rect = getRect();
}
//-----------------------------------------------------------
PRect svmObjPanel::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjPanel::makeClone()
{
  svmObjPanel* obj = new svmObjPanel(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjPanel::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
namespace styleObjPanel {
enum stylePanel {
    up,
    dn,
    up_fill,
    dn_fill,
    border,
    border_fill,
    fill
    };
}
//-----------------------------------------------------------
uint svmObjPanel::getStyle() const
{
  int style = 0;
  if(Prop->style & Property::FILL) {
    if(Prop->style & Property::BORDER)
      style = styleObjPanel::border_fill;
    else if(Prop->style & Property::UP)
      style = styleObjPanel::up_fill;
    else if(Prop->style & Property::DN)
      style = styleObjPanel::dn_fill;
    else
      style = styleObjPanel::fill;
    }

  else {
    if(Prop->style & Property::BORDER)
      style = styleObjPanel::border;
    else if(Prop->style & Property::UP)
      style = styleObjPanel::up;
    else if(Prop->style & Property::DN)
      style = styleObjPanel::dn;
    }
  return style;
}
//-----------------------------------------------------------
bool svmObjPanel::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_SIMPLE_PANEL);
  uint id = moi.calcAndSetExtendId(order);

  TCHAR buff[1000];
  int style = getStyle();

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),

            style,
            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background));

  writeStringChkUnicode(pfCrypt, buff);

  Prop->saveBorder(buff, SIZE_A(buff));
  writeStringChkUnicode(pfCrypt, buff);
  writeStringChkUnicode(pfCrypt, _T("\r\n"));
//  if(Prop->visPerif)
    saveOnlyVisVar(moi.getSecondExtendId(), pfCrypt, Prop->type1);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjPanel::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &no_up_dn,
                    &Rbk, &Gbk, &Bbk);
  p = findNextParamTrim(p, 8);
  Prop->loadBorder(p);

  PPanel::bStyle type;
  switch(no_up_dn) {
    case 0:
      type = PPanel::UP;
      break;
    case 1:
      type = PPanel::DN;
      break;
    case 2:
      type = PPanel::UP_FILL;
      break;
    case 3:
      type = PPanel::DN_FILL;
      break;
    case 4:
      type = PPanel::BORDER;
      break;
    case 5:
      type = PPanel::BORDER_FILL;
      break;
    default:
      type = PPanel::FILL;
      break;
    }
  Prop->background = RGB(Rbk, Gbk, Bbk);
  Prop->style = type;

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  loadOnlyVisVar(id, set, &Prop->type1);

  return true;
}
