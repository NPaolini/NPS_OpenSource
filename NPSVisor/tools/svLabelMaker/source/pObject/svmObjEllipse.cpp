//------------------- svmObjEllipse.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjEllipse.h"
#include "svmDrawProperty.h"
#include "p_File.h"
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmObjEllipse::svmObjEllipse(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oELLIPSE, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjEllipse::svmObjEllipse(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oELLIPSE, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjEllipse::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
Property* svmObjEllipse::allocProperty()
{
  return new drawProperty;
}
//-----------------------------------------------------------
void svmObjEllipse::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d"), getObjName(), getId());
}
//-----------------------------------------------------------
void svmObjEllipse::setCommonProperty(const unionProperty& prop)
{
  baseClass::setCommonProperty(prop);
  drawProperty* rp = dynamic_cast<drawProperty*>(Prop);
  if(PROP_BIT_POS(eupPen) & prop.flagBits)
    rp->idPen = prop.idPen;
  if(PROP_BIT_POS(eupBrush) & prop.flagBits)
    rp->idBrush = prop.idBrush;
}
//-----------------------------------------------------------
bool svmObjEllipse::performDialogProperty()
{
  return IDOK == svmDrawDialogProperty(this, Prop, getParent(), IDD_DIALOG_DRAW_PROP).modal();
}
//-----------------------------------------------------------
void svmObjEllipse::DrawObject(HDC hdc)
{
  drawProperty* rp = dynamic_cast<drawProperty*>(Prop);
#if 0
  if(!rp || !rp->idPen && !rp->idBrush)
    return;
#else
  if(!rp)
    return;
#endif
  HGDIOBJ oldPen = 0;
  HGDIOBJ oldBrush = 0;
  if(rp->idPen) {
    dualPen* dp = getDualPen();
    HPEN pen = dp->getIt(rp->idPen)->hpen;
    if(pen)
      oldPen = SelectObject(hdc, pen);
    }
  if(rp->idBrush) {
    dualBrush* db = getDualBrush();
    HBRUSH brush = db->getIt(rp->idBrush)->hbrush;
    if(brush)
      oldBrush = SelectObject(hdc, brush);
    }

  const PRect& r = getRect();

  if(rp->idBrush > 1)
    Ellipse(hdc, r.left, r.top, r.right, r.bottom);
  else
    Arc(hdc, r.left, r.top, r.right, r.bottom, 0, 0, 0, 0);

  if(oldPen)
    SelectObject(hdc, oldPen);
  if(oldBrush)
    SelectObject(hdc, oldBrush);
}
//-----------------------------------------------------------
PRect svmObjEllipse::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjEllipse::makeClone()
{
  svmObjEllipse* obj = new svmObjEllipse(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjEllipse::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
bool svmObjEllipse::save(P_File& pf)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  drawProperty* rp = dynamic_cast<drawProperty*>(Prop);

  state.savePen(pf, rp->idPen);
  state.saveBrush(pf, rp->idBrush);

  TCHAR buff[256];
  const PRect& r = getRect();
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"),
            PRN_OVAL, r.left, -r.top, r.Width(), -r.Height(), rp->idBrush > 1);

  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
bool svmObjEllipse::load(LPCTSTR row)
{
  row = findNextParamTrim(row);
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  drawProperty* rp = dynamic_cast<drawProperty*>(Prop);

  rp->idPen = state.idPen;
  rp->idBrush = state.idBrush;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int filled = 0;

  _stscanf_s(row, _T("%d,%d,%d,%d,%d"),  &x, &y, &w, &h, &filled);
  if(!filled)
    rp->idBrush = 0;
  rp->Rect = PRect(x, -y, x + w, -(y + h));
  return true;
}
