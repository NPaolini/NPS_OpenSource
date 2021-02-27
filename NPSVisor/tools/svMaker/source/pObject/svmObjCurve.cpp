//-------------------- svmObjCurve.cpp -----------------------
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
#include "svmObjCurve.h"
#include "svmDefObj.h"
#include "svmPropertyCurve.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
//-----------------------------------------------------------
static PropertyCurve staticPT;
//-----------------------------------------------------------
svmObjCurve::svmObjCurve(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oCURVE, x1, y1, x1 + R__X(100), y1 + R__Y(100))
{
  init();
}
//-----------------------------------------------------------
svmObjCurve::svmObjCurve(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oCURVE, PRect(r.left, r.top, r.right, r.top + r.Width()))
{
  init();
}
//-----------------------------------------------------------
void svmObjCurve::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
}
//----------------------------------------------------------------------------
#define OFFY 5
#define RADIUS 1.5
//-----------------------------------------------------------
void svmObjCurve::DrawObject(HDC hdc)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);
  if(pt) {
    PRect r = getRect();
    PPanel panel(r, pt->background, (PPanel::bStyle)(pt->style | Property::FILL));
    panel.draw(hdc);
    COLORREF oldBkg = SetBkColor(hdc, pt->background);
    r.Inflate(-1, -1);
    const curveColors& set = pt->getSet();

    HPEN Pen = CreatePen(PS_DOT, 1, set.Color[curveColors::cGRID]);
    HGDIOBJ oldPen = SelectObject(hdc, Pen);

    if(pt->nDec) {
      double stepx = r.Width() / pt->nDec;
      int OffsX = (int)(stepx / 2);
      double x = r.left + OffsX;
      for(uint i = 0; i < pt->nDec; ++i) {
        MoveToEx(hdc, (int)x, r.top, 0);
        LineTo(hdc, (int)x, r.bottom);
        x += stepx;
        }
      }
    if(pt->nRow) {
      HPEN axPen = CreatePen(PS_DASHDOT, 1, set.Color[curveColors::cAXE]);
      SelectObject(hdc, axPen);

      double stepy = r.Height() - OFFY * 2;
      stepy /= pt->nRow;

      int y =  (int)(r.bottom - stepy * (pt->nRow / 2) - OFFY);
      MoveToEx(hdc, r.left, y, 0);
      LineTo(hdc, r.right, y);

      SelectObject(hdc, Pen);
      DeleteObject(axPen);

      double j = y + stepy;

      for(uint i = 0; i < (pt->nRow / 2); ++i, j += stepy) {
        MoveToEx(hdc, r.left, (int)j, 0);
        LineTo(hdc, r.right, (int)j);
        }
      j = y - stepy;
      for(uint i = 0; i < (pt->nRow / 2); ++i, j -= stepy) {
        MoveToEx(hdc, r.left, (int)j, 0);
        LineTo(hdc, r.right, (int)j);
        }

      }

    DeleteObject(SelectObject(hdc, oldPen));

    if(pt->nDec) {
      Pen = CreatePen(PS_SOLID, 1, pt->foreground);
      oldPen = SelectObject(hdc, Pen);
      double stepx = r.Width() / pt->nDec;
      int OffsX = (int)(stepx / 2);
      double x = r.left + OffsX;
      int vY[500];
      vY[0] = rand() % r.Height() + r.top;
      MoveToEx(hdc, (int)x, vY[0], 0);
      x += stepx;
      for(uint i = 1; i < pt->nDec; ++i) {
        vY[i] = rand() % r.Height() + r.top;
        LineTo(hdc, (int)x, vY[i]);
        x += stepx;
        }
      DeleteObject(SelectObject(hdc, oldPen));

      Pen = CreatePen(PS_SOLID, 1, set.Color[curveColors::cBALL]);
      oldPen = SelectObject(hdc, Pen);

      HBRUSH br = CreateSolidBrush(set.Color[curveColors::cBALL]);
      HGDIOBJ oldBrush = SelectObject(hdc, br);


      x = r.left + OffsX;
      for(uint i = 0; i < pt->nDec; ++i) {
        Ellipse(hdc, (int)(x - RADIUS), (int)(vY[i] - RADIUS), (int)(x + RADIUS), (int)(vY[i] + RADIUS));
        x += stepx;
        }
      DeleteObject(SelectObject(hdc, oldPen));
      DeleteObject(SelectObject(hdc, oldBrush));

      }
    SetBkColor(hdc, oldBkg);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjCurve::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjCurve::getRectTitle(HDC, LPCTSTR /*title*/) const
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
svmObject* svmObjCurve::makeClone()
{
  svmObjCurve* obj = new svmObjCurve(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjCurve::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjCurve::allocProperty() { return new PropertyCurve; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjCurve::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyCurve*>(Prop))->cloneMinusProperty(staticPT);
      }
    else {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      }
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogCurve(this, Prop, getParent()).modal()) {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmObjCurve::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_CURVE);
  uint id = moi.calcAndSetExtendId(order);

  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = pt->style & (Property::FILL - 1);
  switch(style) {
    case Property::NO:
      style = 0;
      break;
    case Property::UP:
      style = 1;
      break;
    case Property::DN:
      style = 2;
      break;
    case Property::BORDER:
      style = 3;
      break;
    }
  int readOnly = Prop->negative;
  if(readOnly && pt->hideBall)
    readOnly = 2;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
            id,
            REV__X(Prop->Rect.left),
            REV__Y(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__Y(Prop->Rect.Height()),

            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),

            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),

            style,

            Prop->nDec,
            Prop->type1,
            Prop->type2,
            readOnly
            );

  writeStringChkUnicode(pfCrypt, buff);

  saveVar(pfCrypt, id);

  const curveColors& set = pt->getSet() ;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            moi.getThirdExtendId(),
            pt->nRow,
            GetRValue(set.Color[curveColors::cBALL]),
            GetGValue(set.Color[curveColors::cBALL]),
            GetBValue(set.Color[curveColors::cBALL]),

            GetRValue(set.Color[curveColors::cGRID]),
            GetGValue(set.Color[curveColors::cGRID]),
            GetBValue(set.Color[curveColors::cGRID]),

            GetRValue(set.Color[curveColors::cAXE]),
            GetGValue(set.Color[curveColors::cAXE]),
            GetBValue(set.Color[curveColors::cAXE])
            );

  writeStringChkUnicode(pfCrypt, buff);
  if(pt->useFileShow && pt->maxX && *pt->fileShow)
    wsprintf(buff, _T(",%d,%d,%d,%d,%s\r\n"),
            pt->maxX,
            GetRValue(set.Color[curveColors::cLINE_SHOW]),
            GetGValue(set.Color[curveColors::cLINE_SHOW]),
            GetBValue(set.Color[curveColors::cLINE_SHOW]),
            pt->fileShow
            );

  else
    wsprintf(buff, _T("\r\n"));

  writeStringChkUnicode(pfCrypt, buff);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjCurve::load(uint id, setOfString& set)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);
  if(!pt)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 100;

  int Rl = 0;
  int Gl = 0;
  int Bl = 0;

  int Rb = 220;
  int Gb = 220;
  int Bb = 220;

  int style = 0;
  int nPoints = 10;
  int perc1stSpace = 0;
  int nPoints1stSpace = 0;
  int readOnly = 0;


  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &Rl, &Gl, &Bl,
                    &Rb, &Gb, &Bb,
                    &style, &nPoints,
                    &perc1stSpace, &nPoints1stSpace,
                    &readOnly
                    );

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  Prop->foreground = RGB(Rl, Gl, Bl);
  Prop->background = RGB(Rb, Gb, Bb);

  switch(style) {
    case 0:
      Prop->style = Property::NO;
      break;
    case 1:
      Prop->style = Property::UP;
      break;
    case 2:
      Prop->style = Property::DN;
      break;
    case 3:
      Prop->style = Property::BORDER;
      break;
    }
//  Prop->style = style;

  pt->type1 = perc1stSpace;
  pt->type2 = nPoints1stSpace;

  curveColors& setColor = pt->getSet() ;

  loadVar(id, set);
  pt->nDec = nPoints;
  Prop->negative = toBool(readOnly);
  pt->hideBall = 2 == readOnly;

  manageObjId moi(id, ID_INIT_VAR_CURVE);

  p = set.getString(moi.getThirdExtendId());
  if(p) {
    int nRow = 10;

    Rb = 0;
    Gb = 0;
    Bb = 0x7f;

    int Rg = 192;
    int Gg = 192;
    int Bg = 192;

    int Ra = 0xff;
    int Ga = 0;
    int Ba = 0;
    int maxX = 0;

    int Rls = 0;
    int Gls = 0;
    int Bls = 0;
    TCHAR fileShow[_MAX_PATH] = _T("\0");
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s"),
                    &nRow,
                    &Rb, &Gb, &Bb,
                    &Rg, &Gg, &Bg,
                    &Ra, &Ga, &Ba,
                    &maxX,
                    &Rls, &Gls, &Bls,
                    fileShow, SIZE_A(fileShow)
                    );
    curveColors& set = pt->getSet() ;
    pt->nRow = nRow;
    set.Color[0] = RGB(Rb, Gb, Bb);
    set.Color[1] = RGB(Rg, Gg, Bg);
    set.Color[2] = RGB(Ra, Ga, Ba);
    set.Color[3] = RGB(Rls, Gls, Bls);
    pt->maxX = maxX;
    if(*fileShow) {
      _tcscpy_s(pt->fileShow, fileShow);
      pt->useFileShow = true;
      }
    else
      pt->useFileShow = false;
    }
  return true;
}
//-----------------------------------------------------------
void svmObjCurve::addInfoAd4Other(manageInfoAd4& set)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);
  LPCTSTR info = _T("Punto n° %d");
  for(int i = 1; i < (int)pt->nDec; ++i) {
		TCHAR t[64];
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
		wsprintf(t, info, i + 1);
    _tcscpy_s(ia4->infoVar, t);
    ia4->prph = pt->perif;
    ia4->addr = pt->addr + i;
    ia4->type = pt->typeVal;
    set.add(ia4);
    }
}


