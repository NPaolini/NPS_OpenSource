//------------------ svmObjButton.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmObjButton.h"
#include "svmDefObj.h"
#include "PTraspBitmap.h"
#include "svmMainClient.h"
#include "svmPageProperty.h"
#include "svmPagePropAction.h"
//-----------------------------------------------------------
static PropertyBtn staticPT;
//-----------------------------------------------------------
svmObjButton::svmObjButton(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oBUTTON, x1, y1, x2, y2), hTheme(0)
{
  init();
}
//-----------------------------------------------------------
svmObjButton::svmObjButton(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oBUTTON, r), hTheme(0)
{
  init();
}
//-----------------------------------------------------------
svmObjButton::~svmObjButton()
{
  if(hTheme)
    manageXP_Theme::CloseThemeData(hTheme);
}
//-----------------------------------------------------------
void svmObjButton::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  offsXText = 0;
  loadTheme();
}
//-----------------------------------------------------------
LPTSTR svmObjButton::chgFunctionData(LPCTSTR source, bool forPrph, const infoOffsetReplace& ior)
{
  TCHAR t[_MAX_PATH + 10];
  _tcscpy_s(t, source);
  LPTSTR pt = t;
  while(*pt && _T('?') != *pt)
    ++pt;
  perifForBtnAction data;
  perifForBtnAction::fromString(data, pt);
  bool changed = false;

  if(forPrph) {
    bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
    if(isValidPrph4OffsetAddr(data.U.s.prf, ior)) {
      if(isValid4OffsetAddr(data.U.s.prf, data.addr, exclude)) {
        data.addr += ior.offs;
        changed = true;
        }
      }
    }
  else {
    if(ior.prphFrom == data.U.s.prf) {
      data.U.s.prf = ior.prphTo;
      changed = true;
      }
    }

  if(changed) {
    perifForBtnAction::toString(pt, data);
    return str_newdup(t);
    }
  return 0;
}
//-----------------------------------------------------------
static bool isFnz4Offs(PropertyBtn* pb)
{
  return btnAction == pb->type2 && !pb->perif && pb->functionLink &&
        ( isSendOneData(pb->functionLink) || isSendToggleData(pb->functionLink));
}
//-----------------------------------------------------------
void svmObjButton::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  PropertyBtn* pb = (PropertyBtn*)Prop;
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isSetBitf(eoAlsoOther, ior.flag)) {
      bool exclude = isSetBitf(eoExcludePrph1, ior.flag);
      if(isFnz4Offs(pb)) {
        LPTSTR t = chgFunctionData(pb->functionLink, true, ior);
        if(t) {
          delete pb->functionLink;
          pb->functionLink = t;
          }
        }
      for(uint i = 0; i < SIZE_A(pb->DataPrf); ++i) {
        if(isValidPrph4OffsetAddr(pb->DataPrf[i].prph, ior))
          if(isValid4OffsetAddr(pb->DataPrf[i].prph, pb->DataPrf[i].addr, exclude))
            pb->DataPrf[i].addr += ior.offs;
        }
      }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    for(uint i = 0; i < SIZE_A(pb->DataPrf); ++i) {
      if(ior.prphFrom == pb->DataPrf[i].prph)
        pb->DataPrf[i].prph = ior.prphTo;
      }

    if(isFnz4Offs(pb)) {
      LPTSTR t = chgFunctionData(pb->functionLink, false, ior);
      if(t) {
        delete pb->functionLink;
        pb->functionLink = t;
        }
      }
    }

  if(isSetBitf(eoSubstVar, ior.flag)) {
    for(uint i = 0; i < SIZE_A(pb->DataPrf); ++i) {
      if(ior.prphFrom2 == pb->DataPrf[i].prph && ior.addrFrom == pb->DataPrf[i].addr) {
        pb->DataPrf[i].prph = ior.prphTo2;
        pb->DataPrf[i].addr = ior.addrTo;
        }
      }
    }
}
//-----------------------------------------------------------
void svmObjButton::loadTheme()
{
  if(manageXP_Theme::isValid())
    hTheme = manageXP_Theme::OpenThemeData(*getParent(), L"Button");
}
//-----------------------------------------------------------
Property* svmObjButton::allocProperty() { return new PropertyBtn; }
//-----------------------------------------------------------
// bordo per riquadro focus (letto dal sVisor
#define EDGE_BTN_X 2
//#define EDGE_BTN_X (rectColor.tickness)
#define EDGE_BTN_Y EDGE_BTN_X
//-----------------------------------------------------------
#define DIM_BORDER GetSystemMetrics(SM_CXEDGE)
//-----------------------------------------------------------
#define INFLATE_BMP_WIDTH 10
#define INFLATE_BMP_HEIGHT 10
//-----------------------------------------------------------
#define EDGE_BMP_X (INFLATE_BMP_WIDTH / 2)
#define EDGE_BMP_Y (INFLATE_BMP_HEIGHT / 2)
//-----------------------------------------------------------
#define RESCALE 0.97
//-----------------------------------------------------------
#define DEF_POINT  { 2, 2 }
//#define DEF_POINT  { R__X(EDGE_BMP_X) + EDGE_BTN_X, R__Y(EDGE_BMP_Y) + EDGE_BTN_Y }
//#define DEF_POINT  { EDGE_BMP_X + EDGE_BTN_X, EDGE_BMP_Y + EDGE_BTN_Y }
extern
double scaleImage(const SIZE& winSz, SIZE& bmpSz);
//-----------------------------------------------------------
void svmObjButton::DrawObject(HDC hdc)
{
  if(btnHide  == Prop->type2) {
    drawFocus(hdc);
    return;
    }
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);

  if(btnS_StdAndBmp  == Prop->type1 || (btnS_NewStdAndBmp  == Prop->type1 && (!pb->Flags.theme || !isThemed()))) {
    PRect r(getRect());
    r.Inflate(-EDGE_BTN_X, -EDGE_BTN_Y);

    if(btnS_NewStdAndBmp  == Prop->type1 && pb->Flags.flat || (pb->Flags.noBorder && Property::TRANSP & Prop->style)) {  }
    else if(Property::TRANSP & Prop->style && !pb->Flags.noBorder)
      DrawEdge(hdc, r, EDGE_RAISED, BF_RECT);

    else {
      HBRUSH br = CreateSolidBrush(Prop->background);
      DrawFrameControl(hdc, r, DFC_BUTTON, DFCS_BUTTONPUSH);
      r.Inflate(-DIM_BORDER, -DIM_BORDER);

      FillRect(hdc, r, br);
      DeleteObject(HGDIOBJ(br));
      }
    }
  else if(btnS_NewStdAndBmp == Prop->type1) {
    PRect r(getRect());
    r.Inflate(-EDGE_BTN_X, -EDGE_BTN_Y);
    ++r.right;
    ++r.bottom;
    DWORD state = PBS_NORMAL;

    if(pb->Flags.flat || pb->Flags.noBorder) {
      // manageXP_Theme::DrawThemeParentBackground(*getParent(), hdc, r);
      }
    else {
//      if(manageXP_Theme::IsThemeBackgroundPartiallyTransparent(hTheme, BP_PUSHBUTTON, state))
//        manageXP_Theme::DrawThemeParentBackground(*getParent(), hdc, r);
      manageXP_Theme::DrawThemeBackground(hTheme, hdc, BP_PUSHBUTTON, state, r, NULL);
      }
    }

  offsXText = 0;

  const PVect<LPCTSTR>& nBmp = pb->getNames() ;
  if(nBmp.getElem()) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, nBmp[0]);
    PBitmap bmp(name);
    if(bmp.isValid()) {
      SIZE sz = bmp.getSize();
      PRect r(getRect());
      SIZE objSz = { r.Width(), r.Height() };

      POINT pt = DEF_POINT;
      SIZE wSz = objSz;
      double Scale = 1.0;
      if(btnS_OnlyBmp == Prop->type1 || btnS_BmpAndText == Prop->type1) {
        pt.x = 0;
        pt.y = 0;
        }
      else if(btnS_NewStdAndBmp  == Prop->type1) {
        switch(pb->Flags.pos) {
          case POwnBtnImageStd::wLeft:
          case POwnBtnImageStd::wRight:
            wSz.cx /= 2;
            break;
          case POwnBtnImageStd::wTop:
          case POwnBtnImageStd::wBottom:
            wSz.cy /= 2;
            break;
          }
        switch(pb->Flags.styleBmpDim) {
          case POwnBtnImageStd::eisStretched:
            break;
          case POwnBtnImageStd::eisScaled:
            Scale = scaleImage(wSz, sz);
            break;
          }
        switch(pb->Flags.pos) {
          case POwnBtnImageStd::wLeft:
            ++pt.x;
            pt.y = (wSz.cy - sz.cy) / 2;
            break;
          case POwnBtnImageStd::wRight:
            pt.x = r.Width() - pt.x - sz.cx - 1;
            pt.y = (wSz.cy - sz.cy) / 2;
            break;
          case POwnBtnImageStd::wTop:
            pt.x = (wSz.cx - sz.cx) / 2;
            ++pt.y;
            break;
          case POwnBtnImageStd::wBottom:
            pt.y = r.Height() - pt.y - sz.cy - 1;
            pt.x = (wSz.cx - sz.cx) / 2;
            break;
          case POwnBtnImageStd::wCenter:
            if(POwnBtnImageStd::eisStretched == pb->Flags.styleBmpDim) {
              pt.x = 0;
              pt.y = 0;
              }
            else {
              pt.x = (wSz.cx - sz.cx) / 2;
              pt.y = (wSz.cy - sz.cy) / 2;
              }
            break;
          }
        }
      else {
        wSz.cx -= R__X(INFLATE_BMP_WIDTH + EDGE_BTN_X * 2);
        wSz.cy -= R__Y(INFLATE_BMP_HEIGHT + EDGE_BTN_Y * 2);
        }

      if(btnS_NewStdAndBmp != Prop->type1) {
        double scaleX = wSz.cx;
        scaleX /= sz.cx;

        double scaleY = wSz.cy;
        scaleY /= sz.cy;

        Scale = scaleX > scaleY ? scaleY : scaleX;
        offsXText = (int)(sz.cx * Scale);
        }
      pt.x += r.left;
      pt.y += r.top;
      if(POwnBtnImageStd::eisStretched == pb->Flags.styleBmpDim) {
        double scaleX = wSz.cx;
        scaleX /= (double)sz.cx;
        double scaleY = wSz.cy;
        scaleY /= (double)sz.cy;
        PTraspBitmap tb(getParent(), &bmp, pt);
        tb.setScale(scaleX, scaleY);
        tb.Draw(hdc);
        }
      else {
        PTraspBitmap tb(getParent(), &bmp, pt);
        if(Scale != 1.0)
          tb.setScale(Scale);
        tb.Draw(hdc);
        }
      }
    }
  drawFocus(hdc);
}
//-----------------------------------------------------------
smartPointerConstString svmObjButton::getTitle() const
{
  if(btnHide  == Prop->type2)
    return smartPointerConstString(_T("Pulsante nascosto"), 0);

  PropertyBtn* pt = dynamic_cast<PropertyBtn*>(Prop);
  if(btnS_OnlyBmp == Prop->type1) {
    if(pt) {
      const PVect<LPCTSTR>& nBmp = pt->getNames() ;
      if(nBmp.getElem())
        return smartPointerConstString(0, 0);
      }
    }
  else if(pt && pt->normalText)
    return getStringByLangSimple(pt->normalText);
  return smartPointerConstString(_T("Pulsante"), 0);
}
//-----------------------------------------------------------
void svmObjButton::drawFocus(HDC hdc)
{
  int midSize = EDGE_BTN_X / 2;
  PRect r(getRect());
  SIZE sz = { r.Width() - midSize, r.Height() - midSize };
//  SIZE sz = { r.Width() - 1 - midSize, r.Height() - 1 - midSize };

  POINT pts[] = {
      { midSize, midSize },
      { sz.cx, midSize },
      { sz.cx, sz.cy },
      { midSize, sz.cy },
      { midSize, midSize }
      };
  for(uint i = 0; i < SIZE_A(pts); ++i) {
    pts[i].x += r.left;
    pts[i].y += r.top;
    }
  HPEN pen = CreatePen(PS_SOLID, EDGE_BTN_X, RGB(0xff, 0, 0));
  HGDIOBJ old = SelectObject(hdc, pen);
  Polyline(hdc, pts, SIZE_A(pts));
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
void svmObjButton::DrawRectTitle(HDC hdc, LPCTSTR title, PRect rect)
{
  SetTextAlign(hdc, TA_LEFT | TA_TOP);
  UINT format = DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS;// | DT_END_ELLIPSIS;

  HFONT fnt = Prop->getFont(Linked);
  HGDIOBJ oldFont = SelectObject(hdc, fnt);
  COLORREF oldColor = SetTextColor(hdc, getProp()->foreground);
  int oldMode = SetBkMode(hdc, TRANSPARENT);

  DrawText(hdc, title, -1, rect, format);
  SetBkMode(hdc, oldMode);
  SetTextColor(hdc, oldColor);
  SelectObject(hdc, oldFont);
}
//-----------------------------------------------------------
PRect svmObjButton::getRectTitle(HDC hdc, LPCTSTR title) const
{
#if 1
  PRect r(getRect());
  if(btnS_StdAndBmp == Prop->type1)
    r.left += offsXText;
  else if(btnS_NewStdAndBmp  == Prop->type1) {
    PropertyBtn* pt = dynamic_cast<PropertyBtn*>(Prop);
    const PVect<LPCTSTR>& nBmp = pt->getNames() ;
    if(nBmp.getElem()) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, nBmp[0]);
      PBitmap bmp(name);
      if(bmp.isValid()) {
        SIZE sz = bmp.getSize();
        SIZE wSz = { r.Width(), r.Height() };
        switch(pt->Flags.pos) {
          case POwnBtnImageStd::wLeft:
          case POwnBtnImageStd::wRight:
            wSz.cx /= 2;
            break;
          case POwnBtnImageStd::wTop:
          case POwnBtnImageStd::wBottom:
            wSz.cy /= 2;
            break;
          }
        switch(pt->Flags.styleBmpDim) {
          case POwnBtnImageStd::eisStretched:
            sz.cx = 0;
            sz.cy = 0;
            break;
          case POwnBtnImageStd::eisScaled:
            scaleImage(wSz, sz);
            break;
          }

        switch(pt->Flags.pos) {
          case POwnBtnImageStd::wLeft:
            r.left += sz.cx;
            break;
          case POwnBtnImageStd::wTop:
            r.top += sz.cy;
            break;
          case POwnBtnImageStd::wRight:
            r.right -= sz.cx;
            break;
          case POwnBtnImageStd::wBottom:
            r.bottom -= sz.cy;
            break;
          }
        }
      }
    }
  PRect calc(r);

  HFONT fnt = Prop->getFont(Linked);
  HGDIOBJ old = SelectObject(hdc, fnt);
  DrawText(hdc, title, -1, calc, DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
  SelectObject(hdc, old);
  if(btnS_NewStdAndBmp  != Prop->type1) {
    if(calc.right > r.right)
      calc.right = r.right;
    }
  int diffW = r.Width() - calc.Width();
  int diffH = r.Height() - calc.Height();
  if(diffH || diffW)
    calc.Offset(diffW / 2, diffH / 2);

  int offs = -1;
  calc.Offset(offs, offs);
  return calc;
#else
  if(btnS_StdAndBmp == Prop->type1) {
    PRect r(getRect());

    r.left += offsXText;
    PRect calc(r);

    HFONT fnt = Prop->getFont(Linked);
    HGDIOBJ old = SelectObject(hdc, fnt);
    DrawText(hdc, title, -1, calc, DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
    SelectObject(hdc, old);
    if(calc.right > r.right)
      calc.right = r.right;

    int diffW = r.Width() - calc.Width();
    int diffH = r.Height() - calc.Height();
    if(diffH || diffW)
      calc.Offset(diffW / 2, diffH / 2);

    int offs = -1;
    calc.Offset(offs, offs);
    return calc;
    }
  return getRect();
#endif
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObject* svmObjButton::makeClone()
{
  svmObjButton* obj = new svmObjButton(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjButton::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
//  PRect r = other.getRect();
//  setRect(r);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjButton::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyBtn*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogBtn(this, Prop, getParent()).modal()) {
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
void svmObjButton::makeDef4Cam(uint prf, uint addr, uint offsBit, LPCTSTR released, LPCTSTR pressed)
{
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);
  if(!pb)
    return;
  pb->normalText = str_newdup(released);
  pb->pressedText = str_newdup(pressed);
  pb->perif = prf;
  pb->addr = addr;
  pb->typeVal = 1;
  pb->nBits = 1;
  pb->offset = offsBit;
  pb->type1 = btnS_StdAndBmp;
  pb->type2 = btnOnOff;
}
//-----------------------------------------------------------
#define ID_PRPH_4_KEYB 100
//-----------------------------------------------------------
static
void formatColor(LPTSTR buff, size_t sz, double val, COLORREF fg, COLORREF bg)
{
  _stprintf_s(buff, sz, _T(",%0.3f,%d,%d,%d,%d,%d,%d"), val,
      GetRValue(fg), GetGValue(fg), GetBValue(fg),
      GetRValue(bg), GetGValue(bg), GetBValue(bg)
      );
}
//-----------------------------------------------------------
static
void unformatColor(LPCTSTR buff, PVect<double>& colorVal, PVect<COLORREF>& fg_, PVect<COLORREF>& bg_)
{
  for(uint i = 0; buff; ++i) {
    double v;
    int fr;
    int fg;
    int fb;
    int br;
    int bg;
    int bb;
    _stscanf_s(buff, _T("%lf,%d,%d,%d,%d,%d,%d"), &v, &fr, &fg, &fb, &br, &bg, &bb);
    colorVal[i] = v;
    fg_[i] = RGB(fr, fg, fb);
    bg_[i] = RGB(br, bg, bb);
    buff = findNextParamTrim(buff, 7);
    }
}
//-----------------------------------------------------------
static
void saveValue(P_File& pf, uint id, const PVect<double>& vals)
{
  uint nElem = vals.getElem();
  TCHAR buff[64];
  wsprintf(buff, _T("%d"), id);
  writeStringChkUnicode(pf, buff);

  for(uint i = 0; i < nElem; ++i) {
    _stprintf_s(buff, _T(",%0.3f"), vals[i]);
    writeStringChkUnicode(pf, buff);
    }
  writeStringChkUnicode(pf, _T("\r\n"));
}
//-----------------------------------------------------------
static
void loadValue(LPCTSTR buff, setOfString& set, PVect<double>& vals)
{
  if(!buff)
    return;
  uint id = _ttoi(buff);
  if(id) {
    LPCTSTR p = set.getString(id);
    if(p) {
      for(uint i = 0; p; ++i) {
        double v;
        _stscanf_s(p, _T("%lf"), &v);
        vals[i] = v;
        p = findNextParamTrim(p);
        }
      }
    }
}
//-----------------------------------------------------------
bool svmObjButton::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_BTN);
  uint id = moi.calcAndSetExtendId(order);
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);
  if(!pb)
    return false;

  const PVect<LPCTSTR>& names = pb->getNames();
  int nBmp = names.getElem();

  if(!nBmp && btnS_OnlyBmp == pb->type1 && btnHide != pb->type2) {
    MessageBox(*getParent(), _T("Pulsante solo Bitmap con nessun bitmap assegnato\nverrà salvato come Standard+Bitmap!"),
        _T("Errore Pulsante"), MB_OK | MB_ICONINFORMATION);
    pb->type1 = btnS_StdAndBmp;
    }
  TCHAR buff[1000];
  dual* dualBmp = getDualBmp4Btn();
  svmObjCount& objCount = getObjCount();

  if(btnS_OnlyBmp == pb->type1 || btnS_BmpAndText == pb->type1)
    pb->fillNameBmp();

  PVect<int> idbmp;
  idbmp.setDim(max(4, nBmp));
  for(int i = 0; i < nBmp; ++i)
    idbmp[i] = dualBmp->addAndGetId(names[i]);

  Prop->nDec = Prop->negative;

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),
            Prop->type1, Prop->type2,
            nBmp);

  for(int i = 0; i < nBmp; ++i) {
    TCHAR t[50];
    wsprintf(t, _T(",%d"), idbmp[i]);
    _tcscat_s(buff, t);
    }

  int idMsg1 = 0;
  int idMsg2 = 0;
  if(btnS_OnlyBmp == pb->type1) {
    // nessuna azione
    }
  else {
    TCHAR t[256];
    if(btnS_StdAndBmp == pb->type1 || btnS_NewStdAndBmp == pb->type1) {
      int r = GetRValue(Prop->background);
      int g = GetGValue(Prop->background);
      int b = GetBValue(Prop->background);

      if(Prop->style & Property::TRANSP) {
        r = -1;
        g = -1;
        b = -1;
        }
      wsprintf(t, _T(",%d,%d,%d,%d,%d,%d"),
            r, g, b,
            GetRValue(pb->bgPress),
            GetGValue(pb->bgPress),
            GetBValue(pb->bgPress));
      _tcscat_s(buff, t);
      }

    wsprintf(t, _T(",%d,%d,%d,%d,%d,%d,%d"),
            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),
            GetRValue(pb->fgPress),
            GetGValue(pb->fgPress),
            GetBValue(pb->fgPress),
            Prop->idFont + ID_INIT_FONT);

    _tcscat_s(buff, t);
    if(pb->normalText && *pb->normalText) {
      idMsg1 = objCount.getGlobalIdCount();
      idMsg2 = idMsg1;
      }
    if(pb->pressedText && *pb->pressedText) {
      if(!idMsg1 || _tcscmp(pb->pressedText, pb->normalText))
        idMsg2 = objCount.getGlobalIdCount();
      }
    if(idMsg2) {
      wsprintf(t, _T(",%d,%d"), idMsg1, idMsg2);
      _tcscat_s(buff, t);
      }
    }
  writeStringChkUnicode(pfCrypt, buff);

  if(!(btnS_NewStdAndBmp == pb->type1))
    writeStringChkUnicode(pfCrypt, _T("\r\n"));
  else { // aggiunge gli id per il nuovo pulsante
    uint nElemCol = min(pb->otherFg.getElem(), pb->otherBg.getElem());
    uint idCol = 0;
    if(nElemCol && pb->Flags.colorByVar)
      idCol = objCount.getGlobalIdCount();

    uint idPrphCol = 0;
    if(pb->DataPrf[0].prph && pb->Flags.colorByVar) {
      manageObjId moi2(0, ID_INIT_VAR_EDI);
      idPrphCol = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
      }

    uint idOtherText = 0;
    uint nElemText = pb->allText.getElem();
    if(nElemText > 2)
      // occorre lasciare un buco vuoto per terminare la lista
      idOtherText = objCount.getGlobalIdCount(nElemText - 1);

    uint idPrphText = 0;
    if(pb->DataPrf[1].prph && pb->Flags.textByVar) {
      manageObjId moi2(0, ID_INIT_VAR_EDI);
      idPrphText = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
      }

    uint idPrphBmp = 0;
    if(pb->DataPrf[2].prph && pb->Flags.bitmapByVar) {
      manageObjId moi2(0, ID_INIT_VAR_EDI);
      idPrphBmp = moi2.calcBaseExtendId(objCount.getIdCount(oEDIT));
      }

    uint nElemValueText = pb->textVal.getElem();
    uint idValueText = 0;
    if(nElemValueText && pb->Flags.textByVar)
      idValueText = objCount.getGlobalIdCount();

    uint nElemValueBmp = pb->bmpVal.getElem();
    uint idValueBmp = 0;
    if(nElemValueBmp && pb->Flags.bitmapByVar)
      idValueBmp = objCount.getGlobalIdCount();

    wsprintf(buff, _T(",%d,%d,%d,%d,%d,%u,%d,%d\r\n"),
        idCol, idPrphCol, idOtherText, idPrphText, idPrphBmp, pb->Flags, idValueText, idValueBmp);

    writeStringChkUnicode(pfCrypt, buff);

    if(idCol) {
      wsprintf(buff, _T("%d"), idCol);
      writeStringChkUnicode(pfCrypt, buff);
      uint nValColor = pb->colorVal.getElem();

      for(uint i = 0; i < nElemCol; ++i) {
        double v = (1 == pb->Flags.colorByVar) ? i : i < nValColor ? pb->colorVal[i] : 0;
        formatColor(buff, SIZE_A(buff), v, pb->otherFg[i], pb->otherBg[i]);
        writeStringChkUnicode(pfCrypt, buff);
        }
      writeStringChkUnicode(pfCrypt, _T("\r\n"));
      }

    if(idPrphCol)
      saveFullPrph(pfCrypt, buff, idPrphCol, pb->DataPrf[0]);


    if(idOtherText) {
      nElemText -= 2;
      for(uint i = 0; i < nElemText; ++i, ++idOtherText) {
        wsprintf(buff, _T("%d,%s\r\n"), idOtherText, pb->allText[i + 2]);
        writeStringChkUnicode(pfClear, pfCrypt, buff);
        }
      }
    if(idPrphText)
      saveFullPrph(pfCrypt, buff, idPrphText, pb->DataPrf[1]);

    if(idPrphBmp)
      saveFullPrph(pfCrypt, buff, idPrphBmp, pb->DataPrf[2]);

    if(idValueText)
      saveValue(pfCrypt, idValueText, pb->textVal);

    if(idValueBmp)
      saveValue(pfCrypt, idValueBmp, pb->bmpVal);
    }
  if(idMsg1) {
    wsprintf(buff, _T("%d,%s\r\n"), idMsg1, pb->normalText);
    writeStringChkUnicode(pfClear, pfCrypt, buff);
    }
  if(idMsg2 && idMsg2 != idMsg1) {
    wsprintf(buff, _T("%d,%s\r\n"), idMsg2, pb->pressedText);
    writeStringChkUnicode(pfClear, pfCrypt, buff);
    }

  if(btnAction == Prop->type2) {
    int idAction = Prop->perif;
    uint idPrph = 0;
    if(!idAction && pb->functionLink) {
      if(isSendOneData(pb->functionLink)) {
        int idBtn = objCount.getIdCount(oBUTTON);
        manageObjId moi2(0, ID_INIT_VAR_BTN);
        idBtn = moi2.calcAndSetExtendId(idBtn);
        int psw = setSendOneByBtn(pb->functionLink, idBtn, pfCrypt);
        idAction = objCount.getFunctionKeyIdCount();
        wsprintf(buff, _T("%d,%d,%d,%d\r\n"), idAction, getSendOneDataCode(), psw, idBtn);
        }
      else if(isSendToggleData(pb->functionLink)) {
        int idBtn = objCount.getIdCount(oBUTTON);
        manageObjId moi2(0, ID_INIT_VAR_BTN);
        idBtn = moi2.calcAndSetExtendId(idBtn);
        int psw = setSendToggleByBtn(pb->functionLink, idBtn, pfCrypt);
        idAction = objCount.getFunctionKeyIdCount();
        wsprintf(buff, _T("%d,%d,%d,%d\r\n"), idAction, getSendToggleDataCode(), psw, idBtn);
        }
      else  if(isChgPsw(pb->functionLink)) {
        idAction = objCount.getFunctionKeyIdCount();
        int pswLevel = Prop->pswLevel;
        if(4 == pswLevel)
          pswLevel = ONLY_CONFIRM;
        else if(!pswLevel)
          pswLevel = 1;
        wsprintf(buff, _T("%d,%d,%d\r\n"), idAction, getChgPswCode(), pswLevel);
        }
      else  if(isKeyb4TouchScreen(pb->functionLink)) {
        idPrph = ID_PRPH_4_KEYB;
        LPCTSTR p = findNextParamTrim(pb->functionLink);
        if(p)
          idAction = _ttoi(p);
        }
      else {
        if(isChangePage(pb->functionLink))
          idAction = objCount.getChgPageIdCount();
        else
          idAction = objCount.getFunctionKeyIdCount();
        TCHAR tmp[500];
        translateFromCRNL(tmp, pb->functionLink);
        wsprintf(buff, _T("%d,%s\r\n"), idAction, tmp);
        }
      if(!idPrph)
        writeStringChkUnicode(pfCrypt, buff);

      }
    wsprintf(buff, _T("%d,%d,%d\r\n"), moi.getFirstExtendId(), idPrph, idAction);
    writeStringChkUnicode(pfCrypt, buff);

 //   if(Prop->visPerif)
      saveOnlyVisVar(moi.getSecondExtendId(), pfCrypt);
    }
  else
    saveVar(pfCrypt, id);

  if(btnModeless == Prop->type2) {
    if(pb->modelessName) {
      int idOpen = moi.getThirdExtendId();
      wsprintf(buff, _T("%d,"), idOpen);
      writeStringChkUnicode(pfCrypt, buff);
      writeStringChkUnicode(pfCrypt, translateFromCRNL(buff, pb->modelessName));
      writeStringChkUnicode(pfCrypt, _T("\r\n"));
      }
    }
  if(btnOpenPageByBit == Prop->type2) {
    while(pb->modelessName) {
      int idOpen = moi.getThirdExtendId();
      wsprintf(buff, _T("%d,"), idOpen);
      writeStringChkUnicode(pfCrypt, buff);
      uint idPage = _ttoi(pb->modelessName);
      LPCTSTR pp = findNextParamTrim(pb->modelessName);
      if(!pp)
        break;
      uint psw = _ttoi(pp);
      pp = findNextParamTrim(pp);
      if(!pp)
        break;
      if(aOpenfileRecipeDan == idPage)
        wsprintf(buff, _T("%d,%d,1,"), aOpenfileRecipe, psw);
      else
        wsprintf(buff, _T("%d,%d,0,"), idPage, psw);
      writeStringChkUnicode(pfCrypt, buff);

      writeStringChkUnicode(pfCrypt, translateFromCRNL(buff, pp));
      writeStringChkUnicode(pfCrypt, _T("\r\n"));
      break;
      }
    }

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjButton::load(uint id, setOfString& set)
{
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);
  if(!pb)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int typeShow = 0;
  int typeCommand = 0;
  int nBmp = 0;

  LPCTSTR p = set.getString(id);
  if(!p)
    return false;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &typeShow,
                    &typeCommand,
                    &nBmp);

  if(btnHide == typeCommand)
    return false;

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  Prop->type1 = typeShow;
  Prop->type2 = typeCommand;

  p = findNextParam(p, 7);
  do {
    if(!p)
      break;

    if(nBmp) {
      PVect<LPCTSTR>& names = pb->getNames();
      dual* dualBmp;
      if(Linked)
        dualBmp = getDualBmp4BtnLinked();
      else
        dualBmp = getDualBmp4Btn();
      int j = 0;
      for(int i = 0; i < nBmp; ++i) {
        int idbmp = _ttoi(p);
        LPCTSTR name = dualBmp->getTxt(idbmp);
        if(name)
          names[j++] = str_newdup(getOnlyName(name, true));
        p = findNextParam(p, 1);
        if(!p)
          break;
        }
      }

    if(btnS_OnlyBmp != typeShow) {
      if(!p)
        break;
      int Rfg1 = 0;
      int Gfg1 = 0;
      int Bfg1 = 0;
      int Rfg2 = 0;
      int Gfg2 = 0;
      int Bfg2 = 0;
      int idfont = ID_INIT_FONT;
      int idMsg1 = 0;
      int idMsg2 = 0;

      if(btnS_StdAndBmp == typeShow || btnS_NewStdAndBmp == typeShow) {
        _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &Rfg1, &Gfg1, &Bfg1,
                    &Rfg2, &Gfg2, &Bfg2);
        Prop->background = RGB(Rfg1, Gfg1, Bfg1);
        pb->bgPress = RGB(Rfg2, Gfg2, Bfg2);
        pb->otherBg[0] = Prop->background;
        pb->otherBg[1] = pb->bgPress;
        if(-1 == Rfg1)
          Prop->style = Property::TRANSP;
        p = findNextParam(p, 6);
        if(!p)
          break;
        }

      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &Rfg1, &Gfg1, &Bfg1,
                    &Rfg2, &Gfg2, &Bfg2,
                    &idfont, &idMsg1, &idMsg2);

      Prop->foreground = RGB(Rfg1, Gfg1, Bfg1);
      pb->fgPress = RGB(Rfg2, Gfg2, Bfg2);
      pb->otherFg[0] = Prop->foreground;
      pb->otherFg[1] = pb->fgPress;

      Prop->idFont = idfont - ID_INIT_FONT;

      pb->allText[0] = pb->normalText = 0;
      pb->allText[1] = pb->pressedText = 0;
      if(idMsg1) {
        LPCTSTR p2 = set.getString(idMsg1);
        if(p2) {
          TCHAR buff[1000];
          translateFromCRNL(buff, p2);
          pb->normalText = pb->allText[0] = str_newdup(buff);
          }
        }
      if(idMsg2) {
        LPCTSTR p2 = set.getString(idMsg2);
        if(p2) {
          TCHAR buff[1000];
          translateFromCRNL(buff, p2);
          pb->pressedText = pb->allText[1] = str_newdup(buff);
          }
        }

      if(btnS_NewStdAndBmp == pb->type1) {
        p = findNextParamTrim(p, 9);
        if(!p)
          break;
        uint id2 = _ttoi(p);
        if(id2) {
          LPCTSTR p2 = set.getString(id2);
          if(p2)
            unformatColor(p2, pb->colorVal, pb->otherFg, pb->otherBg);
          }
        if(!(p = findNextParamTrim(p)))
          break;
        id2 = _ttoi(p);
        loadFullPrph(id2, set, pb->DataPrf[0]);

        if(!(p = findNextParamTrim(p)))
          break;
        id2 = _ttoi(p);

        if(id2) {
          for(uint i = 2;; ++id2, ++i) {
            LPCTSTR p2 = set.getString(id2);
            if(!p2)
              break;
            TCHAR buff[1000];
            translateFromCRNL(buff, p2);
            pb->allText[i] = str_newdup(buff);
            }
          }

        if(!(p = findNextParamTrim(p)))
          break;
        id2 = _ttoi(p);
        loadFullPrph(id2, set, pb->DataPrf[1]);

        if(!(p = findNextParamTrim(p)))
          break;
        id2 = _ttoi(p);
        loadFullPrph(id2, set, pb->DataPrf[2]);

        if(!(p = findNextParamTrim(p)))
          break;
        pb->Flags.flag = _ttoi(p);
        if(!pb->Flags.styleBmpDim && !pb->Flags.fixedBmpDim)
          pb->Flags.styleBmpDim = POwnBtnImageStd::eisScaled;


        if(!(p = findNextParamTrim(p)))
          break;
        loadValue(p, set, pb->textVal);

        if(!(p = findNextParamTrim(p)))
          break;
        loadValue(p, set, pb->bmpVal);
        }
      }
    } while(false);
  int pswLevel = 0;

  manageObjId moi(id, ID_INIT_VAR_BTN);

  if(btnAction == Prop->type2) {
    p = set.getString(moi.getFirstExtendId());
    uint idPrph = 0;
    int idAction = 0;
    if(p)
      idPrph = _ttoi(p);
    p = findNextParam(p, 1);
    if(p)
      idAction = _ttoi(p);

    if(ID_PRPH_4_KEYB == idPrph) { // keyb
      Prop->perif = 0;
      TCHAR buff[50];
      wsprintf(buff, _T("%d,%d"), getKeyb4TouchScreenCode(), idAction);
      pb->functionLink = str_newdup(buff);
      }
    else if(p) {
      Prop->perif = 0;
      if(idAction < FIRST_ID_FUNCTION_KEY) {
        Prop->perif = idAction;
//        idAction += ID_INIT_ACTION_MENU;
        }
      else {
        p = set.getString(idAction);
        if(p) {
          if(isSendOneData(p))
            pb->functionLink = getSendOneByBtn(p, set);
          else if(isSendToggleData(p))
            pb->functionLink = getSendToggleByBtn(p, set);
          else if(isChgPsw(p)) {
            pb->functionLink = str_newdup(p);
            p = findNextParam(p);
            if(p) {
              pswLevel = _ttoi(p);
              if(pswLevel == ONLY_CONFIRM)
                pswLevel = 4;
              }
            }
          else
            pb->functionLink = str_newdup(p);
          }
        }
      }
    loadOnlyVisVar(set, moi.getSecondExtendId());
    Prop->pswLevel = pswLevel;

    }
  else
    loadVar(id, set);

  if(btnModeless == pb->type2) {
    int idOpen = moi.getThirdExtendId();
    p = set.getString(idOpen);
    if(p)
      pb->modelessName = str_newdup(p);
    }


  else if(btnOpenPageByBit == pb->type2) {
    int idOpen = moi.getThirdExtendId();
    p = set.getString(idOpen);
    while(p) {
      uint idPage = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      uint psw = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      uint ix = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      TCHAR t[_MAX_PATH];
      if(ix && aOpenfileRecipe == idPage)
        wsprintf(t, _T("%d,%d,"), aOpenfileRecipeDan, psw);
      else
        wsprintf(t, _T("%d,%d,"), idPage, psw);
      _tcscat_s(t, p);

      pb->modelessName = str_newdup(t);
      break;
      }
    }
  Prop->negative = toBool(Prop->nDec);
  return true;
}
//-----------------------------------------------------------
void svmObjButton::addInfoAd4Base(manageInfoAd4& set)
{
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);
  if(!pb || !isFnz4Offs(pb))
    baseClass::addInfoAd4Base(set);
  else {
    TCHAR t[_MAX_PATH + 10];
    _tcscpy_s(t, pb->functionLink);
    LPTSTR pt = t;
    while(*pt && _T('?') != *pt)
      ++pt;
    perifForBtnAction data;
    perifForBtnAction::fromString(data, pt);
    if(!data.U.s.prf)
      return;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, getTextInfoBase());
    ia4->prph = data.U.s.prf;
    ia4->addr = data.addr;
    ia4->type = data.U.s.type;
    ia4->nBit = data.U.s.nbit;
    ia4->offs = data.U.s.offs;
    set.add(ia4);
    }
}
//-----------------------------------------------------------
static bool isFnz4Action(PropertyBtn* pb)
{
  return btnAction == pb->type2 && !pb->perif && pb->functionLink;
}
//-----------------------------------------------------------
static bool isFnz4Fnz(PropertyBtn* pb)
{
  return btnAction == pb->type2 && pb->perif;
}
//-----------------------------------------------------------
static bool isFnz4PageByBit(PropertyBtn* pb)
{
  switch(pb->type2) {
    case btnModeless:
    case btnOpenPageByBit:
      break;
    default:
      return false;
    }
  return true;
}
//-----------------------------------------------------------
static LPCTSTR getLastParam(LPCTSTR p)
{
  LPCTSTR p2 = findNextParamTrim(p);
  while(p2) {
    p = p2;
    p2 = findNextParamTrim(p2);
    }
  return p;
}
//-----------------------------------------------------------
void svmObjButton::fillTips(LPTSTR tips, int size)
{
  PropertyBtn* pb = dynamic_cast<PropertyBtn*>(Prop);
  if(!pb) {
    baseClass::fillTips(tips, size);
    return;
    }
  bool isOffs = isFnz4Offs(pb);
  bool isAction = isFnz4Action(pb);
  bool isPageByBit = isFnz4PageByBit(pb);
  bool isFz = isFnz4Fnz(pb);
  if(!isOffs && !isAction && !isPageByBit && !isFz) {
    baseClass::fillTips(tips, size);
    return;
    }

  if(isFz) {
    int n = pb->perif;
    TCHAR f = _T('F');
    if(n > 20) {
      n -= 20;
      f = _T('S');
      }
    wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nAzione -> %c%d"),
          getObjName(), getId(), zOrder, f, n);
    }
  else if(isOffs) {
    TCHAR t[_MAX_PATH + 10];
    _tcscpy_s(t, pb->functionLink);
    LPTSTR pt = t;
    while(*pt && _T('?') != *pt)
      ++pt;
    perifForBtnAction data;
    perifForBtnAction::fromString(data, pt);
    if(isAction) {
      LPCTSTR action = getTextAction(_ttoi(pb->functionLink));
      wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nPerif=%d, Addr=%d, Type=%s\r\nBits=%d, Offset=%d\r\nAzione -> %s=%s"),
          getObjName(), getId(), zOrder,
          data.U.s.prf, data.addr, getTypeString(data.U.s.type),
          data.U.s.nbit, data.U.s.offs, action, getLastParam(pb->functionLink));
      }
    else {
      wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nPerif=%d, Addr=%d, Type=%s\r\nBits=%d, Offset=%d"),
          getObjName(), getId(), zOrder,
          data.U.s.prf, data.addr, getTypeString(data.U.s.type),
          data.U.s.nbit, data.U.s.offs);
      }
    }
  else {
    if(isPageByBit) {
      wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nPerif=%d, Addr=%d, Type=%s\r\nBits=%d, Offset=%d\r\nApri Pagina (%s)=%s"),
          getObjName(), getId(), zOrder,
          pb->perif, pb->addr, getTypeString(pb->typeVal),
          pb->nBits, pb->offset,
          btnModeless == pb->type2 ? _T("Modeless") : _T("da Bit"),
          getLastParam(pb->modelessName));
      }
    else {
      LPCTSTR action = getTextAction(_ttoi(pb->functionLink));
      wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\nAzione -> %s=%s"),
          getObjName(), getId(), zOrder, action, getLastParam(pb->functionLink));
      }
    }
  addTipsVis(tips, size);
}
