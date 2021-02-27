//-------------------- svmObjEdit.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
//-----------------------------------------------------------
#include "svmObjEdit.h"
#include "svmDefObj.h"
#include "svmPropertyEdit.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
bool isObjEdit(svmObject* t) { return toBool(dynamic_cast<svmObjEdit*>(t)); }
svmObject* allocObjEdByText(svmObject* text, int id) { return new svmObjEdit(text, id); }
//-----------------------------------------------------------
static Property staticPT;
//-----------------------------------------------------------
svmObjEdit::svmObjEdit(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oEDIT, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  Prop = new Property;
  Prop->background = GetSysColor(COLOR_WINDOW);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
svmObjEdit::svmObjEdit(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oEDIT, r)
{
  Prop = new Property;
  Prop->background = GetSysColor(COLOR_WINDOW);
  Prop->Rect = getFirstRect();
}
//-----------------------------------------------------------
svmObjEdit::svmObjEdit(svmObject* other, int id) :
  svmObject(other->getParent(), id, oEDIT, other->getRect())
{
  Prop = new Property;
  *Prop = *other->getProp();
  if(Prop->typeVal != 14)
    Prop->type1 = 0;
  else
    Prop->type1 = Prop->nDec;
}
//-----------------------------------------------------------
enum editStyle { esOld, esNoBorder, esSingleBorder, esNormalBorder, esRaiseBorder };
//-----------------------------------------------------------
int convertToSave(int style)
{
  switch(style) {
    case stylePanel::trasp:
      style = esNoBorder;
      break;
    case stylePanel::up_fill:
    case stylePanel::dn_fill:
    case stylePanel::fill:
    case stylePanel::border_fill:
      style = esOld;
      break;
    case stylePanel::up_trasp:
      style = esRaiseBorder;
      break;
    case stylePanel::dn_trasp:
      style = esNormalBorder;
      break;
    case stylePanel::border_trasp:
      style = esSingleBorder;
      break;
    }
  return style;
}
//-----------------------------------------------------------
int convertFromLoad(int style)
{
  switch(style) {
    case esNoBorder:
      style = PPanel::TRANSP;
      break;
    case esOld:
      style = PPanel:: FILL;
      break;
    case esRaiseBorder:
      style = PPanel::UP_TRANSP;
      break;
    case esNormalBorder:
      style = PPanel::DN_TRANSP;
      break;
    case esSingleBorder:
      style = PPanel::BORDER_TRANSP;
      break;
    }
  return style;
}
//-----------------------------------------------------------
void svmObjEdit::DrawObject(HDC hdc)
{
  HBRUSH brush = CreateSolidBrush(Prop->background);
  FillRect(hdc, getRect(), brush);
  DeleteObject((HGDIOBJ)brush);
  PRect r = getRect();
  int style =  convertToSave(getStyle());
  switch(style) {
    case esOld:
      DrawEdge(hdc, r, BDR_SUNKENOUTER, BF_RECT);
      break;
    case esNormalBorder:
      DrawEdge(hdc, r, BDR_SUNKENOUTER, BF_RECT);
      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_LEFT);
      break;
    case esSingleBorder:
      DrawEdge(hdc, r, BDR_SUNKENOUTER, BF_TOPLEFT);
      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_BOTTOMRIGHT);

      break;
    case esRaiseBorder:
//      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_BOTTOMRIGHT);

      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_RECT);
      r.Inflate(-1, -1);
      ++r.left;
      ++r.top;
      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_RECT);
//      ++r.left;
//      ++r.top;
//      DrawEdge(hdc, r, BDR_RAISEDOUTER, BF_TOPLEFT);
      break;
    }
//  staticPT.Rect = r;
}
//-----------------------------------------------------------
PRect svmObjEdit::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  PRect r(getRect());
  r.Inflate(-2, 0);
  return r;
}
//-----------------------------------------------------------
void makeTitleHex(LPTSTR buff, size_t dim)
{
  _tcscpy_s(buff, dim, _T("1a-2b-3c-4d"));
}
//-----------------------------------------------------------
void makeTitleBinary(LPTSTR buff, size_t dim)
{
  _tcscpy_s(buff, dim, _T("1000.0101.1110.0011"));
}
//-----------------------------------------------------------
smartPointerConstString svmObjEdit::getTitle() const
{
  static TCHAR buff[256];
  uint lenEdit = Prop->type1;
  if(lenEdit > SIZE_A(buff)) {
    lenEdit = SIZE_A(buff);
    Prop->type1 = lenEdit;
    }
  else if(!lenEdit)
    lenEdit = 6;
  int nDec = Prop->nDec;
  if((int)lenEdit < nDec + 1)
    lenEdit = nDec + 1;
  if(lenEdit && Prop->typeVal != 14) {
    if(Prop->typeVal != 5 && Prop->typeVal != 7 && Prop->BaseNum) {
      switch(Prop->BaseNum) {
        case ebBinary:
          makeTitleBinary(buff, SIZE_A(buff));
          break;
        case ebHex:
          makeTitleHex(buff, SIZE_A(buff));
          break;
        }
      }
    else {
      uint i;
      buff[0] = 0;
      for(i = 0; i < lenEdit - nDec - 1; ++i)
        buff[i] = (i % 10) + _T('1');
      if(nDec) {
        buff[i++] = _T('.');
        for(int j = 0; i < lenEdit; ++i, ++j)
          buff[i] = (j % 10) + _T('1');
        }
      buff[i] = 0;
      }
    return smartPointerConstString(buff, 0);
    }

  return smartPointerConstString(_T("Edit"), 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObject* svmObjEdit::makeClone()
{
  svmObjEdit* obj = new svmObjEdit(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjEdit::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
//  PRect r = other.getRect();
//  setRect(r);
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjEdit::dialogProperty(bool onCreate)
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
    if(IDOK == svmDialogEdit(this, Prop, getParent()).modal()) {
      staticPT = *Prop;
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void svmObjEdit::makeDef4Cam(uint prf, uint addr)
{
  Prop->perif = prf;
  Prop->addr = addr;
  Prop->typeVal = 10;
}
//-----------------------------------------------------------
bool svmObjEdit::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_EDI);
  uint id = moi.calcAndSetExtendId(order);

  TCHAR buff[1000];
  int lenEdit = Prop->type1;

  int style =  convertToSave(getStyle());

  int align = Prop->alignText;


  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),

            Prop->idFont + ID_INIT_FONT,

            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),

            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),
            lenEdit, align, style
            );

  writeStringChkUnicode(pfCrypt, buff);
  saveVar(pfCrypt, id);
  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjEdit::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = ID_INIT_FONT;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int lenEdit = 0;
  int style = 0;
  int align = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &lenEdit, &align, &style
                    );

  Prop->foreground = RGB(Rfg, Gfg, Bfg);
  Prop->background = RGB(Rbk, Gbk, Bbk);
  Prop->type1 = lenEdit;
  Prop->style =  convertFromLoad(style);
  Prop->alignText = align;
  Prop->idFont = idfont - ID_INIT_FONT;

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  loadVar(id, set);

  return true;
}
