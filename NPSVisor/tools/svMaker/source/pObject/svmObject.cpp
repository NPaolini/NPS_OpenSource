//-------------------- svmObject.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "svmObject.h"
#include "svmHistory.h"
#include "pPanel.h"
#include "p_File.h"
#include "svmMainClient.h"
#include "pTextPanel.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
uint manageObjId::calcBase(uint id)
{
  if(OFFS_INIT_VAR_EXT <= id)
    return ((id - OFFS_INIT_VAR_EXT) / 1000) * 100 + 1;
  if(OFFS_INIT_SIMPLE_BMP <= id)
#if 1
    return ID_INIT_BMP;
#else
    return ((id - OFFS_INIT_SIMPLE_BMP + ID_INIT_BMP) / 100) * 100 + 1;
#endif
  if(id < 1000)
    return (id / 100) * 100 + 1;
  return (id / 1000) * 1000 + 1;
}
//----------------------------------------------------------------------------
uint manageObjId::calcBaseExtendId(uint ix)
{
  switch(getBaseId()) {
    case ID_INIT_BARGRAPH:
      if(ix < GRAPH_VISIBILITY_OFFS)
        return getBaseId() + ix;
      return ix + ADD_INIT_GRAPH + 1;

    case ID_INIT_BMP:
      if(ix < ID_INIT_BARGRAPH - ID_INIT_BMP)
        return ix + getBaseId();
      return ix + OFFS_INIT_SIMPLE_BMP + 1 - (ID_INIT_BARGRAPH - ID_INIT_BMP);
    }
  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(ix < idSec)
    return ix + getBaseId();

  return ix + (getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT - ADD_INIT_VAR;
}
//----------------------------------------------------------------------------
uint manageObjId::calcAndSetExtendId(uint ix)
{
  Id = calcBaseExtendId(ix);
  return Id;
}
//----------------------------------------------------------------------------
uint manageObjId::getFirstExtendId()
{
  if(ID_INIT_BARGRAPH == getBaseId()) {
    if(getId() < getBaseId() + GRAPH_VISIBILITY_OFFS)
      return getId() + GRAPH_VISIBILITY_OFFS;
    return getId() - getBaseId() + ADD_INIT_GRAPH + 1;
    }
  if(ID_INIT_BMP == getBaseId()) {
    int ix = getId();
    if(ix >= OFFS_INIT_SIMPLE_BMP)
      ix -= OFFS_INIT_SIMPLE_BMP - 1 - ID_INIT_BMP;
    return ix + ADD_INIT_SIMPLE_BMP;
    }

  if(ID_INIT_SIMPLE_PANEL == getBaseId())
    return getId() + ADD_INIT_VAR * 2;

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR;

  return getId() + ADD_INIT_VAR_EXT;
//  return getId() + (getBaseId() - 1) * 9 + ADD_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
uint manageObjId::getSecondExtendId()
{
  if(ID_INIT_BMP == getBaseId())
    return getFirstExtendId();

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
#define MAX_THIRD_OFFSET MAX_NORMAL_OPEN_MODELESS
//----------------------------------------------------------------------------
uint manageObjId::getThirdExtendId()
{
  if(ID_INIT_BMP == getBaseId())
    return getFirstExtendId();
  if(ID_INIT_VAR_BTN == getBaseId()) {
    if(getId() < MAX_NORMAL_OPEN_MODELESS)
      return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;
    // se l'id è già esteso occorre ricavare l'indice aumentato di ADD_INIT_VAR prima di sommarlo all'offset
    if(getId() > OFFS_INIT_VAR_EXT)
      return getId() - ((getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT) + OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN + ADD_INIT_VAR;

    return getId() + OFFSET_BTN_OPEN_MODELES;
    }

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : MAX_THIRD_OFFSET;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT + ADD_INIT_THIRD_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
manageInfoAd4::~manageInfoAd4()
{
  flushPV(Set);
}
//----------------------------------------------------------------------------
#define RET_CMP(v1, v2) do {\
  diff = int(v1 - v2); \
  if(diff) return diff;\
  } while(false)
//----------------------------------------------------------------------------
#define RET_CMP_STR(v1, v2) do {\
  diff = _tcscmp(v1, v2); \
  if(diff) return diff;\
  } while(false)
//----------------------------------------------------------------------------
typedef infoAd4* pInfoAd4;
//----------------------------------------------------------------------------
int cmpInfoAd4(const pInfoAd4& v1, const pInfoAd4& v2)
{
  int diff;
#if 1
  RET_CMP_STR(v1->objName, v2->objName);
  RET_CMP(v1->id, v2->id);
  RET_CMP(v1->prph, v2->prph);
  RET_CMP(v1->addr, v2->addr);
  RET_CMP(v1->type, v2->type);
  RET_CMP(v1->nBit, v2->nBit);
  RET_CMP_STR(v1->infoVar, v2->infoVar);
#else
  RET_CMP(v1->prph, v2->prph);
  RET_CMP(v1->addr, v2->addr);
  RET_CMP(v1->type, v2->type);
  RET_CMP(v1->nBit, v2->nBit);
  RET_CMP_STR(v1->objName, v2->objName);
  RET_CMP_STR(v1->infoVar, v2->infoVar);
  RET_CMP(v1->id, v2->id);
#endif
  return 1;
}
//----------------------------------------------------------------------------
void manageInfoAd4::add(infoAd4* iad4)
{
  Set.insertEx(iad4, cmpInfoAd4);
//  Set[Set.getElem()] = iad4;
}
//-----------------------------------------------------------
const int minBox = 5;
//-----------------------------------------------------------
int svmObject::howVerifyOutScreen = 0;
//-----------------------------------------------------------
// no #define RND rand() | 129
//#define RND rand() + 127
#define RND rand()
//#define RND rand() | ((rand() & 1) << 8)
#define MAKE_RND_RGB RGB(RND, RND, RND)
//-----------------------------------------------------------
//#define MAKE_RND_RGB (rand() | ((long)rand() << 16)) & 0xffffff
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, int id, int idType, int x1, int y1, int x2, int y2) :
    Rect(DATA_NOT_USED == x1 ? 10 : x1, DATA_NOT_USED == y1 ? 10 : y1,
         DATA_NOT_USED == x2 ? x1 + 110 : x2, DATA_NOT_USED == y2 ? y1 + 30 : y2),
    RectDragging(Rect), Id(id), //Font(0),
    Selected(svmObject::notSel), whichPoint(svmObject::Extern), Par(par),
    Prop(0), oldProp(0), IdType(idType), zOrder(0), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)

{ }
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, int id, int idType, const PRect& r) :
    Rect(r), RectDragging(r), //Font(0),
    Id(id), Selected(svmObject::notSel),
    whichPoint(svmObject::Extern), Par(par),
    Prop(0), oldProp(0), IdType(idType), zOrder(0), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)
{ }
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, setOfString& sos, int id, int idType, int idCode) :
      Id(id), Selected(svmObject::notSel),
      whichPoint(svmObject::Extern), Par(par),
      Prop(0), oldProp(0), IdType(idType), zOrder(0), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)
{ }
//-----------------------------------------------------------
void svmObject::standBySel(bool add)
{
  baseClass::standBySel(add);
  if((svmObject::notSel == Selected || svmObject::signalRemove == Selected) && add)
    Selected = svmObject::signalAdd;
  else if(svmObject::yesSel == Selected && !add)
    Selected = svmObject::signalRemove;
}
//-----------------------------------------------------------
void svmObject::setAnchor(svmObject::typeOfAnchor anchor, const POINT& pt)
{
  if(whichPoint == anchor)
    return;
  RectDragging = Prop->Rect;
  if(Extern == anchor || Inside == anchor) {
    RectDragging.left = pt.x;
    RectDragging.top = pt.y;
    RectDragging.right = pt.x;
    RectDragging.bottom = pt.y;
    }
  whichPoint = anchor;
}
//-----------------------------------------------------------
void svmObject::drawSel(HDC hdc)
{
  uint old = whichPoint;
  whichPoint = Top;
  RectDragging = Prop->Rect;
  drawHandleUI(hdc);
  whichPoint = (svmObject::typeOfAnchor)old;
}
//-----------------------------------------------------------
void svmObject::Select(HDC hdc)
{
  baseClass::Select(hdc);
  if(svmObject::notSel == Selected || svmObject::signalAdd == Selected) {
    drawSel(hdc);
    }
  Selected = yesSel;
}
//-----------------------------------------------------------
void svmObject::Unselect(HDC hdc)
{
  baseClass::Unselect(hdc);
  if(svmObject::yesSel == Selected || svmObject::signalRemove == Selected) {
    drawSel(hdc);
    }
  Selected = notSel;
}
//-----------------------------------------------------------
void svmObject::setLock(bool set)
{
  if(locked != set) {
    locked = set;
    baseClass::setLock(set);
    }
}
//-----------------------------------------------------------
void svmObject::setVisible(bool set)
{
  if(Visible != set) {
    Visible = set;
    baseClass::setVisible(set);
    }
}
//-----------------------------------------------------------
LPCTSTR getTypeString(uint ix)
{
  static LPCTSTR cbType[] = {
    _T(" 0 - null"),
    _T(" 1 - bit"),
    _T(" 2 - byte"),
    _T(" 3 - word"),
    _T(" 4 - dword"),
    _T(" 5 - float"),
    _T(" 6 - int64"),
    _T(" 7 - real"),
    _T(" 8 - dateHour"),
    _T(" 9 - Date"),
    _T("10 - Hour"),
    _T("11 - char"),
    _T("12 - short"),
    _T("13 - long"),
    _T("14 - string"),
    };
  if(ix >= SIZE_A(cbType))
    ix = 0;
  return cbType[ix];
}
//-----------------------------------------------------------
void svmObject::addTipsVis(LPTSTR tips, int size)
{
  if(!Prop->visPerif)
    return;
  int len = _tcslen(tips);
//  size -= len;
  wsprintf(tips + len, _T("\r\n------ Visibilità ------\r\nPerif(%d), Addr(%d), nBit(%d), Offset(%d)%s"),
          Prop->visPerif, Prop->visAddr,
          Prop->visnBits, Prop->visOffset, Prop->visNegative ? _T(", Neg") : _T(" ")
          );

}
//-----------------------------------------------------------
void svmObject::addInfoAd4Vis(manageInfoAd4& set)
{
  if(!Prop->visPerif)
    return;
  infoAd4* ia4 = new infoAd4;
  ia4->id = getId();
  copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
  _tcscpy_s(ia4->infoVar, getTextInfoVis());
  ia4->prph = Prop->visPerif;
  ia4->addr = Prop->visAddr;
  ia4->type = 4;
  ia4->nBit = Prop->visnBits;
  ia4->offs = Prop->visOffset;
  set.add(ia4);
}
//-----------------------------------------------------------
void svmObject::addInfoAd4Move(manageInfoAd4& set)
{
  if(!Prop->MoveInfo.isEnabled())
    return;
  LPCTSTR info[] = { _T("minX"), _T("minY"), _T("maxX"), _T("maxY"), _T("currX"), _T("currY") };
  objMoveInfo* mInfo = Prop->MoveInfo.getObj();
  for(uint i = 0; i < SIZE_A(info); ++i) {
    const objMoveInfo::varInfo& vI = mInfo->vI[i];
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
}
//-----------------------------------------------------------
#define ROUND_4(v) (((v) / 4) + (((v) & 3) > 0))
//-----------------------------------------------------------
void svmObject::addInfoAd4Base(manageInfoAd4& set)
{
  if(!Prop->perif)
    return;
  uint nRepeat = 1;
  if(14 == Prop->typeVal)
    nRepeat = ROUND_4(Prop->nDec);
  for(uint i = 0; i < nRepeat; ++i) {
    infoAd4* ia4 = new infoAd4;
    ia4->id = getId();
    copyStrZ(ia4->objName, getObjName(), SIZE_A(ia4->objName) - 1);
    _tcscpy_s(ia4->infoVar, getTextInfoBase());
    ia4->prph = Prop->perif;
    ia4->addr = Prop->addr + i;
    ia4->type = Prop->typeVal;
    ia4->nBit = Prop->nBits;
    ia4->offs = Prop->offset;
    set.add(ia4);
    }
}
//-----------------------------------------------------------
static void makeStrConst(const Property& vi, LPTSTR buff)
{
  TCHAR t[128];
  makeConstValue(t, SIZE_A(t), vi.addr, PRPH_4_CONST, vi.typeVal);
  wsprintf(buff, _T("[Costante, Valore=%s]"), t);
}
//-----------------------------------------------------------
static void makeStrVar(const Property& vi, LPTSTR buff)
{
  wsprintf(buff, _T("--\r\nPerif=%d, Addr=%d, Type=%s\r\nBits=%d, Offset=%d, Norm=%d\r\n--"),
    vi.perif, vi.addr, getTypeString(vi.typeVal), vi.nBits, vi.offset, vi.normaliz);
}
//-----------------------------------------------------------
void svmObject::fillTips(LPTSTR tips, int size)
{
  TCHAR buff[256];
  if(PRPH_4_CONST == Prop->perif)
    makeStrConst(*Prop, buff);
  else
    makeStrVar(*Prop, buff);

  wsprintf(tips, _T("Obj -> %s\r\nid=%d, zOrder=%d\r\n%s\r\nMove=%s"),
          getObjName(), getId(), zOrder, buff,
          Prop->MoveInfo.isEnabled() ? _T("ON") : _T("OFF"));
  addTipsVis(tips, size);
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObject::pointIn(const POINT& pt) const
{
  int x = pt.x;
  int y = pt.y;
  if(x < Prop->Rect.left || y < Prop->Rect.top)
    return Extern;
  if(x > Prop->Rect.right || y > Prop->Rect.bottom)
    return Extern;

  if(getGroup() && useBlock())
    return Inside;

  PRect rt(0, 0, minBox, minBox);

  int w = Prop->Rect.Width();
  int h = Prop->Rect.Height();

  int x1 = Prop->Rect.left + 1;
  int x2 = x1 + (w - minBox) / 2;
  int x3 = Prop->Rect.left + w - minBox + 1;

  int y1 = Prop->Rect.top + 1;
  int y2 = y1 + (h - minBox) / 2;
  int y3 = Prop->Rect.top + h - minBox + 1;

  rt.MoveTo(x1, y1);
  if(rt.IsIn(pt))
    return LeftTop;

  rt.MoveTo(x3, y1);
  if(rt.IsIn(pt))
    return RightTop;

  rt.MoveTo(x3, y3);
  if(rt.IsIn(pt))
    return RightBottom;

  rt.MoveTo(x1, y3);
  if(rt.IsIn(pt))
    return LeftBottom;

  if(canDistorce() && !needFixedRatio()) {
    rt.MoveTo(x2, y1);
    if(rt.IsIn(pt))
      return Top;

    rt.MoveTo(x3, y2);
    if(rt.IsIn(pt))
      return Right;

    rt.MoveTo(x2, y3);
    if(rt.IsIn(pt))
      return Bottom;

    rt.MoveTo(x1, y2);
    if(rt.IsIn(pt))
      return Left;
    }
  return Inside;
}
//-----------------------------------------------------------
void svmObject::DrawRectTitle(HDC hdc, LPCTSTR title, PRect rect)
{
  HFONT Font = Prop->getFont(Linked);

  PTextFixedPanel txt(title, rect, Font, getProp()->foreground, PPanel::NO, 0, PTextPanel::NO3D);
  UINT align = Prop->alignText;

  UINT vAlign = Property::aMid == (align & Property::aMid) ? DT_VCENTER :
         Property::aBottom == (align & Property::aBottom) ? DT_BOTTOM :
                                     DT_TOP;

  align = Property::aLeft == (align & Property::aLeft) ? TA_LEFT :
         Property::aRight == (align & Property::aRight) ? TA_RIGHT :
                                     TA_CENTER;

  txt.setAlign(align);
  txt.setVAlign(vAlign);

  txt.draw(hdc);
}
//-----------------------------------------------------------
void svmObject::DrawTitle(HDC hdc)
{
  smartPointerConstString title = getTitle();
  if(&title && *&title) {
    if(_tcschr(&title, _T('\\'))) {
      LPTSTR buff = new TCHAR[_tcslen(&title) + 2];
      translateToCRNL(buff, &title);
      PRect rect = getRectTitle(hdc, buff);
      DrawRectTitle(hdc, buff, rect);
      delete []buff;
      }
    else {
      PRect rect = getRectTitle(hdc, &title);
      DrawRectTitle(hdc, &title, rect);
      }
    }
  TCHAR buff[500];
  PRect r = getRect();
//  PRect r = getRectTitle();
  r.Inflate(-2, -2);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  int oldColor = SetTextColor(hdc, getProp()->foreground);
  HFONT font = D_FONT(R__Y(8), 0, 0, _T("arial"));
  HGDIOBJ oldFont =  SelectObject(hdc, font);
  wsprintf(buff, _T("%d"), zOrder);
  DrawText(hdc, buff, -1, r, DT_RIGHT | DT_WORDBREAK | DT_BOTTOM | DT_SINGLELINE);

  wsprintf(buff, _T("%d"), getIdType() + 1);
  DrawText(hdc, buff, -1, r, DT_LEFT | DT_WORDBREAK | DT_BOTTOM | DT_SINGLELINE);

  SelectObject(hdc, oldFont);
  DeleteObject(font);

  SetTextColor(hdc, oldColor);
  SetBkMode(hdc, oldMode);

}
//-----------------------------------------------------------
void svmObject::DrawObjectLinked(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(!(rect & getRect()))
    return;

  DrawObject(hdc);
  DrawTitle(hdc);

  // disegnare una differenziazione
  PRect r(getRect());
  r.Inflate(-2, -2);
  HPEN pen = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff));

  HGDIOBJ oldPen = SelectObject(hdc, pen);
  for(int i = r.top; i < r.bottom; i += 2) {
    MoveToEx(hdc, r.left, i, 0);
    LineTo(hdc, r.right, i);
    }
  DeleteObject(SelectObject(hdc, oldPen));
}
//-----------------------------------------------------------
void svmObject::DrawOnlyObject(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(isLocked()) {
    DrawObjectLinked(hdc, rect);
    return;
    }
  if(rect & getRect()) {
    DrawObject(hdc);
    DrawTitle(hdc);
    }
}
//-----------------------------------------------------------
void svmObject::DrawOnlySelect(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(!(rect & getRect()))
    return;
  if(isSelected() != notSel && isSelected() != signalRemove) {
    Selected = signalAdd;
    Select(hdc);
    }
}
//-----------------------------------------------------------
//#define FRECT(hdc, r) Ellipse(hdc, r.left, r.top, r.right, r.bottom)
#define FRECT(hdc, r) Rectangle(hdc, r.left, r.top, r.right, r.bottom)
//-----------------------------------------------------------
void svmObject::drawBoxes(HDC hdc, const PRect& all)
{
  int w = all.Width();
  int h = all.Height();

  int x1 = all.left + 1;
  int x2 = x1 + (w - minBox) / 2;
  int x3 = all.left + w - minBox + 1;

  int y1 = all.top + 1;
  int y2 = y1 + (h - minBox) / 2;
  int y3 = all.top + h - minBox + 1;

  HBRUSH brush = CreateSolidBrush(getXoredColor());

  HGDIOBJ oldBrush = SelectObject(hdc, brush);

  PRect rt(0, 0, minBox, minBox);

  rt.MoveTo(x1, y1);
  FRECT(hdc, rt);

  rt.MoveTo(x3, y1);
  FRECT(hdc, rt);

  rt.MoveTo(x3, y3);
  FRECT(hdc, rt);

  rt.MoveTo(x1, y3);
  FRECT(hdc, rt);


  if(canDistorce()) {
    rt.MoveTo(x2, y1);
    FRECT(hdc, rt);

    rt.MoveTo(x3, y2);
    FRECT(hdc, rt);

    rt.MoveTo(x2, y3);
    FRECT(hdc, rt);

    rt.MoveTo(x1, y2);
    FRECT(hdc, rt);
    }
  SelectObject(hdc, oldBrush);
  DeleteObject(brush);
}
//-----------------------------------------------------------
void svmObject::drawHandleUI(HDC hdc)
{
  wrapMoveInfo& MoveInfo = Prop->MoveInfo;
  if(Prop->MoveInfo.isEnabled()) {
    PRect rtAll(RectDragging);
    if(Inside == whichPoint || Extern == whichPoint)
      rtAll = Prop->Rect;

    objMoveInfo* mInfo = Prop->MoveInfo.getObj();
    if(mInfo->absoluteSize) {
      rtAll.right = rtAll.left + mInfo->sz.cx;
      rtAll.bottom = rtAll.top + mInfo->sz.cy;
      }
    else {
      rtAll.right += mInfo->sz.cx;
      rtAll.bottom += mInfo->sz.cy;
      }
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

    switch(mInfo->dir) {
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

  PRect rtAll(RectDragging);
  if(Inside == whichPoint || Extern == whichPoint) {
    rtAll = Prop->Rect;
    rtAll.Offset(RectDragging.left - RectDragging.right,
                 RectDragging.top - RectDragging.bottom);
    }
  getDataProject().calcToScreen(rtAll);

  int oldRop = SetROP2(hdc, R2_XORPEN);

  {
    HPEN pen = CreatePen(PS_SOLID, 1, getXoredColor());
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    MoveToEx(hdc, rtAll.left, rtAll.top, 0);
    LineTo(hdc, rtAll.right, rtAll.top);
    LineTo(hdc, rtAll.right, rtAll.bottom);
    LineTo(hdc, rtAll.left, rtAll.bottom);
    LineTo(hdc, rtAll.left, rtAll.top);
    DeleteObject(SelectObject(hdc, oldPen));
  }

  HPEN pen = CreatePen(PS_NULL, 1, getXoredColor());
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  drawBoxes(hdc, rtAll);

  DeleteObject(SelectObject(hdc, oldPen));
  SetROP2(hdc, oldRop);
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObject::beginDrag(const POINT& pt)
{
  if(Selected != yesSel || !Visible || locked)
    return Extern;
  whichPoint = pointIn(pt);
  if(svmObject::Extern == whichPoint || svmObject::Inside == whichPoint) {
    RectDragging.left = pt.x;
    RectDragging.top = pt.y;
    RectDragging.right = pt.x;
    RectDragging.bottom = pt.y;
    }
  else
    RectDragging = Prop->Rect;
  firstPoint = pt;
  howVerifyOutScreen = 0;

  return whichPoint;
}
//-----------------------------------------------------------
void svmObject::getDelta(int& dx, int& dy, const POINT& pt)
{
  PRect r(getRect());
  switch(whichPoint) {
    case LeftTop:
      dx = r.right - pt.x;
      dy = r.bottom - pt.y;
      break;

    case Top:
    case RightTop:
      dx = pt.x - r.left;
      dy = r.bottom - pt.y;
      break;

    case Right:
    case RightBottom:
    case Bottom:
      dx = pt.x - r.left;
      dy = pt.y - r.top;
      break;

    case LeftBottom:
    case Left:
      dx = r.right - pt.x;
      dy = pt.y - r.top;
      break;
    }
}
//-----------------------------------------------------------
void svmObject::Drag(HDC hdc, const POINT& pt)
{
  drawHandleUI(hdc);
  calcNewPoint(pt);
  drawHandleUI(hdc);
}
//-----------------------------------------------------------
void svmObject::calcNewPoint(const POINT& pt)
{
  bool moving = Inside == whichPoint || Extern == whichPoint;
  bool fixRatio = needFixedRatio() || !canDistorce() && !moving;
  if(fixRatio) {
    if(moving) {
      int dx = abs(firstPoint.x - pt.x);
      int dy = abs(firstPoint.y - pt.y);
      if(dx > dy) {
        RectDragging.left = pt.x;
        RectDragging.top = firstPoint.y;
        RectDragging.bottom = firstPoint.y;
        }
      else {
        RectDragging.left = firstPoint.x;
        RectDragging.right = firstPoint.x;
        RectDragging.top = pt.y;
        }
      }
    else {
      PRect rect(getRect());
      int dx = 0;
      int dy = 0;
      getDelta(dx, dy, pt);
      if(!dx && !dy)
        RectDragging = getRect();
      else {
        double ratio = rect.Height();
        ratio /= rect.Width();
        double ratioTmp = dy;
        if(!dx)
          ratioTmp = ratio * 2;
        else
          ratioTmp /= dx;
        if(ratioTmp < 0)
          ratioTmp = -ratioTmp;

        switch(whichPoint) {
          case LeftTop:
            if(ratioTmp > ratio) {
              RectDragging.left = (int)(rect.right - (double)dy / ratio);
              RectDragging.top = pt.y;
              }
            else {
              RectDragging.left = pt.x ;
              RectDragging.top = (int)(rect.bottom - (double)dx * ratio);
              }
            break;

          case Top:
          case RightTop:
            if(ratioTmp > ratio) {
              RectDragging.right = (int)(rect.left + (double)dy / ratio);
              RectDragging.top = pt.y;
              }
            else {
              RectDragging.right = pt.x ;
              RectDragging.top = (int)(rect.bottom - (double)dx * ratio);
              }
            break;

          case Right:
          case RightBottom:
          case Bottom:
            if(ratioTmp > ratio) {
              RectDragging.right = (int)(rect.left + (double)dy / ratio);
              RectDragging.bottom = pt.y;
              }
            else {
              RectDragging.right = pt.x ;
              RectDragging.bottom = (int)(rect.top + (double)dx * ratio);
              }
            break;

          case LeftBottom:
          case Left:
            if(ratioTmp > ratio) {
              RectDragging.left = (int)(rect.right - (double)dy / ratio);
              RectDragging.bottom = pt.y;
              }
            else {
              RectDragging.left = pt.x ;
              RectDragging.bottom = (int)(rect.top + (double)dx * ratio);
              }
            break;
          }
        }
      }
    }
  else {
    switch(whichPoint) {
      case LeftTop:
      case Inside:
      case Extern:
        RectDragging.left = pt.x;
        RectDragging.top = pt.y;
        break;
      case Top:
        RectDragging.top = pt.y;
        break;
      case RightTop:
        RectDragging.right = pt.x;
        RectDragging.top = pt.y;
        break;
      case Right:
        RectDragging.right = pt.x;
        break;
      case RightBottom:
        RectDragging.right = pt.x;
        RectDragging.bottom = pt.y;
        break;
      case Bottom:
        RectDragging.bottom = pt.y;
        break;
      case LeftBottom:
        RectDragging.left = pt.x;
        RectDragging.bottom = pt.y;
        break;
      case Left:
        RectDragging.left = pt.x;
        break;
      }
    }
}
//-----------------------------------------------------------
void svmObject::beginDragSizeByKey()
{
  if(Selected != yesSel || !Visible || locked)
    return;
  RectDragging = Prop->Rect;
  whichPoint = Top;
  howVerifyOutScreen = 0;
}
//-----------------------------------------------------------
void svmObject::DragOnResizeByKey(HDC hdc, const POINT& pt)
{
  drawHandleUI(hdc);
  calcNewPointOnResize(pt);
  drawHandleUI(hdc);
}
//-----------------------------------------------------------
void svmObject::calcNewPointOnResize(const POINT& pt)
{
  whichPoint = pt.x < 0 ? Left :
               pt.x > 0 ? Right :
               pt.y < 0 ? Top : Bottom;
  bool fixRatio = needFixedRatio() || !canDistorce();
  if(fixRatio) {
    PRect rect(getRect());
    if(!RectDragging.Width() || !RectDragging.Height())
      RectDragging = getRect();
    else {
      double ratio = rect.Height();
      ratio /= rect.Width();
      switch(whichPoint) {
        case Top:
        case Bottom:
          RectDragging.bottom += pt.y;
          RectDragging.right = (int)(RectDragging.Height() / ratio) + RectDragging.left;
          break;

        case Left:
        case Right:
          RectDragging.right += pt.x;
          RectDragging.bottom = (int)(RectDragging.Width() * ratio) + RectDragging.top;
          break;
        }
      }
    }
  else {
    switch(whichPoint) {
      case Top:
      case Bottom:
        RectDragging.bottom += pt.y;
        break;
      case Left:
      case Right:
        RectDragging.right += pt.x;
        break;
      }
    }
}
//-----------------------------------------------------------
bool svmObject::endDragOnResizeByKey(HDC hdc)
{
  bool modified = false;
  drawHandleUI(hdc);

  if(Prop->Rect != RectDragging) {
    Prop->Rect = RectDragging;
    modified = true;
    }

  Prop->Rect.Normalize();
  Selected = signalAdd;
  return modified;
}
//-----------------------------------------------------------
bool operator !=(const PRect&r1, const PRect& r2)
{
  if(r1.left != r2.left)
    return true;
  if(r1.right != r2.right)
    return true;
  if(r1.top != r2.top)
    return true;
  if(r1.bottom != r2.bottom)
    return true;
  return false;
}
//-----------------------------------------------------------
class D_OffScreen : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    D_OffScreen(PWin* parent, uint id, int& result, svmObject* obj) :
        baseClass(parent, id), Result(result), Obj(obj) {}
    ~D_OffScreen() { destroy(); }

    bool create();
  protected:
    virtual void CmOk();
    svmObject* Obj;
    int& Result;
    static bool lastSkip;
};
//-----------------------------------------------------------
bool D_OffScreen::lastSkip = true;
//-----------------------------------------------------------
bool D_OffScreen::create()
{
  if(!baseClass::create())
    return false;

  PRect rObj = Obj->getRect();
  TCHAR msg[1024];
  wsprintf(msg, _T("L'oggetto %s ha coordinate\r\n[%d, %d, %d, %d], e va fuori schermo\r\n")
                _T("Scegli l'azione da compiere"),
                Obj->getObjName(), rObj.left, rObj.top, rObj.right, rObj.bottom);
  SET_TEXT(IDC_INFO_OBJ, msg);
  if(lastSkip)
    SET_CHECK(IDC_SKIP);
  else
    SET_CHECK(IDC_APPLY);
  return true;
}
//-----------------------------------------------------------
void D_OffScreen::CmOk()
{
  Result = 0;
  lastSkip = false;
  if(IS_CHECKED(IDC_SKIP)) {
    Result = 1;
    lastSkip = true;
    }
  else if(IS_CHECKED(IDC_APPLY))
    Result = 2;

  if(IS_CHECKED(IDC_ALL))
    Result = -Result;
  baseClass::CmOk();
}
//-----------------------------------------------------------
void reInsideRect(PRect& test, const PRect& area)
{
  if(test.left >= area.Width())
    test.MoveTo(area.Width() - test.Width(), test.top);
  if(test.top >= area.Height())
    test.MoveTo(test.left, area.Height() - test.Height());

  if(test.right <= 0)
    test.MoveTo(0, test.top);
  if(test.bottom <= 0)
    test.MoveTo(test.left, 0);
}
//-----------------------------------------------------------
void svmObject::re_InsideRect()
{
  PRect r;
  GetClientRect(*getParent(), r);
  getDataProject().calcFromScreen(r);
  reInsideRect(Prop->Rect, r);
}
//-----------------------------------------------------------
bool svmObject::isOutScreen()
{
  PRect r;
  GetClientRect(*getParent(), r);
  getDataProject().calcFromScreen(r);

  PRect& rTest = Prop->Rect;
  return rTest.right <= 0 || rTest.bottom <= 0 || rTest.left >= r.Width() || rTest.top >= r.Height();
}
//-----------------------------------------------------------
void svmObject::verifyOutScreen()
{
  if(isOutScreen()) {
    int result = 0;
    if(!howVerifyOutScreen) {
      int result = 0;
      if(IDOK == D_OffScreen(getParent(), IDD_OFFSCREEN, result, this).modal()) {
        bool all = false;
        if(result < 0) {
          result = -result;
          all = true;
          }
        if(2 == result) {
          if(all)
            howVerifyOutScreen = 1;
          else
            re_InsideRect();
          }
        else if(all)
          howVerifyOutScreen = -1;
        }
      }
    if(1 == howVerifyOutScreen)
      re_InsideRect();
    }
}
//-----------------------------------------------------------
bool svmObject::endDrag(HDC hdc, const POINT& pt)
{
  bool modified = false;
  drawHandleUI(hdc);
  calcNewPoint(pt);

  switch(whichPoint) {
    case Inside:
    case Extern:
      if(RectDragging.Width() || RectDragging.Height()) {
        Prop->Rect.Offset(RectDragging.left - RectDragging.right, RectDragging.top - RectDragging.bottom);
        verifyOutScreen();
        modified = true;
        }
      break;
    default:
      if(Prop->Rect != RectDragging) {
        Prop->Rect = RectDragging;
        modified = true;
        }
      break;
    }
  Prop->Rect.Normalize();
  Selected = signalAdd;
  return modified;
}
//-----------------------------------------------------------
bool svmObject::isValid4OffsetAddr(uint prph, uint addr, bool noPrph1)
{
  if(PRPH_4_CONST == prph)
    return false;
  if(prph > 1)
    return true;
  if(prph < 1 || noPrph1)
    return false;
  return addr > 1;
}
//-----------------------------------------------------------
bool svmObject::isValidPrph4OffsetAddr(uint prph, const infoOffsetReplace& ior)
{
  if(PRPH_4_CONST == prph)
    return false;
  if(!isSetBitf(eoOnlyOnePrphOnUseAddr, ior.flag))
    return true;
  return prph == ior.prphFilterOnAddr;
}
//-----------------------------------------------------------
void svmObject::invalidate()
{
  svmMainClient* mc = getMainClient(getParent());
  mc->invalidate();
}
//-----------------------------------------------------------
void svmObject::setOffsetAddr(const infoOffsetReplace& ior)
{
  if(isSetBitf(eoUseAddr, ior.flag)) {
    if(isValidPrph4OffsetAddr(Prop->perif, ior))
      if(isValid4OffsetAddr(Prop->perif, Prop->addr, isSetBitf(eoExcludePrph1, ior.flag))) {
        Prop->addr += ior.offs;
        Prop->offset += ior.bitOffs;
        }
    if(isValidPrph4OffsetAddr(Prop->visPerif, ior))
      if(isValid4OffsetAddr(Prop->visPerif, Prop->visAddr, isSetBitf(eoExcludePrph1, ior.flag)) && isSetBitf(eoAlsoVisibility, ior.flag)) {
        Prop->visAddr += ior.offs;
        Prop->visOffset += ior.bitOffs;
        }
    }
  if(isSetBitf(eoUsePrph, ior.flag)) {
    if(ior.prphFrom == Prop->perif)
      Prop->perif = ior.prphTo;
    if(isSetBitf(eoAlsoPrphVisibility, ior.flag) && ior.prphFrom == Prop->visPerif)
      Prop->visPerif = ior.prphTo;
    }

  if(isSetBitf(eoSubstVar, ior.flag)) {
    if(ior.prphFrom2 == Prop->perif && ior.addrFrom == Prop->addr) {
      Prop->perif = ior.prphTo2;
      Prop->addr = ior.addrTo;
      }
    if(ior.prphFrom2 == Prop->visPerif && ior.addrFrom == Prop->visAddr) {
      Prop->visPerif = ior.prphTo2;
      Prop->visAddr = ior.addrTo;
      }
    }
}
//-----------------------------------------------------------
void svmObject::restoreByHistoryObject(const svmObjHistory& history)
{
  if(svmObjHistory::Change == history.getAction()) {
    svmObject* obj = (svmObject*)history.getClone();
    if(obj)
      Clone(*obj);
    }
  else if(svmObjHistory::Group == history.getAction())
    baseClass::restoreByHistoryObject(history);
  else
    Prop->Rect = history.getRect();
}
//-----------------------------------------------------------
svmObjHistory* svmObject::makeHistoryObject(svmObjHistory::typeOfAction act, svmObject* prev, bool useOld)
{
  svmObjHistory* obj = baseClass::makeHistoryObject(act, prev, useOld);
  if(!obj)
    return 0;
  if(svmObjHistory::Change == act) {
    svmObject* clone = makeClone();
    if(useOld && oldProp) {
      delete clone->Prop;
      clone->Prop = oldProp;
      oldProp = 0;
      }
    obj->setClone(clone);
    }
  return obj;
}
//-----------------------------------------------------------
#define BITS_RECT ((1 << CHANGED_X) | (1 << CHANGED_Y) | (1 << CHANGED_W) | (1 << CHANGED_H))
//-----------------------------------------------------------
#define COPY_PROP(a) Prop->a = prop->a
//-----------------------------------------------------------
void svmObject::setCommonProperty(const Property* prop, DWORD bits, LPCTSTR simpleText)
{
    if(!oldProp)
      oldProp = allocProperty();
    oldProp->cloneMinus(*Prop);

  if((1 << CHANGED_FG) & bits)
    COPY_PROP(foreground);

  if((1 << CHANGED_BG) & bits)
    COPY_PROP(background);

  if((1 << CHANGED_BORDER) & bits)
    COPY_PROP(style);

  if((1 << CHANGED_X) & bits)
    Prop->Rect.MoveTo(prop->Rect.left, Prop->Rect.top);

  if((1 << CHANGED_Y) & bits)
    Prop->Rect.MoveTo(Prop->Rect.left, prop->Rect.top);

  if((1 << CHANGED_W) & bits)
    Prop->Rect.right = Prop->Rect.left + prop->Rect.Width();

  if((1 << CHANGED_H) & bits)
    Prop->Rect.bottom = Prop->Rect.top + prop->Rect.Height();

  if((1 << CHANGED_DX) & bits)
    Prop->Rect.Offset(prop->type1, 0);

  if((1 << CHANGED_DY) & bits)
    Prop->Rect.Offset(0, prop->type2);

  if((1 << CHANGED_PRF) & bits)
    COPY_PROP(perif);

  if((1 << CHANGED_ADDR) & bits)
    COPY_PROP(addr);

  if((1 << CHANGED_VALUETYPE) & bits)
    COPY_PROP(typeVal);

  if((1 << CHANGED_NDEC) & bits)
    COPY_PROP(nDec);

  if((1 << CHANGED_NBITS) & bits)
    COPY_PROP(nBits);

  if((1 << CHANGED_OFFSET) & bits)
    COPY_PROP(offset);

  if((1 << CHANGED_NORMALIZ) & bits)
    COPY_PROP(normaliz);

  if((1 << CHANGED_NEGATIVE) & bits)
    COPY_PROP(negative);

  if((1 << CHANGED_FONT) & bits)
    COPY_PROP(idFont);
#if 1
  if((1 << CHANGED_ALIGN) & bits) {
    if((1 << CHANGED_ALIGN2) & bits)
      COPY_PROP(alignText);
    else {
      Prop->alignText &= ~3;
      Prop->alignText |= prop->alignText & 3;
      }
    }
  else if((1 << CHANGED_ALIGN2) & bits) {
    Prop->alignText &= 3;
    Prop->alignText |= prop->alignText & ~3;
    }
#else
  if((1 << CHANGED_ALIGN) & bits)
    COPY_PROP(alignText);
#endif

  if((1 << CHANGED_PSW_LEVEL) & bits)
    COPY_PROP(pswLevel);

  if((1 << CHANGED_PRF_V) & bits)
    COPY_PROP(visPerif);

  if((1 << CHANGED_ADDR_V) & bits)
    COPY_PROP(visAddr);

  if((1 << CHANGED_NBITS_V) & bits)
    COPY_PROP(visnBits);

  if((1 << CHANGED_OFFSET_V) & bits)
    COPY_PROP(visOffset);

  if((1 << CHANGED_NEG_V) & bits)
    COPY_PROP(visNegative);
  if((1 << CHANGED_MOVEMENT) & bits)
    COPY_PROP(MoveInfo);
}
//-----------------------------------------------------------
uint svmObject::getStyle() const
{
  int style;
  if(Prop->style & Property::FILL) {
    if(Prop->style & Property::BORDER)
      style = stylePanel::border_fill;
    else if(Prop->style & Property::UP)
      style = stylePanel::up_fill;
    else if(Prop->style & Property::DN)
      style = stylePanel::dn_fill;
    else
      style = stylePanel::fill;
    }

  else {
    if(Prop->style & Property::BORDER)
      style = stylePanel::border_trasp;
    else if(Prop->style & Property::UP)
      style = stylePanel::up_trasp;
    else if(Prop->style & Property::DN)
      style = stylePanel::dn_trasp;
    else
      style = stylePanel::trasp;
    }
  return style;
}
//-----------------------------------------------------------
bool svmObject::saveVar(P_File& pf, uint baseId, uint addCod)
{
  manageObjId moi(baseId);
  uint addId = moi.getFirstExtendId();
  TCHAR buff[100];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
              addId,
              Prop->perif,
              Prop->addr,
              Prop->typeVal,
              Prop->normaliz,
              Prop->nDec,
              addCod,
              Prop->BaseNum);
  if(writeStringChkUnicode(pf, buff))
    return saveOnlyVisVar(pf, baseId);
  return false;
}
//-----------------------------------------------------------
void svmObject::loadVar(uint id, setOfString& set, uint* addCod)
{
  int perif = 0;
  int addr = 0;
  int typeVal = 0;
  int nDec = 0;
  int normaliz = 0;
  int addC = 0;
  int base = 0;

  manageObjId moi(id);
  uint addId = moi.getFirstExtendId();
  LPCTSTR p = set.getString(addId);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &perif, &addr, &typeVal, &normaliz, &nDec, &addC, &base);

  if(addCod)
    *addCod = addC;
  Prop->perif = perif;
  Prop->addr = addr;
  Prop->typeVal = typeVal;
  Prop->nDec = nDec;
  Prop->normaliz = normaliz;
  Prop->BaseNum = base;

  loadOnlyVisVar(id, set);
}
//-----------------------------------------------------------
bool svmObject::saveOnlyVisVar(P_File& pf, uint baseId, uint addCod)
{
  manageObjId moi(baseId);
  baseId = moi.getSecondExtendId();
  return saveOnlyVisVar(baseId, pf, addCod);
}
//-----------------------------------------------------------
bool svmObject::saveOnlyVisVar(uint trueId, P_File& pf, uint addCod)
{
  TCHAR buff[100];
  int pswLevel = Prop->pswLevel;
  if(pswLevel > 3)
    pswLevel = ONLY_CONFIRM;

  POINT pt = { Prop->Rect.left, Prop->Rect.top };
  int idMove = Prop->MoveInfo.save(pf, pt);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
              trueId,
              addCod ? addCod : Prop->nBits,
              Prop->offset,
              Prop->visPerif,
              Prop->visAddr,
              Prop->visnBits,
              Prop->visOffset,
              Prop->visNegative,
              pswLevel,
              idMove
              );
  return toBool(writeStringChkUnicode(pf, buff));
}
//-----------------------------------------------------------
void svmObject::loadOnlyVisVar(uint id, setOfString& set, uint* addCod)
{

  manageObjId moi(id);
  id = moi.getSecondExtendId();
  loadOnlyVisVar(set, id, addCod);
}
//-----------------------------------------------------------
void svmObject::loadOnlyVisVar(setOfString& set, uint trueId, uint* addCod)
{
  int nBits = 0;
  int offset = 0;

  int vPrf = 0;
  int vAddr = 0;
  int vnBit = 0;
  int vOffs = 0;
  int vNeg = 0;
  int pswLevel = 0;
  int idMove = 0;
  LPCTSTR p = set.getString(trueId);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
          &nBits, &offset,
          &vPrf, &vAddr, &vnBit,
          &vOffs, &vNeg, &pswLevel, &idMove
          );

  if(addCod)
    *addCod = nBits;
  else
    Prop->nBits = nBits;
  Prop->offset = offset;

  Prop->visPerif = vPrf;
  Prop->visAddr = vAddr;
  Prop->visnBits = vnBit;
  Prop->visOffset = vOffs;
  Prop->visNegative = toBool(vNeg);

  if(pswLevel == ONLY_CONFIRM)
    pswLevel = 4;
  Prop->pswLevel = pswLevel;
  Prop->MoveInfo.load(set, idMove);
}
//-----------------------------------------------------------
void svmObject::resolveFont(const typeFont* tf)
{
  if(!useFont() || !tf)
    return;
  svmFont& Fnt = getFontObj();
  Fnt.getId_add(tf, Prop->idFont);
}
//-----------------------------------------------------------
void svmObject::saveInfoFont()
{
  if(!useFont())
    return;
  svmFont& Fnt = getFontObj();
  Prop->forPaste = Fnt.getCopyType(Prop->idFont);
}
//-----------------------------------------------------------
void svmObject::setMirror(uint vert_horz, SIZE sz)
{
  PRect rect = getRect();
  if(emtHorz & vert_horz) {
    int t = rect.right;
    rect.right = sz.cx - rect.left;
    rect.left = sz.cx - t;
    }
  if(emtVert & vert_horz) {
    int t = rect.bottom;
    rect.bottom = sz.cy - rect.top;
    rect.top = sz.cy - t;
    }
  setRect(rect);
}
//-----------------------------------------------------------
static svmObjCount staticObjCount;
//-----------------------------------------------------------
svmObjCount& getObjCount()
{
  return staticObjCount;
}
//-----------------------------------------------------------
svmObjCount::svmObjCount()
{
  reset();
}
//-----------------------------------------------------------
void svmObjCount::reset()
{
  counts[oMAX_OBJ_TYPE] = FIRST_ID_MSG;
  counts[oMAX_OBJ_TYPE + 1] = FIRST_ID_FUNCTION_KEY;
  counts[oMAX_OBJ_TYPE + 2] = FIRST_ID_CHG_PAGE;
  for(int i = 0; i < oMAX_OBJ_TYPE; ++i)
    counts[i] = 0;
}
//-----------------------------------------------------------
DWORD svmObjCount::getChgPageIdCount(DWORD add)
{
  counts[oMAX_OBJ_TYPE + 2] += add;
  return counts[oMAX_OBJ_TYPE + 2] - add;
}
//-----------------------------------------------------------
DWORD svmObjCount::getFunctionKeyIdCount(DWORD add)
{
  counts[oMAX_OBJ_TYPE + 1] += add;
  return counts[oMAX_OBJ_TYPE + 1] - add;
}
//-----------------------------------------------------------
DWORD svmObjCount::getGlobalIdCount(DWORD add)
{
  counts[oMAX_OBJ_TYPE] += add;
  return counts[oMAX_OBJ_TYPE] - add;
}
//-----------------------------------------------------------
DWORD svmObjCount::getIdCount(objType which, DWORD add)
{
  counts[which] += add;
  return counts[which] - add;
}
//-----------------------------------------------------------
class dualBmp4Btn : public dual
{
  private:
    typedef dual baseClass;
  public:
    dualBmp4Btn() : baseClass(ID_INIT_BMP_4_BTN) {}
    virtual ~dualBmp4Btn();

    int addAndGetId(LPCTSTR txt);

    int findId(LPCTSTR txt) const;
    LPCTSTR getTxt(int id) const;
    virtual void reset();
    virtual bool save(P_File& pf);
    virtual bool load(setOfString& set);

  private:
    struct link
    {
      int id;
      LPCTSTR txt;
    };

    PVect<link> Lnk;
};
//-----------------------------------------------------------
static dualBmp4Btn Dual_Bmp;
//-----------------------------------------------------------
dual* getDualBmp4Btn()
{
  return &Dual_Bmp;
}
//-----------------------------------------------------------
static dualBmp4Btn Dual_BmpLinked;
//-----------------------------------------------------------
dual* getDualBmp4BtnLinked()
{
  return &Dual_BmpLinked;
}
//-----------------------------------------------------------
dualBmp4Btn::~dualBmp4Btn()
{
  reset();
}
//-----------------------------------------------------------
int dualBmp4Btn::addAndGetId(LPCTSTR txt)
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(!_tcsicmp(txt, Lnk[i].txt))
      return Lnk[i].id;
  Lnk[nElem].id = currCount;
  Lnk[nElem].txt = str_newdup(txt);
  return currCount++;
}
//-----------------------------------------------------------
int dualBmp4Btn::findId(LPCTSTR txt) const
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(!_tcsicmp(txt, Lnk[i].txt))
      return Lnk[i].id;
  return 0;
}
//-----------------------------------------------------------
LPCTSTR dualBmp4Btn::getTxt(int id) const
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(Lnk[i].id == id)
      return Lnk[i].txt;
  return 0;
}
//-----------------------------------------------------------
void dualBmp4Btn::reset()
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    delete []Lnk[i].txt;
  Lnk.reset();
  currCount = ID_INIT_BMP_4_BTN;
}
//-----------------------------------------------------------
bool dualBmp4Btn::save(P_File& pf)
{
  int nElem = Lnk.getElem();
  TCHAR buff[500];
  TCHAR name[500];
  for(int i = 0; i < nElem; ++i) {
    makeRelNameImage(name, Lnk[i].txt, true);
    wsprintf(buff, _T("%d,%s\r\n"), Lnk[i].id, name);
    writeStringChkUnicode(pf, buff);
    }
  return true;
}
//-----------------------------------------------------------
bool dualBmp4Btn::load(setOfString& set)
{
  int ix = 0;
  for(;;) {
    LPCTSTR p = set.getString(currCount);
    if(!p)
      break;
    Lnk[ix].id = currCount++;
    Lnk[ix].txt = str_newdup(p);
    ++ix;
    }
  return true;
}
//-----------------------------------------------------------
bool isMetafile(LPCTSTR filename)
{
  int len = _tcslen(filename);
  return !_tcsicmp(filename + len - 2, _T("mf"));
}
//-----------------------------------------------------------
LPCTSTR get_objName(int obj)
{
  static LPCTSTR names[] =
    {
        _T("Pulsante"),
        _T("Edit"),
        _T("Pannello"),
        _T("Text"),
        _T("BarGraph"),
        _T("Led"),
        _T("Listbox"),
        _T("Diametro"),
        _T("Scelta Lista"),
        _T("Testo Semplice"),
        _T("Bitmap variabile"),
        _T("Curve"),
        _T("X Meter"),
        _T("Cam"),
        _T("Oscilloscopio"),
        _T("Listbox Allarmi"),
        _T("Trend"),
        _T("Spin"),
        _T("Tabella Info"),
        _T("Plot XY"),
        _T("Slider"),
        _T("Bitmap di sfondo"),
        _T("Oggetto sconosciuto")
     };
  if((uint)obj >= SIZE_A(names))
    obj = SIZE_A(names) - 1;
  return names[obj];
}
