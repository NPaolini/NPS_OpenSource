//------------------- svmObjBitmap.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjBitmap.h"
#include "svmBitmapProperty.h"
#include "p_File.h"
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmObjBitmap::svmObjBitmap(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oPICTURE, x1, y1, x2, y2)
{
  init();
}
//-----------------------------------------------------------
svmObjBitmap::svmObjBitmap(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oPICTURE, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjBitmap::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
Property* svmObjBitmap::allocProperty()
{
  return new bitmapProperty;
}
//-----------------------------------------------------------
void svmObjBitmap::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d"), getObjName(), getId());
}
//-----------------------------------------------------------
bool svmObjBitmap::performDialogProperty()
{
  return IDOK == svmBitmapDialogProperty(this, Prop, getParent(), IDD_DIALOG_IMG_PROP).modal();
}
//-----------------------------------------------------------
void svmObjBitmap::DrawObject(HDC hdc)
{
  bitmapProperty* rp = dynamic_cast<bitmapProperty*>(Prop);
  if(!rp)
    return;

  dual* dbmp = getDualBmp();
  const PRect& r = getRect();

  LPCTSTR bmpName = 0;
  if(rp->idBmp && dbmp)
    bmpName = dbmp->getIt(rp->idBmp);

  if(bmpName) {
    dataProject& dp = getDataProject();
    TCHAR path[_MAX_PATH];
    dp.fullPathImg(path);
    appendPath(path, bmpName);
    PBitmap bmp(path);
    if(bmp.isValid()) {
      POINT pt = { r.left, r.top };
      SIZE sz = { r.Width(), r.Height() };
      bmp.draw(hdc, pt, sz);
      return;
      }
    }
  HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 0 ,0));
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  Rectangle(hdc, r.left, r.top, r.right, r.bottom);
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.right, r.bottom);
  MoveToEx(hdc, r.right, r.top, 0);
  LineTo(hdc, r.left, r.bottom);
  DeleteObject(SelectObject(hdc, oldPen));
}
//-----------------------------------------------------------
PRect svmObjBitmap::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjBitmap::makeClone()
{
  svmObjBitmap* obj = new svmObjBitmap(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjBitmap::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
bool svmObjBitmap::save(P_File& pf)
{
  dataProject& dp = getDataProject();
  bitmapProperty* rp = dynamic_cast<bitmapProperty*>(Prop);
  dual* dbmp = getDualBmp();
  LPCTSTR bmpName = dbmp->getIt(rp->idBmp);
  if(!bmpName)
    return false;
  TCHAR buff[512];
  const PRect& r = getRect();
  if(*dp.getimagePath())
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%s\\%s\r\n"),
            PRN_BMP, r.left, -r.top, r.Width(), -r.Height(), dp.getimagePath(), bmpName);
  else
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%s\r\n"),
            PRN_BMP, r.left, -r.top, r.Width(), -r.Height(), bmpName);

  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
LPCTSTR removePath(LPCTSTR row)
{
  LPCTSTR p = row + _tcslen(row) - 1;
  while(p > row) {
    if(_T('\\') == *p)
      return p + 1;
    --p;
    }
  return row;
}
//-----------------------------------------------------------
bool svmObjBitmap::load(LPCTSTR row)
{
  dataProject& dp = getDataProject();
  bitmapProperty* rp = dynamic_cast<bitmapProperty*>(Prop);

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  LPCTSTR nameBmp = 0;

  row = findNextParamTrim(row);
  _stscanf_s(row, _T("%d,%d,%d,%d"),  &x, &y, &w, &h);
  row = findNextParamTrim(row, 4);
  rp->idBmp = 0;
  if(row) {
    dual* dbmp = getDualBmp();
    rp->idBmp = dbmp->addAndGetId(removePath(row));
    }
  Prop->Rect = PRect(x, -y, x + w, -(y + h));

  return true;
}
