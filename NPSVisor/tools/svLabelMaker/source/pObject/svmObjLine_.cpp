//------------------- svmObjLine.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjLine.h"
#include "svmLineProperty.h"
#include "p_File.h"
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmObjLine::svmObjLine(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oLINE, x1, y1, x2, y2), A(0), K(0), isVert(false)
{
  init();
}
//-----------------------------------------------------------
svmObjLine::svmObjLine(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oLINE, r), A(0), K(0), isVert(false)
{
  init();
}
//-----------------------------------------------------------
void svmObjLine::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  makeEquation();
}
//-----------------------------------------------------------
Property* svmObjLine::allocProperty()
{
  return new lineProperty;
}
//-----------------------------------------------------------
void svmObjLine::makeEquation()
{
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  const PRect& r = Prop->Rect;
  if(r.left == r.right)
    isVert = true;
  else {
    int x1 = r.left;
    int x2 = r.right;
    int y1 = r.top;
    int y2 = r.bottom;
    lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
    if(lineProperty::edTopRight == rp->dir) {
      x1 = r.right;
      x2 = r.left;
      }
    A = y2 - y1;
    double tmp = x2 - x1;
    A /= tmp;
    K = y1 - A * x1;
    isVert = false;
    }
}
//-----------------------------------------------------------
// se definita usa i quattro punti per calcolare l'intersezione
// altrimenti, visto che i casi di verticale (più ostico) e orizzontale
// sono già risolti, controlla se il punto sulla retta, in uno dei quattro punti
// del rettangolo coincide col lato

//#define FULL_INTERSECT
//-----------------------------------------------------------
#ifdef FULL_INTERSECT
//-----------------------------------------------------------
struct triple
{
  double a;
  double b;
  double c;
  triple() : a(0), b(0), c(0) {}
  triple(const POINT& p1, const POINT& p2) { calc(p1, p2); }
  void calc(const POINT& p1, const POINT& p2)
  {
    a = p2.y - p1.y;
    b = -(p2.x - p1.x);
    c = -p1.x * a + p1.y * -b;
  }
};
//-----------------------------------------------------------
static bool calcIntersecLine(POINT& res, const triple& p1, const triple& p2)
{
  double denom = (p1.a * p2.b - p2.a * p1.b);
  if(denom != 0) {
    res.y = int((p2.a * p1.c - p1.a * p2.c) / denom);
    res.x = int((p1.b * p2.c - p2.b * p1.c) / denom);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
static bool calcIntersecLine(POINT& res, const triple& p1, const POINT& p1b, const POINT& p2b)
{
  triple p2(p1b, p2b);
  return calcIntersecLine(res, p1, p2);
}
//-----------------------------------------------------------
static bool calcIntersecLine(POINT& res, const POINT& p1a, const POINT& p2a, const POINT& p1b, const POINT& p2b)
{
  triple p1(p1a, p2a);
  triple p2(p1b, p2b);
  return calcIntersecLine(res, p1, p2);
}
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
static bool isInX(int x, const PRect& r)
{
  return x >= r.left && x <= r.right;
}
//-----------------------------------------------------------
static bool isInY(int y, const PRect& r)
{
  return y >= r.top && y <= r.bottom;
}
//-----------------------------------------------------------
#define RET_NOT_EQU(v)   if(r1.v != r2.v) return false
//-----------------------------------------------------------
static bool operator==(const PRect& r1, const PRect& r2)
{
  RET_NOT_EQU(left);
  RET_NOT_EQU(top);
  RET_NOT_EQU(right);
  RET_NOT_EQU(bottom);

  return true;
}
//-----------------------------------------------------------
#undef RET_NOT_EQU
//-----------------------------------------------------------
/*
//-----------------------------------------------------------
intersezione tra due rette usando l'equazione (non usabile con linee verticali)
y = Ax + K [y = Bx + C]
---------------------------
Ax - y + K = Bx - y + C
Ax - Bx = C - K

x = (C - K) / (A - B)
y = Ax + K
---------------------------
non è necessaria perché il confronto è con rette verticali e orizzontali
(i quattro lati del rettangolo) ed il calcolo va fatto in altro modo
//-----------------------------------------------------------
*/
//-----------------------------------------------------------
bool svmObjLine::intersec(const PRect& frame)
{
  if(!baseClass::intersec(frame))
    return false;

  if(isVert)
    return true;

  double tA = A > 0 ? A : -A;
  if(tA < 0.0001)
    return true;

  PRect rect(Prop->Rect.left, -Prop->Rect.top, Prop->Rect.right, -Prop->Rect.bottom);
  PRect r(frame.left, -frame.top, frame.right, -frame.bottom);
  do {
    PRect r2(r | rect);
    if(r == r2)
      return true;
    } while(false);

#ifndef FULL_INTERSECT

  int x = ROUND_REAL((-r.top - K) / A);
  if(isInX(x, r))
    return true;
  x = ROUND_REAL((-r.bottom - K) / A);
  if(isInX(x, r))
    return true;

  int y = -ROUND_REAL(r.left * A + K);
  if(isInY(y, r))
    return true;
  y = -ROUND_REAL(r.right * A + K);
  if(isInY(y, r))
    return true;

#else

  POINT res;
  POINT ptLineTop = { rect.left, rect.top };
  POINT ptLineBottom = { rect.right, rect.bottom };
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  if(lineProperty::edTopRight == rp->dir) {
    ptLineTop.x = rect.right;
    ptLineBottom.x = rect.left;
    }

  triple lineObj(ptLineTop, ptLineBottom);

  POINT pt1 = { r.left, r.top };
  POINT pt2 = { r.left, r.bottom };
  if(calcIntersecLine(res, lineObj, pt1, pt2))
    if(isInY(res.y, r))
      return true;

  pt2.x = r.right;
  pt2.y = r.top;
  if(calcIntersecLine(res, lineObj, pt1, pt2))
    if(isInX(res.x, r))
      return true;

  pt1.x = r.right;
  pt1.y = r.bottom;
  if(calcIntersecLine(res, lineObj, pt1, pt2))
    if(isInY(res.y, r))
      return true;

  pt2.x = r.left;
  pt2.y = r.bottom;
  if(calcIntersecLine(res, lineObj, pt1, pt2))
    if(isInX(res.x, r))
      return true;

#endif
  return false;
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObjLine::pointIn(const POINT& pt_) const
{
  POINT pt = { pt_.x, -pt_.y };
  PRect rect(Prop->Rect.left, -Prop->Rect.top, Prop->Rect.right, -Prop->Rect.bottom);

  PRect rt = getMinBox();
  rt.bottom = -rt.bottom;

  if(pt.x < (rect.left - rt.Width() / 2) || pt.x > (rect.right + rt.Width() / 2))
    return Extern;
  if(pt.y < (rect.top - rt.Height() / 2) || pt.y > (rect.bottom + rt.Height() / 2))
    return Extern;

  if(isVert) {

    rt.Inflate(2, 2);
    int w = rt.Width();
    int h = rect.Height();

    int x = rect.left - w / 2;

    int y1 = rect.top - rt.Height() / 2  - 1;
    int y2 = rect.top + h - rt.Height() / 2 - 1;

    rt.MoveTo(x, y1);
    if(rt.IsIn(pt))
      return LeftTop;

    rt.MoveTo(x, y2);
    if(rt.IsIn(pt))
      return RightBottom;

    return Inside;
    }
  double tA = A > 0 ? A : -A;
  if(tA > 1) {
    int x = ROUND_REAL((-pt.y - K) / A);
    if(abs(x - pt.x) > rt.Width() / 2)
      return Extern;
    }
  else {
    int y = -ROUND_REAL(pt.x * A + K);
    if(abs(y - pt.y) > rt.Height() / 2)
      return Extern;
    }
  rt.Inflate(2, 2);

  int x1 = rect.left;
  int x2 = rect.right;
  int y1 = rect.top;
  int y2 = rect.bottom;
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  if(lineProperty::edTopRight == rp->dir) {
    x2 = rect.left;
    x1 = rect.right;
    }
  y1 -=  rt.Height() / 2 + 1;
  y2 -=  rt.Height() / 2 + 1;

  x1 -=  rt.Width() / 2 - 1;
  x2 -=  rt.Width() / 2 - 1;

  rt.MoveTo(x1, y1);
  if(rt.IsIn(pt))
    return lineProperty::edTopRight == rp->dir ? RightTop : LeftTop;

  rt.MoveTo(x2, y2);
  if(rt.IsIn(pt))
    return lineProperty::edTopRight == rp->dir ? LeftBottom : RightBottom;

  return Inside;
}
//-----------------------------------------------------------
void svmObjLine::drawRectUI(HDC hdc, const PRect& rtAll)
{
  int x1 = rtAll.left;
  int x2 = rtAll.right;
  int y1 = rtAll.top;
  int y2 = rtAll.bottom ;
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  if(lineProperty::edTopRight == rp->dir) {
    x2 = rtAll.left;
    x1 = rtAll.right;
    }

  HPEN pen = CreatePen(PS_SOLID, 3, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y2);
  DeleteObject(SelectObject(hdc, oldPen));
}
//-----------------------------------------------------------
#define FRECT(hdc, r) Ellipse(hdc, r.left, r.top, r.right, r.bottom)
//#define FRECT(hdc, r) Rectangle(hdc, r.left, r.top, r.right, r.bottom)
//-----------------------------------------------------------
void svmObjLine::drawBoxes(HDC hdc, const PRect& all)
{
  PRect rt = getMinBox();
  int x1 = all.left;
  int x2 = all.right;

  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  if(lineProperty::edTopRight == rp->dir) {
    x1 = x2;
    x2 = all.left;
    }
  int y1 = all.top;
  int y2 = all.bottom;
  y1 -=  rt.Height() / 2 + 1;
  y2 -=  rt.Height() / 2 + 1;

  x1 -=  rt.Width() / 2 - 1;
  x2 -=  rt.Width() / 2 - 1;

  HBRUSH brush = CreateSolidBrush(getXoredColor());

  HGDIOBJ oldBrush = SelectObject(hdc, brush);

  rt.MoveTo(x1, y1);
  FRECT(hdc, rt);

  rt.MoveTo(x2, y2);
  FRECT(hdc, rt);

  SelectObject(hdc, oldBrush);
  DeleteObject(brush);
}
//-----------------------------------------------------------
void svmObjLine::reInsideRect(PRect& test, const PRect& area)
{
  baseClass::reInsideRect(test, area);
  makeEquation();
}
//-----------------------------------------------------------
void svmObjLine::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s, id=%d"), getObjName(), getId());
}
//-----------------------------------------------------------
bool svmObjLine::performDialogProperty()
{
  if(IDOK == svmLineDialogProperty(this, Prop, getParent(), IDD_DIALOG_LINE_PROP).modal()) {
    makeEquation();
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void svmObjLine::DrawObject(HDC hdc)
{
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
#if 0
  if(!rp || !rp->idPen)
    return;
#else
  if(!rp)
    return;
#endif
  HGDIOBJ oldPen = 0;
  if(rp->idPen) {
    dualPen* dp = getDualPen();
    HPEN pen = dp->getIt(rp->idPen)->hpen;
    if(pen)
      oldPen = SelectObject(hdc, pen);
    }
  const PRect& r = getRect();
  int x1 = r.left;
  int y1 = r.top;
  int x2 = r.right;
  int y2 = r.bottom;
  switch(rp->dir) {
    case lineProperty::edTopRight:
      x1 = x2;
      x2 = r.left;
      break;
    }
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y2);

  if(oldPen)
    SelectObject(hdc, oldPen);
}
//-----------------------------------------------------------
PRect svmObjLine::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
svmObject* svmObjLine::makeClone()
{
  svmObjLine* obj = new svmObjLine(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjLine::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
void svmObjLine::toggleDir()
{
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  if(rp->dir == lineProperty::edTopRight)
    rp->dir = lineProperty::edTopLeft;
  else
    rp->dir = lineProperty::edTopRight;
}
//-----------------------------------------------------------
bool svmObjLine::endDrag(HDC hdc, const POINT& pt)
{
  PRect old = getRect();
  // nessuna modifica
  if(!baseClass::endDrag(hdc, pt))
    return false;

  // se spostameno
  switch(whichPoint) {
    case Inside:
    case Extern:
      makeEquation();
      return true;
    }
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  const int x1 = RectDragging.left;
  const int y1 = RectDragging.top;
  const int x2 = RectDragging.right;
  const int y2 = RectDragging.bottom;
  if(x1 > x2) {
    if(y1 > y2)
      toggleDir();
    }
  else if(y1 < y2)
    toggleDir();
  makeEquation();
  return true;
}
//-----------------------------------------------------------
bool svmObjLine::save(P_File& pf)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);

  if(rp->idPen != state.idPen) {
    state.idPen = rp->idPen;
    savePen(pf, rp->idPen);
    }
  TCHAR buff[256];
  const PRect& r = getRect();
  int x1 = r.left;
  int y1 = r.top;
  int x2 = r.right;
  int y2 = r.bottom;
  switch(rp->dir) {
    case lineProperty::edTopRight:
      x1 = x2;
      x2 = r.left;
      break;
    }
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"),
            PRN_LINE, x1, -y1, x2, -y2);

  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
bool svmObjLine::load(LPCTSTR row)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);

  rp->idPen = state.idPen;

  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;

  _stscanf_s(row, _T("%d,%d,%d,%d,%d"),  &x1, &y1, &x2, &y2);
  if(x1 > x2) {
    if(y1 > y2) {
      int t = x1;
      x1 = x2;
      x2 = t;
      t = y1;
      y1 = y2;
      y2 = t;
      }
    else {
      int t = x1;
      x1 = x2;
      x2 = t;
      rp->dir = lineProperty::edTopRight;
      }
    }
  else if(y1 > y2) {
    int t = y1;
    y1 = y2;
    y2 = t;
    rp->dir = lineProperty::edTopRight;
    }
  rp->Rect = PRect(x1, -y1, x2, -y2);
  return true;
}
