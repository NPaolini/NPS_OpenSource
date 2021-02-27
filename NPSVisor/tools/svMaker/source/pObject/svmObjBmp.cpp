//-------------------- svmObjBmp.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "common.h"
#include "p_File.h"
#include "pPanel.h"
#include "pModDialog.h"
#include "svmObjBmp.h"
#include "svmDefObj.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
static PropertyBmp staticPT;
//-----------------------------------------------------------
svmObjBmp::svmObjBmp(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oBITMAP, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjBmp::svmObjBmp(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oBITMAP, r)
{
  init();
}
//-----------------------------------------------------------
void svmObjBmp::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  const int dim = 40;
#if 1
  Prop->Rect.right = Prop->Rect.left + dim;
  Prop->Rect.bottom = Prop->Rect.top + dim;
#else
  if(!staticPT.Rect.Width()) {
    staticPT.Rect = Prop->Rect;
    staticPT.Rect.right = staticPT.Rect.left + dim;
    staticPT.Rect.bottom = staticPT.Rect.top + dim;
    }
  Prop->Rect.right = Prop->Rect.left + staticPT.Rect.Width();
  Prop->Rect.bottom = Prop->Rect.top + staticPT.Rect.Height();
#endif
}
//-----------------------------------------------------------
Property* svmObjBmp::allocProperty() { return new PropertyBmp; }
//-----------------------------------------------------------
void svmObjBmp::DrawObject(HDC hdc)
{
#if 1
//  staticPT.Rect = getRect();
#else
  PRect r = getRect();
  r.right = r.left + REV__X(r.Width());
  r.bottom = r.top + REV__Y(r.Height());
  staticPT.Rect = r; //getRect__R();
#endif
  PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
  const PVect<LPCTSTR>& names = pb->getNames();
  if(names.getElem() > 0) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, names[0]);
    PBitmap bmp(name);
    if(bmp.isValid()) {
      PRect r = getRect();
      SIZE sz = bmp.getSize();
      double scaleX = r.Width();
      scaleX /= (double)sz.cx;
      double scaleY = r.Height();
      scaleY /= (double)sz.cy;
      double scale = scaleX > scaleY ? scaleY : scaleX;

      POINT pt = { r.left, r.top };
#if 1
      pt.x = REV__X(pt.x);
      pt.y = REV__Y(pt.y);

      pt.x = R__X(pt.x);
      pt.y = R__Y(pt.y);

      PBitmap* aBmp[] = { &bmp };
      PSeqTraspBitmap tb(getParent(), aBmp, 1, pt);
      tb.setScale(scale);
      if(!(Prop->style & Property::TRANSP))
        tb.setTransp(false);
      tb.Draw(hdc);
#else
      if(Prop->style & Property::TRANSP) {
        PTraspBitmap tb(getParent(), &bmp, pt);
        tb.setScale(scale);
        tb.Draw(hdc);
        }
      else {
        sz.cx = r.Width() * scale / scaleX;
        sz.cy = r.Height() * scale / scaleY;
        bmp.draw(hdc, pt, sz);
        }
#endif
      return;
      }
    }
  PPanel panel(getRect(), GetSysColor(COLOR_BTNFACE), PPanel::UP_FILL);
  panel.draw(hdc);
}
//-----------------------------------------------------------
void svmObjBmp::drawHandleUI(HDC hdc)
{
/*
  if(useMove(Prop)) {
    PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
    if(pb && pb->MoveInfo.isEnabled() && pb->MoveInfo.rect.Width() && pb-MoveInfo.rect.Height()) {
      PRect rtAll(pb->MoveInfo.rect);
      if(Inside == whichPoint || Extern == whichPoint)
        rtAll.Offset(RectDragging.left - RectDragging.right,
                 RectDragging.top - RectDragging.bottom);

      else
        rtAll.Offset(RectDragging.left - Prop->Rect.left,
                 RectDragging.top - Prop->Rect.top);
      getDataProject().calcToScreen(rtAll);

      int oldRop = SetROP2(hdc, R2_XORPEN);
      HPEN pen = CreatePen(PS_SOLID, 1, getXoredColor());
      HGDIOBJ oldPen = SelectObject(hdc, pen);
      MoveToEx(hdc, rtAll.left, rtAll.top, 0);
      LineTo(hdc, rtAll.right, rtAll.top);
      LineTo(hdc, rtAll.right, rtAll.bottom);
      LineTo(hdc, rtAll.left, rtAll.bottom);
      LineTo(hdc, rtAll.left, rtAll.top);

      const int D_ARROW = 3;
      int x1 = rtAll.left;
      int x2 = x1 + rtAll.Width() / 4;

      int y1 = rtAll.top;
      int y2 = y1 + rtAll.Height() / 4;

      switch(pb->mInfo->dir) {
        case 0:
          break;
        case 1:
          x1 = rtAll.right;
          x2 = x1 - rtAll.Width() / 4;
          break;
        case 2:
          y1 = rtAll.bottom;
          y2 = y1 - rtAll.Height() / 4;
          break;
        case 3:
          x1 = rtAll.right;
          x2 = x1 - rtAll.Width() / 4;
          y1 = rtAll.bottom;
          y2 = y1 - rtAll.Height() / 4;
          break;
        }
      MoveToEx(hdc, x1, y1, 0);
      LineTo(hdc, x2, y2);
      Arc(hdc, x2 - R__X(D_ARROW), y2 - R__Y(D_ARROW), x2 + R__X(D_ARROW), y2 + R__Y(D_ARROW),
          0,  0,  0,  0);
      DeleteObject(SelectObject(hdc, oldPen));
      SetROP2(hdc, oldRop);
      }
    }
*/
  baseClass::drawHandleUI(hdc);
  }
//-----------------------------------------------------------
bool svmObjBmp::endDrag(HDC hdc, const POINT& pt)
{
  PRect r = Prop->Rect;
  bool modified = baseClass::endDrag(hdc, pt);
/*
  if(useMove(Prop)) {
    PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
    if(pb && pb->MoveInfo.isEnabled() && pb->MoveInfo.rect.Width() && pb->MoveInfo.rect.Height()) {
      PRect& rM = pb->MoveInfo.rect;
      switch(whichPoint) {
        case Inside:
        case Extern:
          rM.Offset(RectDragging.left - RectDragging.right, RectDragging.top - RectDragging.bottom);
          break;
        default:
          rM.Offset(RectDragging.left - r.left, RectDragging.top - r.top);
          break;
        }
      }
    }
*/    
  return modified;
}
//-----------------------------------------------------------
void svmObjBmp::DrawTitle(HDC hdc)
{
  baseClass::DrawTitle(hdc);

  PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
  const PVect<LPCTSTR>& names = pb->getNames();
  if(names.getElem() <= 0) {
    PRect r = getRect();
    int h = r.Height() / 3;
    HFONT hf = D_FONT(h, 0, fBOLD, _T("arial"));
    HGDIOBJ oldFont = SelectObject(hdc, hf);


    int oldMode = SetBkMode(hdc, TRANSPARENT);
    int oldColor = SetTextColor(hdc, 0);

    DrawText(hdc, _T("BMP"), -1, r, DT_CENTER | DT_WORDBREAK | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hf);

    SetTextColor(hdc, oldColor);
    SetBkMode(hdc, oldMode);
    }
}
//-----------------------------------------------------------
smartPointerConstString svmObjBmp::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjBmp::getRectTitle(HDC, LPCTSTR /*title*/) const
{
#if 1
  return Prop->Rect;
#else
  PRect r(Prop->Rect);
  int w = r.Width() / 3;
  r.Inflate(-w, 0);
  return r;
#endif
}
//-----------------------------------------------------------
svmObject* svmObjBmp::makeClone()
{
  svmObjBmp* obj = new svmObjBmp(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjBmp::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int svmObjBmp::getIdType() const
{
  return baseClass::getIdType();
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjBmp::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyBmp*>(Prop))->cloneMinusProperty(staticPT);
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
    if(IDOK == svmDialogBmp(this, Prop, getParent()).modal()) {
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
/*
bool svmObjBmp::saveVarMoveInfo(P_File& pf, uint baseId, const bmpMoveInfo::varInfo& vI)
{
  baseId = manageObjId(baseId).getFirstExtendId();
  TCHAR buff[100];
  wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"),
              baseId,
              vI.perif,
              vI.addr,
              vI.typeVal,
              vI.normaliz);
  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
void svmObjBmp::loadVarMoveInfo(uint id, setOfString& set, bmpMoveInfo::varInfo& vI)
{
  int perif = 0;
  int addr = 0;
  int typeVal = 0;
  int normaliz = 0;

  id = manageObjId(id).getFirstExtendId();
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &perif, &addr, &typeVal, &normaliz);

  vI.perif = perif;
  vI.addr = addr;
  vI.typeVal = typeVal;
  vI.normaliz = normaliz;
}
//-----------------------------------------------------------
*/
//-----------------------------------------------------------
bool svmObjBmp::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_BMP);
  uint id = moi.calcAndSetExtendId(order);
  PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
  if(!pb)
    return false;
  Prop->nDec = Prop->negative;

  int scale = 0;

  const PVect<LPCTSTR>& names = pb->getNames();
  int nBmp = names.getElem();
  if(nBmp > 0) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, names[0]);
    PBitmap bmp(name);
    if(bmp.isValid()) {
      PRect r = getRect();
      SIZE sz = bmp.getSize();
      if(isMetafile(name))
        scale = -REV__X(sz.cx);
      else {
        double scaleX = REV__X(r.Width());
        scaleX /= (double)sz.cx;
        scale = (int)(scaleX * 1000.0 + 0.45);
        }
      }
    }

  TCHAR buff[1000];
  dual* dualBmp = getDualBmp4Btn();
  svmObjCount& objCount = getObjCount();

  PVect<int> idbmp;
  idbmp.setDim(nBmp);
  for(int i = 0; i < nBmp; ++i)
    idbmp[i] = dualBmp->addAndGetId(names[i]);

  int flag = Prop->type1;
  int neededBmp = 0;
  bool canRemoveTransp = true;
  switch(flag) {
    case bFixed:
    case bFixedLamp:
      neededBmp = 1;
//      canRemoveTransp = false;
      break;

    case bOnOff:
    case bDualLamp:
      neededBmp = 2;
      break;

    case bVarSeq:
    case bFixSeq:
    case bAnimSeq:
      neededBmp = nBmp;
      break;
    }
  if(neededBmp > nBmp)
    neededBmp = nBmp;

//  if(Prop->negative)
//    flag += 1000;
  if(canRemoveTransp && !(Prop->style & Property::TRANSP))
    flag += 10000;
  if(useShowHide(Prop))
    flag += 100000;
/*
  if(useMove(Prop))
    flag += 1000000;
*/
  wsprintf(buff, _T("%d,%d,%d,%d,%d"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            scale, flag);
  for(int i = 0; i < neededBmp; ++i) {
    TCHAR t[50];
    wsprintf(t, _T(",%d"), idbmp[i]);
    _tcscat_s(buff, t);
    }
  if(pb->timeCycle) {
    TCHAR t[50];
    wsprintf(t, _T(",-%d"), pb->timeCycle);
    _tcscat_s(buff, t);
    }
  _tcscat_s(buff, _T("\r\n"));
  writeStringChkUnicode(pfCrypt, buff);
  saveVar(pfCrypt, id);
/*
  if(useMove(Prop) && pb->mInfo) {
    const PRect& r = pb->mInfo->rect;

    manageObjId moi(id, ID_INIT_VAR_BMP);
    uint id2 = moi.getThirdExtendId();

    wsprintf(buff, _T("%d,%d,%d,%d,%d"),
        id2,
        REV__X(r.left),
        REV__Y(r.top),
        REV__X(r.right),
        REV__Y(r.bottom)
        );
//    svmObjCount& objCount = getObjCount();
    TCHAR t[50];
    for(int i = 0; i < bmpMoveInfo::MAX_MOVE_INFO; ++i) {
      int code = objCount.getIdCount(oEDIT);
      manageObjId moi2(0, ID_INIT_VAR_EDI);
      code = moi2.calcBaseExtendId(code);
      wsprintf(t, _T(",%d"), code);
      _tcscat_s(buff, t);
      saveVarMoveInfo(pfCrypt, code, pb->mInfo->vI[i]);
      }
    wsprintf(t, _T(",%d\r\n"), pb->mInfo->dir);
    _tcscat_s(buff, t);
    writeStringChkUnicode(pfCrypt, buff);
    }
*/
  return baseClass::save(pfCrypt, pfClear, id);
}
/*
        id+900,x1,y1,x2,y2,code_X_min,code_Y_min,code_X_max,code_Y_max,code_X_curr,code_Y_curr,dir

        dove x1,y1,x2,y2 rappresentano il rettangolo entro cui si muove,
        code_?_??? rapresenta un id simile ai campi di edit o testo abbinato a variabile,
        si può scrivere per es. 4010 e in 4310 immettere i valori di periferica,indirizzo ecc.
        Non c'è bisogno di attivare il codice 4010 se non necessario, basta che sia presente
        il codice 4310.
*/
//-----------------------------------------------------------
// usata esternamente (da btn, led, XMeter)
LPCTSTR removePath(LPCTSTR path)
{
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i)
    if('\\' == path[i] || '/' == path[i])
      return path + i + 1;
  return path;
}
//-----------------------------------------------------------
bool svmObjBmp::load(uint id, setOfString& set)
{
  PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
  if(!pb)
    return false;

  int x = 0;
  int y = 0;
  int scale = 0;
  int flag = 0;

  PVect<int> idbmp;
  LPCTSTR p = set.getString(id);
  if(p) {
    _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &scale, &flag);
    p = findNextParam(p, 4);
    for(int i = 0; p; ++i) {
      int t = _ttoi(p);
      if(t > 0) {
        idbmp[i] = t;
        p = findNextParam(p, 1);
        }
      else
        break;
      }
    if(p) {
      int t = _ttoi(p);
      if(t < 0)
        pb->timeCycle = -t;
      }
    }

  if(!scale)
    scale = 1000;

  Prop->type2 = 0;
  if(flag >= 1000000) {
    setUseMove(Prop);
    flag -= 1000000;
    }
  if(flag >= 100000) {
    setUseShowHide(Prop);
    flag -= 100000;
    }
  Prop->style = Property::TRANSP;
  if(flag >= 10000) {
    Prop->style = 0;
    flag -= 10000;
    }
//  if(flag >= 1000) {
//    Prop->negative = true;
//    flag -= 1000;
//    }
  Prop->type1 = flag;

  PVect<LPCTSTR>& names = pb->getNames();
  dual* dualBmp;
  if(Linked)
    dualBmp = getDualBmp4BtnLinked();
  else
    dualBmp = getDualBmp4Btn();

  int nBmp = idbmp.getElem();
  int j = 0;
  for(int i = 0; i < nBmp; ++i) {
    LPCTSTR name = dualBmp->getTxt(idbmp[i]);
    if(name)
#if 1
      names[j++] = str_newdup(getOnlyName(name, true));
#else
      names[j++] = str_newdup(removePath(name));
#endif
    }

  int w = 30;
  int h = 30;

  if(j > 0) {
    TCHAR name[_MAX_PATH];
    makeNameImage(name, names[0]);
    PBitmap bmp(name);
    if(bmp.isValid()) {
      PRect r = getRect();
      SIZE sz = bmp.getSize();
      double tScale = scale;
      if(scale < 0) {
        w = -scale;
        double t = w;
        t /= sz.cx;
        h = (int)(sz.cy * t + 0.45);
        }
      else {
        double tScale = scale;
        tScale /= 1000;
        w = (int)(tScale * sz.cx + 0.45);
        h = (int)(tScale * sz.cy + 0.45);
        }
      }
    }

  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  loadVar(id, set);
  Prop->negative = toBool(Prop->nDec);

// va lasciata per caricare la gestione movimento da progetti vecchi
  if(useMove(Prop)) {
    manageObjId moi(id, ID_INIT_VAR_BMP);
    uint id2 = moi.getThirdExtendId();
#if 1
    Prop->MoveInfo.load(set, id2);
#else
    p = set.getString(id2);
    if(p) {
      pb->mInfo = new bmpMoveInfo;
      int x1 = 0;
      int y1 = 0;
      int x2 = 0;
      int y2 = 0;
      _stscanf_s(p, _T("%d,%d,%d,%d"), &x1, &y1, &x2, &y2);
      // uno in meno, verrà prelevato all'inizio del ciclo for
      p = findNextParam(p, 4 - 1);

      pb->mInfo->rect = PRect(R__X(x1), R__Y(y1), R__X(x2), R__Y(y2));
      for(int i = 0; i < bmpMoveInfo::MAX_MOVE_INFO; ++i) {
        p = findNextParam(p, 1);
        if(!p)
          break;
        loadVarMoveInfo(_ttoi(p), set, pb->mInfo->vI[i]);
        }
      p = findNextParam(p, 1);
      if(p)
        pb->mInfo->dir = _ttoi(p);
      }
#endif
    }
  return true;
}
//-----------------------------------------------------------
void svmObjBmp::addInfoAd4Other(manageInfoAd4& set)
{
/*
  PropertyBmp* pb = dynamic_cast<PropertyBmp*>(Prop);
  if(!pb->mInfo)
    return;
  LPCTSTR info[] = { _T("minX"), _T("minY"), _T("maxX"), _T("maxY"), _T("currX"), _T("currY") };
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const bmpMoveInfo::varInfo& vI =  pb->mInfo->vI[i];
    if(!vI.perif)
      continue;
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, info[i]);
    ia4->prph = vI.perif;
    ia4->addr = vI.addr;
    ia4->type = vI.typeVal;
    set.add(ia4);
    }
*/
}


