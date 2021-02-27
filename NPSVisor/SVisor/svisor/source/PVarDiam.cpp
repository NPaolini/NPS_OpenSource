//-------- PVarDiam.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "PVarDiam.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "newnormal.h"
#include "p_util.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define STD_COLOR_0 (RGB(0xff, 0xff, 0xff))
#define STD_COLOR_1 (RGB(0x00, 0x00, 0x00))
#define STD_COLOR_2 (RGB(0xff, 0x00, 0x00))
#define STD_COLOR_3 (RGB(0xa0, 0xa0, 0xa0))
//----------------------------------------------------------------------------
#define STD_COLORS  { STD_COLOR_0, STD_COLOR_1, STD_COLOR_2, STD_COLOR_3 }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PVarDiam::PVarDiam(P_BaseBody* owner, uint id) : baseVar(owner, id),
  Panel(0), Type(0), currColor(0)
{}
//----------------------------------------------------------------------------
PVarDiam::~PVarDiam()
{
  delete Panel;
}
//----------------------------------------------------------------------------
bool PVarDiam::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int style1 = 0;
//  int style2 = 0;
  int idMin = 0;
  int idMax = 0;
  int idVal = 0;
  int idTypeColor = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &style1, &Type,
                    &idMin, &idMax, &idVal,
                    &idTypeColor);

  if(!(TRANSP_PIVOT & Type))
    Type &= ~INVERT;

  int typeBorder = style1 % 4;
  int typeBkg = style1 / 4;

  if(typeBorder)
    typeBorder = 1 << (typeBorder - 1);

  switch(typeBkg) {
    case 1:
      style1 = PPanel::NO;
      break;

    case 2:
      style1 = PPanel::FILL;
      break;

    default:
    case 0:
      style1 = 0;
      break;
    }
  style1 |= typeBorder;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));
  setRect(rect);
  makeOtherVar(vMax, idMax);
  makeOtherVar(vMin, idMin);
  makeOtherVar(vColor, idTypeColor);
  makeOtherVar(BaseVar, idVal);

  LPCTSTR p1 = getOwner()->getPageString(getSecondExtendId());
  allocVisibility(p1);

  if(PPanel::NO != style1)
    Panel = new PPanel(rect, 0, (PPanel::bStyle)style1);

  colors tmp;
  tmp.type[0] = STD_COLOR_0;
  tmp.type[1] = STD_COLOR_1;
  tmp.type[2] = STD_COLOR_2;
  tmp.type[3] = STD_COLOR_3;
  Colors[0] = tmp;

  p = getOwner()->getPageString(getFirstExtendId());
  if(vColor.getPrph()) {
    if(!p)
      vColor.setPrph(0);
    else {
      int nElem = 0;
      int idFirst = 0;
      _stscanf_s(p, _T("%d,%d"), &nElem, &idFirst);
      Colors.setDim(nElem);
      for(int i = 0; i < nElem; ++i) {
        p = getOwner()->getPageString(idFirst + i);
        if(!p)
          Colors[i] = tmp;
        else {
          colors c;
          loadColors(p, c);
          Colors[i] = c;
          }
        }
      }
    }
  else {
    if(p) {
      colors c;
      loadColors(p, c);
      Colors[0] = c;
      }
    }
  if(Panel)
    Panel->setBkgColor(Colors[currColor].type[cBKG]);
  return true;
}
//----------------------------------------------------------------------------
bool PVarDiam::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;

  force |= upd;
  if(!force && !isChanged())
    return upd;
  invalidate();
  return true;
}
//----------------------------------------------------------------------------
bool PVarDiam::isChanged()
{
  prfData data;
  if(prfData::isChanged == BaseVar.getData(getOwner(), data, getOffs()))
    return true;
  if(prfData::isChanged == vMin.getData(getOwner(), data, getOffs()))
    return true;
  if(prfData::isChanged == vMax.getData(getOwner(), data, getOffs()))
    return true;
  if(prfData::isChanged == vColor.getData(getOwner(), data, getOffs()))
    return true;
  return false;
}
//----------------------------------------------------------------------------
void PVarDiam::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vMin);
  addReqVar2(allBits, vMax);
  addReqVar2(allBits, vColor);
}
//----------------------------------------------------------------------------
void PVarDiam::performDraw(HDC hdc)
{
  setColors();
  if(Panel) {
    Panel->setRect(get_Rect());
    Panel->paint(hdc);
    }
  const PVect<colors>& vc = Colors;
  const colors& c = vc[currColor];

  REALDATA vals[MAX_DATA_DIAM];
  if(!loadVal(vals))
    return;

  bool transp = toBool(TRANSP_PIVOT & Type);
  if(transp)
    drawClipped(hdc, vals);
  else
    drawFilled(hdc, vals);

  if(DRAW_MAX & Type)
    drawMaxVal(hdc);

  if(AXIS & Type)
    drawAxis(hdc);
}
//----------------------------------------------------------------------------
void PVarDiam::setColors()
{
  if(!vColor.getPrph())
    return;

  prfData data;
  vColor.getData(getOwner(), data, getOffs());

  currColor = data.U.dw;
  if(currColor >= Colors.getElem())
    currColor = Colors.getElem() - 1;
  const PVect<colors>& vc = Colors;
  const colors& c = vc[currColor];
  if(Panel)
    Panel->setBkgColor(c.type[idColor]);
}
//----------------------------------------------------------------------------
void PVarDiam::loadColors(LPCTSTR p, colors& c)
{
  uint col[MAX_COLOR_TYPE * 3];
  memset(col, 0, sizeof(col));
  for(int i = 0; i < SIZE_A(col); ++i) {
    if(!p)
      break;
    col[i] = _ttoi(p);
    p = findNextParamTrim(p);
    }
  int j = 0;
  for(int i = 0; i < MAX_COLOR_TYPE; ++i, j += 3)
    c.type[i] = RGB(col[j], col[j + 1], col[j + 2]);
}
//----------------------------------------------------------------------------
static void squareRect(PRect& r)
{
  int width = r.Width();
  int height = r.Height();

  if(width > height) {
    r.left += (width - height) / 2;
    r.right = r.left + height;
    }
  else if(width < height) {
    r.top += (height - width) / 2;
    r.bottom = r.top + width;
    }
}
//----------------------------------------------------------------------------
#define DEF_INFLATE 2
//----------------------------------------------------------------------------
bool PVarDiam::loadVal(REALDATA* vals)
{
  prfData data;
  vMin.getData(getOwner(), data, getOffs());
  vals[idMin] = vMin.getNormalizedResult(data);

  vMax.getData(getOwner(), data, getOffs());
  vals[idMax] = vMax.getNormalizedResult(data);

  BaseVar.getData(getOwner(), data, getOffs());
  vals[idCurr] = BaseVar.getNormalizedResult(data);

  vColor.getData(getOwner(), data, getOffs());
  vals[idColor] = vColor.getNormalizedResult(data);

  if(vals[idCurr] > vals[idMax])
    vals[idCurr] = vals[idMax];
  if(vals[idCurr] < vals[idMin])
    vals[idCurr] = vals[idMin];

  if(/*!vals[idMin] ||*/ !vals[idMax])
    return true;

  minRect = get_Rect();
  squareRect(minRect);

  REALDATA diff = (vals[idMax] - vals[idMin]) / 2 / vals[idMax];
  int dim = (int)(minRect.Width() * diff + DEF_INFLATE);
  if(dim < 0 || !(INVERT & Type))
    minRect.Inflate(-DEF_INFLATE, -DEF_INFLATE);
  else
    minRect.Inflate(-dim, -dim);
  return true;
}
//----------------------------------------------------------------------------
PRect PVarDiam::getCurrRect(REALDATA* vals, PRect& rMax)
{
  rMax = get_Rect();
  rMax.Inflate(-DEF_INFLATE, -DEF_INFLATE);
  squareRect(rMax);
  int width = rMax.Width();

  int curr = (int)(vals[idCurr] * width / vals[idMax]);

  int delta = (width - curr) / 2;
  PRect rCurr(rMax);
  rCurr.Inflate(-delta, - delta);

  return rCurr;
}
//----------------------------------------------------------------------------
void PVarDiam::drawClipped(HDC hdc, REALDATA* vals)
{
  if(vals[idMax] <= 0)
    return;

  PRect r;
  PRect rCurr = getCurrRect(vals, r);

  int width = r.Width();
  int pivot = (int)(vals[idMin] * width / vals[idMax]);

  int delta = (width - pivot) / 2;
  PRect rPivot(r);
  rPivot.Inflate(-delta, - delta);

  if(INVERT & Type) {
    rPivot = rCurr;
    rCurr = r;
    }

  HRGN hrgn1 = CreateEllipticRgn(rPivot.left, rPivot.top, rPivot.right, rPivot.bottom);
  ExtSelectClipRgn(hdc, hrgn1, RGN_DIFF);

  HBRUSH brushCurr = CreateSolidBrush(Colors[currColor].type[cCROWN]);
  HPEN penCurr = CreatePen(PS_SOLID, 1, Colors[currColor].type[cCROWN]);

  HGDIOBJ oldBrush = SelectObject(hdc, brushCurr);
  HGDIOBJ oldPen = SelectObject(hdc, penCurr);

  Ellipse(hdc, rCurr.left, rCurr.top, rCurr.right, rCurr.bottom);

  ExtSelectClipRgn(hdc, hrgn1, RGN_OR);
  DeleteObject(hrgn1);

  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);

  DeleteObject(penCurr);
  DeleteObject(brushCurr);
}
//----------------------------------------------------------------------------
void PVarDiam::drawFilled(HDC hdc, REALDATA* vals)
{
  if(vals[idMax] <= 0)
    return;
  PRect r;
  PRect rCurr = getCurrRect(vals, r);

  HBRUSH brushCurr = CreateSolidBrush(Colors[currColor].type[cCROWN]);
  HPEN penCurr = CreatePen(PS_SOLID, 1, Colors[currColor].type[cCROWN]);

  HGDIOBJ oldBrush = SelectObject(hdc, brushCurr);
  HGDIOBJ oldPen = SelectObject(hdc, penCurr);

  Ellipse(hdc, rCurr.left, rCurr.top, rCurr.right, rCurr.bottom);

  int width = r.Width();
  int pivot = (int)(vals[idMin] * width / vals[idMax]);

  int delta = (width - pivot) / 2;
  PRect rPivot(r);
  rPivot.Inflate(-delta, - delta);

  HBRUSH brushPivot = CreateSolidBrush(Colors[currColor].type[cPIVOT]);
  HPEN penPivot = CreatePen(PS_SOLID, 1, Colors[currColor].type[cPIVOT]);

  SelectObject(hdc, brushPivot);
  SelectObject(hdc, penPivot);

  Ellipse(hdc, rPivot.left, rPivot.top, rPivot.right, rPivot.bottom);

  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);

  DeleteObject(penPivot);
  DeleteObject(brushPivot);
  DeleteObject(penCurr);
  DeleteObject(brushCurr);
}
//----------------------------------------------------------------------------
void PVarDiam::drawMaxVal(HDC hdc)
{
  HPEN penMax = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff)); //Colors[currColor].type[cCROWN]);
  uint oldMode = SetBkMode(hdc, OPAQUE);
  COLORREF oldColor = SetBkColor(hdc, RGB(0, 0, 0));
  HGDIOBJ oldPen = SelectObject(hdc, penMax);
  Arc(hdc, minRect.left, minRect.top, minRect.right, minRect.bottom, 0, 0, 0, 0);
  SetBkColor(hdc, oldColor);
  SetBkMode(hdc, oldMode);
  SelectObject(hdc, oldPen);
  DeleteObject(penMax);
}
//----------------------------------------------------------------------------
void PVarDiam::drawAxis(HDC hdc)
{
  PRect r = get_Rect();
  r.Inflate(-DEF_INFLATE, -DEF_INFLATE);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  HPEN penAxis = CreatePen(PS_DASHDOT, 1, Colors[currColor].type[cAXIS]);
  HGDIOBJ oldPen = SelectObject(hdc, penAxis);

  int width = r.Width();
  MoveToEx(hdc, r.left + width / 2, r.top, 0);
  LineTo(hdc, r.left + width / 2, r.bottom);

  int height = r.Height();
  MoveToEx(hdc, r.left, r.top + height / 2, 0);
  LineTo(hdc, r.right, r.top + height / 2);

  SelectObject(hdc, oldPen);
  SetBkMode(hdc, oldMode);

  DeleteObject(penAxis);
}
