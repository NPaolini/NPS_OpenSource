//------------------- svmObjRect.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjRect.h"
#include "svmRectProperty.h"
#include "p_File.h"
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmObjRect::svmObjRect(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oRECT, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjRect::svmObjRect(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oRECT, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjRect::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
Property* svmObjRect::allocProperty()
{
  return new rectProperty;
}
//-----------------------------------------------------------
void svmObjRect::setCommonProperty(const unionProperty& prop)
{
  baseClass::setCommonProperty(prop);
  rectProperty* rp = dynamic_cast<rectProperty*>(Prop);
  if(PROP_BIT_POS(eupCX) & prop.flagBits)
    rp->cX = prop.cX;
  if(PROP_BIT_POS(eupCY) & prop.flagBits)
    rp->cY = prop.cY;
  if(PROP_BIT_POS(eupPen) & prop.flagBits)
    rp->idPen = prop.idPen;
  if(PROP_BIT_POS(eupBrush) & prop.flagBits)
    rp->idBrush = prop.idBrush;
}
//-----------------------------------------------------------
void svmObjRect::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d"), getObjName(), getId());
}
//-----------------------------------------------------------
bool svmObjRect::performDialogProperty()
{
  return IDOK == svmRectDialogProperty(this, Prop, getParent()).modal();
}
//-----------------------------------------------------------
void svmObjRect::DrawObject(HDC hdc)
{
  rectProperty* rp = dynamic_cast<rectProperty*>(Prop);
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
  if(rp->cX || rp->cY)
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, rp->cX, rp->cY);
  else
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
  if(oldPen)
    SelectObject(hdc, oldPen);
  if(oldBrush)
    SelectObject(hdc, oldBrush);
}
//-----------------------------------------------------------
PRect svmObjRect::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjRect::makeClone()
{
  svmObjRect* obj = new svmObjRect(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjRect::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
bool svmObjRect::save(P_File& pf)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  rectProperty* rp = dynamic_cast<rectProperty*>(Prop);

  state.savePen(pf, rp->idPen);
  state.saveBrush(pf, rp->idBrush);

  TCHAR buff[256];
  const PRect& r = getRect();
  if(rp->cX || rp->cY)
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"),
            PRN_ROUND_RECT, r.left, -r.top, r.Width(), -r.Height(), rp->cX, rp->cY);
  else
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"),
            PRN_RECT, r.left, -r.top, r.Width(), -r.Height());

  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
bool svmObjRect::load(LPCTSTR row)
{
  row = findNextParamTrim(row);
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  rectProperty* rp = dynamic_cast<rectProperty*>(Prop);

  rp->idPen = state.idPen;
  rp->idBrush = state.idBrush;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int cx = 0;
  int cy = 0;

  _stscanf_s(row, _T("%d,%d,%d,%d,%d,%d"),  &x, &y, &w, &h, &cx, &cy);
  rp->Rect = PRect(x, -y, x + w, -(y + h));
  rp->cX = cx;
  rp->cY = cy;
  return true;
}
