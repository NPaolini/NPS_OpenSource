//-------------------- svmObjSpin.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
#include "pModDialog.h"
#include "svmObjSpin.h"
#include "svmDefObj.h"
#include "svmPropertySpin.h"
#include "PTraspBitmap.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
#define CHECK_ADD(a, s) ((DATA_NOT_USED == a ? 10 : a) + s)
//-----------------------------------------------------------
inline PRect makeRectCheck(int x, int y)
{
  if(DATA_NOT_USED == x)
    x = 10;
  if(DATA_NOT_USED == y)
    y = 10;
  return PRect(x, y, x + DEF_EDGE_BMP, y + DEF_EDGE_BMP);
}
//-----------------------------------------------------------
static Property staticPT;
//-----------------------------------------------------------
svmObjSpin::svmObjSpin(PWin* par, int id, int x, int y) :
  svmObject(par, id, oSPIN, makeRectCheck(x, y))
{
  init();
}
//-----------------------------------------------------------
svmObjSpin::svmObjSpin(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oSPIN, makeRectCheck(r.left, r.top))
{
  init();
}
//-----------------------------------------------------------
void svmObjSpin::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
const PRect& svmObjSpin::getRect() const
{
  Prop->Rect = makeRectCheck(Prop->Rect.left, Prop->Rect.top);
  return baseClass::getRect();
}
//-----------------------------------------------------------
void svmObjSpin::setRect(const PRect& rect)
{
  PRect r = makeRectCheck(rect.left, rect.top);
  baseClass::setRect(r);
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObjSpin::pointIn(const POINT& pt) const
{
// si è copiato qui il codice per evitare controlli inutili, altrimenti
// per un più corretto uso sarebbe dovuto essere: se il ritorno dalla classe
// base è diverso da Extern, tornare Inside, altrimenti Extern
  int x = pt.x;
  int y = pt.y;
  if(x < Prop->Rect.left || y < Prop->Rect.top)
    return Extern;
  if(x > Prop->Rect.right || y > Prop->Rect.bottom)
    return Extern;

  return Inside;
}
//----------------------------------------------------------------------------
void svmObjSpin::DrawObject(HDC hdc)
{
  PRect r = getRect();
  PBitmap bmp(ID_BMP_SPIN, getParent()->getHInstance());
  if(bmp.isValid()) {
    POINT pt = { r.left, r.top };
    PTraspBitmap(getParent(), &bmp, pt).Draw(hdc);
    }
  else {
    PPanel panel(r, RGB(0, 0, 200), PPanel::UP_FILL);
    panel.draw(hdc);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjSpin::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjSpin::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return getRect();
}
//-----------------------------------------------------------
svmObject* svmObjSpin::makeClone()
{
  svmObjSpin* obj = new svmObjSpin(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjSpin::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjSpin::allocProperty()
{
  Property* prp = new Property;
  prp->style = 0;
  prp->alignText = 0;
  prp->foreground = 0;
  return prp;
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjSpin::dialogProperty(bool onCreate)
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
    if(IDOK == svmDialogSpin(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
/*
  Si usano i dati già presenti nella classe base Property.
  style -> idSpin,
  type1 -> addrMem,
  type2 -> timeBeforeSend,
  idFont -> StepUp
  foreground -> StepDn
  alignText -> come flag per il valore memorizzato negli Step, 0 => intero, 1 => real
*/
//-----------------------------------------------------------
union U__
{
  int iv;
  float fv;
};
//-----------------------------------------------------------
bool svmObjSpin::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_SPIN);
  uint id = moi.calcAndSetExtendId(order);

  TCHAR buff[256];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,"),
            id,
            REV__X(Prop->Rect.left),
            REV__Y(Prop->Rect.top),
            Prop->style,
            Prop->type1,
            Prop->type2
            );
  LPTSTR p = buff + _tcslen(buff);
  U__ stepUp;
  U__ stepDn;

  stepUp.iv = Prop->idFont;
  stepDn.iv = Prop->foreground;

  if(Prop->alignText)
    _stprintf_s(p, SIZE_A(buff) - (p - buff), _T("%f,%f\r\n"), stepUp.fv, stepDn.fv);
  else
    wsprintf(p, _T("%d,%d\r\n"), stepUp.iv, stepDn.iv);

  writeStringChkUnicode(pfCrypt, buff);

  saveVar(pfCrypt, id);

  return baseClass::save(pfCrypt, pfClear, id);
}
//----------------------------------------------------------------------------
static bool isReal(LPCTSTR p)
{
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i)
    if(_T('.') == p[i])
      return true;
    else if(_T(',') == p[i])
      return false;
  return false;
}
//-----------------------------------------------------------
bool svmObjSpin::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;

  LPCTSTR p = set.getString(id);
  int nscan = _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &Prop->style, &Prop->type1, &Prop->type2);

  Prop->alignText = 0;
  p = findNextParamTrim(p, nscan);
  LPCTSTR p2 = findNextParamTrim(p);
  if(!p2)
    p2 = p;

  U__ stepUp;
  U__ stepDn;

  if(p && p2) {
    bool useReal = isReal(p) || isReal(p2);
    if(useReal) {
      stepUp.fv = (float)_tstof(p);
      stepDn.fv = (float)_tstof(p2);
      Prop->alignText = 1;
      }
    else {
      stepUp.iv = _ttoi(p);
      stepDn.iv = _ttoi(p2);
      }
    }
  Prop->idFont = stepUp.iv;
  Prop->foreground = stepDn.iv;

  Prop->Rect = PRect(0, 0, DEF_EDGE_BMP, DEF_EDGE_BMP);
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  loadVar(id, set);

  return true;
}


