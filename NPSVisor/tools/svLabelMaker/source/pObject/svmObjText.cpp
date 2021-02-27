//-------------------- svmObjText.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
#include "pTextPanel.h"
#include "pModDialog.h"
#include "svmObjText.h"
#include "svmDefObj.h"
#include "svmTextProperty.h"
#include "lineEquation.h"
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 80
//-----------------------------------------------------------
//-----------------------------------------------------------
static textProperty staticPT;
//-----------------------------------------------------------
svmObjText::svmObjText(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oTEXT, x1, y1, x2, DATA_NOT_USED == y2 ? -DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjText::svmObjText(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oTEXT, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjText::init()
{
  Prop = new textProperty;
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  po->setText(_T("Txt"));
  Prop->Rect = getFirstRect();
  makeTrueRect();
}
//-----------------------------------------------------------
void svmObjText::resolveFont(const typeFont* tf)
{
  baseClass::resolveFont(tf);
  makeTrueRect();
}
//-----------------------------------------------------------
void svmObjText::DrawObject(HDC hdc)
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  svmFont& f = isLinked() ? getFontObjLinked() : getFontObj();

  int angle = (po->textType & textProperty::eboxed) ? 0 : po->Angle % 3600;
  HFONT hf = f.getFont2(po->idFont, angle);

  uint align = ((po->textType & textProperty::eboxed) || 0 == po->hAlign) ? TA_LEFT :
              (1 == po->hAlign) ? TA_CENTER : TA_RIGHT;
    align |= TA_TOP;

  uint oldAlign = SetTextAlign(hdc, align);
  COLORREF oldColor = SetTextColor(hdc, po->foreground);
//  COLORREF oldBkgColor = SetBkColor(hdc, po->background);
  uint oldTransp = SetBkMode(hdc, TRANSPARENT);
  HGDIOBJ oldFont = SelectObject(hdc, hf);

  uint d_align = DT_WORDBREAK | DT_EXPANDTABS | (!(po->textType & textProperty::eMultiline) ? DT_SINGLELINE : 0);
  align = (0 == po->hAlign) ? DT_LEFT :
              (1 == po->hAlign) ? DT_CENTER : DT_RIGHT;
  d_align |= align;

  align = ((po->textType & textProperty::eMultiline) || 0 == po->vAlign) ? DT_TOP :
              (1 == po->vAlign) ? DT_VCENTER : DT_BOTTOM;
  DT_TOP;
  d_align |= align;
/*
  if(cTRANSP != po->background)
    ExtTextOut(hdc, Prop->Rect.left, Prop->Rect.top, ETO_OPAQUE, Prop->Rect, 0, 0, 0);
*/
  TCHAR text[4096];
  po->getVarText(text, SIZE_A(text));

  if(angle || !(po->textType & textProperty::eboxed))
    TextOut(hdc, po->Rect.left, po->Rect.top, text, _tcslen(text));
  else
    DrawText(hdc, text, -1, po->Rect, d_align);
  SetTextAlign(hdc, oldAlign);
  SetTextColor(hdc, oldColor);
//  SetBkColor(hdc, oldBkgColor);
  SetBkMode(hdc, oldTransp);
  DeleteObject(SelectObject(hdc, oldFont));
}
//-----------------------------------------------------------
void svmObjText::fillTips(LPTSTR tips, int size)
{
  textProperty* pt = dynamic_cast<textProperty*>(Prop);
  if(textProperty::eVariable == (pt->textType & textProperty::eVariable)) {
    wsprintf(tips, _T("Obj -> %s, id=%d, var=%d"), getObjName(), getId(), pt->idVar);
    }
  else
    baseClass::fillTips(tips, size);
}
//-----------------------------------------------------------
smartPointerConstString svmObjText::getTitle() const
{
  return smartPointerConstString(_T("Text"), 0);
}
//-----------------------------------------------------------
Property* svmObjText::allocProperty() { return new textProperty; }
//-----------------------------------------------------------
void svmObjText::setCommonProperty(const unionProperty& prop)
{
  baseClass::setCommonProperty(prop);
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(PROP_BIT_POS(eupFG) & prop.flagBits)
    po->foreground = prop.foreground;
  if(PROP_BIT_POS(eupFont) & prop.flagBits)
    po->idFont = prop.idFont;
  if(PROP_BIT_POS(eupHAlign) & prop.flagBits)
    po->hAlign = prop.hAlign;
  if(PROP_BIT_POS(eupVAlign) & prop.flagBits)
    po->vAlign = prop.vAlign;

  if(BASE_BIT_POS & prop.flagBits)
    makeTrueRect();
}
//-----------------------------------------------------------
bool svmObjText::setOffsetVar(int offs)
{
  textProperty* pt = dynamic_cast<textProperty*>(Prop);
  if(textProperty::eVariable == (pt->textType & textProperty::eVariable)) {
    pt->idVar += offs;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void svmObjText::setRect(const PRect& rect)
{
  baseClass::setRect(rect);
  makeTrueRect();
}
//-----------------------------------------------------------
PRect svmObjText::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return Prop->Rect;
}
//-----------------------------------------------------------
svmObject* svmObjText::makeClone()
{
  svmObjText* obj = new svmObjText(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  obj->makeTrueRect();
  return obj;
}
//-----------------------------------------------------------
void svmObjText::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
  makeTrueRect();
}
//-----------------------------------------------------------
void svmObjText::restoreByHistoryObject(const svmObjHistory& history)
{
  baseClass::restoreByHistoryObject(history);
  makeTrueRect();
}
//-----------------------------------------------------------
void svmObjText::copyPropFrom(const Property* other)
{
  *Prop = *other;
  makeTrueRect();
}
//-----------------------------------------------------------
bool svmObjText::performDialogProperty()
{
  if(IDOK == svmTextDialogProperty(this, Prop, getParent(), IDD_DIALOG_TEXT_PROP).modal()) {
    makeTrueRect();
    return true;
    }
  return false;
}
//-----------------------------------------------------------
static bool isInX(int x, const PRect& r)
{
  return x >= r.left && x <= r.right;
}
//-----------------------------------------------------------
static bool isInY(int y, const PRect& r)
{
  return y <= r.top && y >= r.bottom;
}
//-----------------------------------------------------------
static bool checkVert(const POINT& pt1, const POINT& pt2, const PRect& r)
{
  if(pt1.x < r.left || pt1.x > r.right)
    return false;
  int y1 = min(-pt1.y, -pt2.y);
  int y2 = max(-pt1.y, -pt2.y);
  if(y1 > -r.bottom || y2 < -r.top)
    return false;
  return true;
}
//-----------------------------------------------------------
static bool checkHorz(const POINT& pt1, const POINT& pt2, const PRect& r)
{
  if(pt1.y > r.top || pt1.y < r.bottom)
    return false;
  int x1 = min(pt1.x, pt2.x);
  int x2 = max(pt1.x, pt2.x);
  if(x1 > r.right || x2 < r.left)
    return false;
  return true;
}
//-----------------------------------------------------------
bool svmObjText::intersec(const PRect& frame)
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(po->textType & textProperty::eboxed)
    return baseClass::intersec(frame);
  PRect rect = trueRect;
  POINT pt[5];
  calcRectUI(pt, rect);
  do {
    int x1 = pt[0].x;
    int x2 = x1;
    int y1 = pt[0].y;
    int y2 = y1;
    for(uint i = 1; i < SIZE_A(pt) - 1; ++i) {
      if(pt[i].x < x1)
        x1 = pt[i].x;
      if(pt[i].x > x2)
        x2 = pt[i].x;
      if(pt[i].y > y1)
        y1 = pt[i].y;
      if(pt[i].y < y2)
        y2 = pt[i].y;
      }
    rect = PRect(x1, y1, x2, y2);
    } while(false);
  normalizeYNeg(rect);
  if(!intersecYNeg(rect, frame))
    return false;
  rect.left = max(rect.left, frame.left);
  rect.right = min(rect.right, frame.right);
  rect.top = min(rect.top, frame.top);
  rect.bottom = max(rect.bottom, frame.bottom);

  uint found = 0;
  for(uint i = 0; i < 4; ++i) {
    lineEquation eq(pt[i], pt[i + 1]);
    if(eq.isVert()) {
      if(checkVert(pt[i], pt[i + 1], frame))
        return true;
      }
    else {
      int v = eq.getY(frame.left);
      if(isInY(v, rect))
        if(++found > 1)
          return true;
      v = eq.getY(frame.right);
      if(isInY(v, rect))
        if(++found > 1)
          return true;
      if(eq.isHorz()) {
        if(checkHorz(pt[i], pt[i + 1], frame))
          return true;
        }
      else {
        v = eq.getX(frame.top);
        if(isInX(v, rect))
          if(++found > 1)
            return true;
        v = eq.getX(frame.bottom);
        if(isInX(v, rect))
          if(++found > 1)
            return true;
        }
      }
    }
  return false;
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObjText::pointIn(const POINT& pt_) const
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(po->textType & textProperty::eboxed)
    return baseClass::pointIn(pt_);

  PRect r = trueRect;
  POINT pt[5];
  calcRectUI(pt, r);
  lineEquation::eSide side[4];
  do {
    lineEquation eq(pt[0], pt[1]);
    if(eq.isVert() || eq.isHorz()) {
      PRect r(pt[0].x, pt[0].y, pt[2].x, pt[2].y);
      normalizeYNeg(r);
      return isInYNeg(r, pt_) ? Inside : Extern;
      }
    side[0] = eq.whereIs(pt_);
    if(lineEquation::eInside == side[0])
      return Inside;
    } while(false);
  do {
    lineEquation eq(pt[1], pt[2]);
    side[1] = eq.whereIs(pt_);
    if(lineEquation::eInside == side[1])
      return Inside;
    } while(false);
  do {
    lineEquation eq(pt[2], pt[3]);
    side[2] = eq.whereIs(pt_);
    if(lineEquation::eInside == side[2])
      return Inside;
    } while(false);
  do {
    lineEquation eq(pt[3], pt[4]);
    side[3] = eq.whereIs(pt_);
    if(lineEquation::eInside == side[3])
      return Inside;
    } while(false);
  if(side[0] == side[2] || side[1] == side[3])
    return Extern;
  return Inside;
}
//-----------------------------------------------------------
#define _USE_MATH_DEFINES
#include <math.h>
//-----------------------------------------------------------
static POINT calcXY(const POINT& pt0, const POINT& ptOffs, int x, int y, double angle)
{
  POINT pt;
  x -= pt0.x;
  y -= pt0.y;
  pt.x = ROUND_REAL(x * cos(angle) - y * sin(angle)) + pt0.x + ptOffs.x;
  pt.y = ROUND_REAL(x * sin(angle) + y * cos(angle)) + pt0.y + ptOffs.y;
  return pt;
}
//-----------------------------------------------------------
#define ROT(x, y) calcXY(ptCenter, ptOffs, x, y, rad)
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmObjText::calcRectUI(POINT* pPoints, const PRect& rect) const
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  PRect r = trueRect;
  r.MoveTo(rect.left, rect.top);
  POINT ptCenter = { r.left, r.top };
  if((textProperty::taCenter & po->hAlign) == textProperty::taCenter)
    ptCenter.x += r.Width() / 2;
  else if((textProperty::taRight & po->hAlign) == textProperty::taRight)
    ptCenter.x += r.Width();
  int angle = (po->textType & textProperty::eboxed) ? 0 : po->Angle % 3600;

  POINT ptOffs = { r.left - ptCenter.x, r.top - ptCenter.y };
  int x1 = r.left;
  int y1 = r.top;
  int x2 = r.right;
  int y2 = r.bottom;
  if(angle < 0)
    angle = 3600 + angle;
  double rad = ((3600 - angle) / 1800.0) * M_PI;

  pPoints[0] = ROT(x1, y1);
  pPoints[1] = ROT(x2, y1);
  pPoints[2] = ROT(x2, y2);
  pPoints[3] = ROT(x1, y2);
  pPoints[4] =  pPoints[0];
  return true;
}
//-----------------------------------------------------------
void svmObjText::drawRectUI(HDC hdc, const PRect& rtAll)
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(po->textType & textProperty::eboxed) {
    baseClass::drawRectUI(hdc, rtAll);
    return;
    }
  PRect r = rtAll;
  POINT pt[5];
  if(!calcRectUI(pt, r)) {
    baseClass::drawRectUI(hdc, r);
    return;
    }

  HPEN pen = CreatePen(PS_SOLID, 2, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  Polyline(hdc, pt, SIZE_A(pt));
  DeleteObject(SelectObject(hdc, oldPen));
}
//-----------------------------------------------------------
void svmObjText::drawBoxes(HDC hdc, const PRect& all)
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(!(po->textType & textProperty::eboxed))
    return;
  baseClass::drawBoxes(hdc, all);
}
//-----------------------------------------------------------
void svmObjText::reInsideRect(PRect& test, const PRect& area)
{
  baseClass::reInsideRect(test, area);
  makeTrueRect();
}
//-----------------------------------------------------------
bool svmObjText::endDrag(HDC hdc, const POINT& pt)
{
  if(!baseClass::endDrag(hdc, pt))
    return false;

  makeTrueRect();
  return true;
}
//-----------------------------------------------------------
static uint getTA_Align(uint a)
{
  uint align = TA_TOP;
  switch(a) {
    case 0:
      align |= TA_LEFT;
      break;
    case 1:
      align |= TA_CENTER;
      break;
    case 2:
      align |= TA_RIGHT;
      break;
    }
  return align;
}
//-----------------------------------------------------------
static uint getDT_Align(uint h, uint v, bool multi)
{
  uint align = 0;
  switch(h) {
    case 0:
      align = DT_LEFT;
      break;
    case 1:
      align = DT_CENTER;
      break;
    case 2:
      align = DT_RIGHT;
      break;
    }
  if(multi)
    align |= DT_TOP;
  else {
    switch(v) {
      case 0:
        align |= DT_TOP;
        break;
      case 1:
        align |= DT_VCENTER;
        break;
      case 2:
        align |= DT_BOTTOM;
        break;
      }
    align |= DT_SINGLELINE;
    }
  return align;
}
//-----------------------------------------------------------
void svmObjText::makeTrueRect()
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(po->textType & textProperty::eboxed)
    return;

  HDC hdc = GetDC(*getParent());

  svmFont& f = isLinked() ? getFontObjLinked() : getFontObj();

  int angle = (po->textType & textProperty::eboxed) ? 0 : po->Angle % 3600;
  HFONT hf = f.getFont3(po->idFont, angle);
  uint align = getTA_Align(po->hAlign);

  uint oldAlign = SetTextAlign(hdc, align);
  HGDIOBJ oldFont = SelectObject(hdc, hf);

  trueRect = getRect();
  TCHAR text[4096] = { 0 };
  po->getVarText(text, SIZE_A(text));
  SIZE sz;
  uint len = _tcslen(text);
  GetTextExtentPoint32(hdc, text, len, &sz);
  trueRect.right = trueRect.left + sz.cx + len;
  trueRect.bottom = trueRect.top - sz.cy;
  SetTextAlign(hdc, oldAlign);
  DeleteObject(SelectObject(hdc, oldFont));
//  SetMapMode(hdc, oldMap);
  ReleaseDC(*getParent(), hdc);
}
//-----------------------------------------------------------
const PRect& svmObjText::getLinkedRect() const
{
  textProperty* po = dynamic_cast<textProperty*>(Prop);
  if(po->textType & textProperty::eboxed)
    return trueRect;
  POINT pt[5];
  if(!calcRectUI(pt, trueRect))
    return trueRect;

  static PRect r;
  r.left = pt[0].x - 1;
  r.top = pt[0].y;
  r.right = pt[2].x + 1;
  r.bottom = pt[2].y;
/*
  r = trueRect;
  switch(po->hAlign) {
    case 1:
      r.Offset(-r.Width() / 2, 0);
      break;
    case 2:
      r.Offset(-r.Width(), 0);
      break;
    }
*/
  return r;
}
//-----------------------------------------------------------
/*
      PRN_TXT = 1,      // X,Y,testo
      PRN_TXT_ON_BOX,    // X,Y,W,H,align,testo

testo con valore da variabile
51,X,Y,variabile
-----------------------------------------------------------
testo all'interno del rettangolo con valore da variabile
52,X,Y,W,H,align,variabile
*/
//-----------------------------------------------------------
bool svmObjText::save(P_File& pf)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  textProperty* rp = dynamic_cast<textProperty*>(Prop);

  state.saveForeground(pf, rp->foreground);
  state.saveFont(pf, rp->idFont, rp->Angle);
  state.saveBkMode(pf, TRANSPARENT);

  TCHAR buff[256];
  const PRect& r = getRect();

  if(rp->textType & textProperty::eboxed) {
    state.saveAlign(pf, TA_LEFT | TA_TOP);
    uint align = getDT_Align(rp->hAlign, rp->vAlign, toBool(rp->textType & textProperty::eMultiline));
    if(rp->textType & textProperty::eVariable) {
      wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"),
            PRN_TXT_ON_BOX_VAR, r.left, -r.top, r.Width(), -r.Height(), align, rp->idVar - 1);
      return writeStringChkUnicode(pf, buff) > 0;
      }
    else {
      wsprintf(buff, _T("%d,%d,%d,%d,%d,"), PRN_TXT_ON_BOX, r.left, -r.top, r.Width(), -r.Height());
      if(writeStringChkUnicode(pf, buff))
        if(writeStringChkUnicode(pf, rp->Text))
          return writeStringChkUnicode(pf, _T("\r\n")) > 0;
      return false;
      }
    }

  uint align = getTA_Align(rp->hAlign);
  state.saveAlign(pf, align);
  if(rp->textType & textProperty::eVariable) {
    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), PRN_TXT_VAR, r.left, -r.top, rp->idVar - 1);
    return writeStringChkUnicode(pf, buff) > 0;
    }
  else {
    wsprintf(buff, _T("%d,%d,%d,"), PRN_TXT, r.left, -r.top);
    if(writeStringChkUnicode(pf, buff))
      if(writeStringChkUnicode(pf, rp->Text))
        return writeStringChkUnicode(pf, _T("\r\n")) > 0;
    }
  return false;
}
//-----------------------------------------------------------
static bool getDim(LPCTSTR& row, PRect& r)
{
  r.right = r.left + _ttoi(row);
  row = findNextParamTrim(row);
  if(!row)
    return false;
  r.bottom = r.top -_ttoi(row);
  row = findNextParamTrim(row);
  if(!row)
    return false;
  return true;
}
//-----------------------------------------------------------
void svmObjText::setAlign(int align)
{
  textProperty* rp = dynamic_cast<textProperty*>(Prop);
  rp->vAlign = 0;
  rp->hAlign = 0;
  if (align & DT_VCENTER)
    rp->vAlign = 1;
  else if (align & DT_BOTTOM)
    rp->vAlign = 2;

  if (align & DT_CENTER)
    rp->hAlign = 1;
  else if (align & DT_RIGHT)
    rp->hAlign = 2;
}
//-----------------------------------------------------------
LPCTSTR svmObjText::setBoxedAlign(LPCTSTR row)
{
  textProperty* rp = dynamic_cast<textProperty*>(Prop);
  rp->textType |= textProperty::eboxed;
  uint align = _ttoi(row);
  rp->vAlign = 0;
  rp->hAlign = 0;
  if (!(align & DT_SINGLELINE))
    rp->textType |= textProperty::eMultiline;
  else if (align & DT_VCENTER)
    rp->vAlign = 1;
  else if (align & DT_BOTTOM)
    rp->vAlign = 2;

  if (align & DT_CENTER)
    rp->hAlign = 1;
  else if (align & DT_RIGHT)
    rp->hAlign = 2;
  return findNextParamTrim(row);
}
//-----------------------------------------------------------
bool svmObjText::load(LPCTSTR row)
{
  dataProject& dp = getDataProject();
  infoState& state = dp.getCurrState();
  textProperty* rp = dynamic_cast<textProperty*>(Prop);

  rp->foreground = state.foreground;
  svmFont& f = isLinked() ? getFontObjLinked() : getFontObj();

  rp->idFont = state.idFont;
  rp->Angle = state.Angle;

  uint code = _ttoi(row);
  row = findNextParamTrim(row);
  PRect r = getRect();
  while(row) {
    r.Offset(_ttoi(row), 0);
    row = findNextParamTrim(row);
    if(!row)
      break;
    r.Offset(0, -_ttoi(row));
    row = findNextParamTrim(row);
    if(!row)
      break;
    bool success = true;
    switch(code) {
      case PRN_TXT_ON_BOX:
        if(!getDim(row, r)) {
          success = false;
          break;
          }
        row = setBoxedAlign(row);
        rp->setText(row);
        break;
      case PRN_TXT:
        rp->setText(row);
    setAlign(state.Align);
        break;
      case PRN_TXT_ON_BOX_VAR:
        if(!getDim(row, r)) {
          success = false;
          break;
          }
        row = setBoxedAlign(row);
        if(row) {
          TCHAR t[64];
          rp->idVar = _ttoi(row) + 1;
          wsprintf(t, _T("Var. %d"), rp->idVar);
          rp->setText(t);
          };
        rp->textType |= textProperty::eVariable;
        break;
      case PRN_TXT_VAR:
        do {
          TCHAR t[64];
          rp->idVar = _ttoi(row) + 1;
          wsprintf(t, _T("Var. %d"), rp->idVar);
          rp->setText(t);
          } while(false);
        rp->textType |= textProperty::eVariable;
        break;
      }
    if(!success)
      break;
    Prop->Rect = r;
    makeTrueRect();
    return true;
    }
  return false;
}
