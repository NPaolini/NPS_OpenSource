//-------------------- svmObject.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "svmObject.h"
#include "svmHistory.h"
#include "p_File.h"
#include "svmMainClient.h"
#include "pTextPanel.h"
#include "macro_utils.h"
//-----------------------------------------------------------
const int minBox = 5;
//-----------------------------------------------------------
int svmObject::howVerifyOutScreen = 0;
//-----------------------------------------------------------
#define RND rand()
#define MAKE_RND_RGB RGB(RND, RND, RND)
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, int id, int idType, int x1, int y1, int x2, int y2) :
    Rect(DATA_NOT_USED == x1 ? 100 : x1, DATA_NOT_USED == y1 ? -100 : y1,
         DATA_NOT_USED == x2 ? x1 + 500 : x2, DATA_NOT_USED == y2 ? y1 - 300 : y2),
    RectDragging(Rect), Id(id),
    Selected(svmObject::notSel), whichPoint(svmObject::Extern), Par(par),
    Prop(0), oldProp(0), IdType(idType), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)

{ }
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, int id, int idType, const PRect& r) :
    Rect(r), RectDragging(r),
    Id(id), Selected(svmObject::notSel),
    whichPoint(svmObject::Extern), Par(par),
    Prop(0), oldProp(0), IdType(idType), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)
{ }
//-----------------------------------------------------------
svmObject::svmObject(PWin* par, setOfString& sos, int id, int idType, int idCode) :
      Id(id), Selected(svmObject::notSel),
      whichPoint(svmObject::Extern), Par(par),
      Prop(0), oldProp(0), IdType(idType), locked(false), Linked(false)
    ,Visible(true), XoredColor(MAKE_RND_RGB)
{ }
//-----------------------------------------------------------
void svmObject::standBySel(bool add)
{
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
void svmObject::Select(HDC hdc)
{
  if(svmObject::notSel == Selected || svmObject::signalAdd == Selected) {
    Selected = yesSel;
    uint old = whichPoint;
    whichPoint = Top;
    RectDragging = Prop->Rect;
    drawHandleUI(hdc);
    whichPoint = (svmObject::typeOfAnchor)old;
    }
}
//-----------------------------------------------------------
void svmObject::Unselect(HDC hdc)
{
  if(svmObject::yesSel == Selected || svmObject::signalRemove == Selected) {
    Selected = notSel;
    Select(hdc);
    }
  Selected = notSel;
}
//-----------------------------------------------------------
void svmObject::fillTips(LPTSTR tips, int size)
{
  wsprintf(tips, _T("Obj -> %s\r\nid=%d"),
          getObjName(), getId());
}
//-----------------------------------------------------------
PRect svmObject::getMinBox() const
{
  PRect r(0, 0, minBox, minBox);
  HDC hdc = getDC();
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  releaseDC(hdc);
  return r;
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObject::pointIn(const POINT& pt_) const
{
  // per semplificare e mantenere la vecchia impostazione, rimettiamo tutto in positivo
  POINT pt = { pt_.x, -pt_.y };
  PRect rect(Prop->Rect.left, -Prop->Rect.top, Prop->Rect.right, -Prop->Rect.bottom);

  PRect rt = getMinBox();
  // idem come sopra
  rt.bottom = -rt.bottom;

  int x = pt.x;
  int y = pt.y;
  if(x < (rect.left - rt.Width()) || y < (rect.top - rt.Height()))
    return Extern;
  if(x > (rect.right + rt.Width()) || y > (rect.bottom + rt.Height()))
    return Extern;

  int w = rect.Width();
  int h = rect.Height();

  int x1 = rect.left + 1;
  int x2 = x1 + (w - rt.Width()) / 2;
  int x3 = rect.left + w - rt.Width() + 1;

  int y1 = rect.top + 1;
  int y2 = y1 + (h - rt.Height()) / 2;
  int y3 = rect.top + h - rt.Height() + 1;

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
}
//-----------------------------------------------------------
void svmObject::DrawTitle(HDC hdc)
{
}
//-----------------------------------------------------------
#define LOCKED_DIST 6
//-----------------------------------------------------------
void svmObject::DrawObjectLocked(HDC hdc, const PRect& rect)
{
  // disegnare una differenziazione
  PRect r(getLinkedRect());
  r.Inflate(-LOCKED_DIST, LOCKED_DIST);
  HPEN pen = CreatePen(PS_DOT, 1, RGB(0xff, 0xff, 0xff));

  HGDIOBJ oldPen = SelectObject(hdc, pen);
  for(int i = r.top; i > r.bottom; i -= LOCKED_DIST * 2) {
    MoveToEx(hdc, r.left, i, 0);
    LineTo(hdc, r.right, i);
    }

  HPEN pen2 = CreatePen(PS_DOT, 1, RGB(0x7f, 0x7f, 0x7f));
  SelectObject(hdc, pen2);
  for(int i = r.top - LOCKED_DIST; i > r.bottom; i -= LOCKED_DIST * 2) {
    MoveToEx(hdc, r.left, i, 0);
    LineTo(hdc, r.right, i);
    }
  DeleteObject(SelectObject(hdc, oldPen));
  DeleteObject(pen);
}
//-----------------------------------------------------------
void svmObject::DrawOnlyObject(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(rect & getRect()) {
    DrawObject(hdc);
    DrawTitle(hdc);
    if(isLocked()) {
      DrawObjectLocked(hdc, rect);
      drawRectUI(hdc, Prop->Rect);
      }
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
void svmObject::DrawObjectLinked(HDC hdc, const PRect& rect)
{
  bool old = isLocked();
  setLock(true);
  DrawOnlyObject(hdc, rect);
  setLock(old);
}
//-----------------------------------------------------------
//#define FRECT(hdc, r) Ellipse(hdc, r.left, r.top, r.right, r.bottom)
#define FRECT(hdc, r) Rectangle(hdc, r.left, r.top, r.right, r.bottom)
//-----------------------------------------------------------
void svmObject::drawBoxes(HDC hdc, const PRect& all)
{
  int w = all.Width();
  int h = all.Height();

  PRect rt = getMinBox();
  int x1 = all.left + 1;
  int x2 = x1 + (w - rt.Width()) / 2;
  int x3 = all.left + w - rt.Width() + 1;

  int y1 = all.top - 1;
  int y2 = y1 + (h - rt.Height()) / 2;
  int y3 = all.top + h - rt.Height() - 1;

  HBRUSH brush = CreateSolidBrush(getXoredColor());

  HGDIOBJ oldBrush = SelectObject(hdc, brush);

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
void svmObject::drawRectUI(HDC hdc, const PRect& rtAll)
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
//-----------------------------------------------------------
void svmObject::drawHandleUI(HDC hdc)
{
  PRect rtAll(RectDragging);
  if(Inside == whichPoint || Extern == whichPoint) {
    rtAll = Prop->Rect;
    rtAll.Offset(RectDragging.left - RectDragging.right,
                 RectDragging.top - RectDragging.bottom);
    }

  int oldRop = SetROP2(hdc, R2_XORPEN);
  drawRectUI(hdc, rtAll);

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
        double ratio = -rect.Height();
        ratio /= rect.Width();
        double ratioTmp = -dy;
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
              RectDragging.top = (int)(rect.bottom + (double)dx * ratio);
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
              RectDragging.top = (int)(rect.bottom + (double)dx * ratio);
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
              RectDragging.bottom = (int)(rect.top - (double)dx * ratio);
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
              RectDragging.bottom = (int)(rect.top - (double)dx * ratio);
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
               pt.y > 0 ? Top : Bottom;
  bool fixRatio = needFixedRatio() || !canDistorce();
  if(fixRatio) {
    PRect rect(getRect());
    if(!RectDragging.Width() || !RectDragging.Height())
      RectDragging = getRect();
    else {
      double ratio = -rect.Height();
      ratio /= rect.Width();
      switch(whichPoint) {
        case Top:
        case Bottom:
          RectDragging.bottom -= pt.y;
          RectDragging.right = (int)(-RectDragging.Height() / ratio) + RectDragging.left;
          break;

        case Left:
        case Right:
          RectDragging.right += pt.x;
          RectDragging.bottom = (int)(-RectDragging.Width() * ratio) - RectDragging.top;
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
void normalizeYNeg(PRect& r)
{
  if(r.right < r.left) {
    int t = r.right;
    r.right = r.left;
    r.left = t;
    }
  if(r.bottom > r.top) {
    int t = r.bottom;
    r.bottom = r.top;
    r.top = t;
    }
}
//-----------------------------------------------------------
bool intersecYNeg(const PRect& r1, const PRect& r2)
{
  if(r1.left > r2.right || r1.top < r2.bottom || r1.right < r2.left || r1.bottom > r2.top)
    return false;
  return true;
}
//-----------------------------------------------------------
bool isInYNeg(const PRect& r, const POINT& pt)
{
  if(pt.x > r.right || pt.y < r.bottom || pt.x < r.left || pt.y > r.top)
    return false;
  return true;
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

  normalizeYNeg(Prop->Rect);
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
                Obj->getObjName(), rObj.left, -rObj.top, rObj.right, -rObj.bottom);
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
void svmObject::reInsideRect(PRect& test, const PRect& area)
{
  if(test.left >= area.Width())
    test.MoveTo(area.Width() - test.Width(), test.top);
  if(-test.top >= -area.Height())
    test.MoveTo(test.left, area.Height() + test.Height());

  if(test.right <= 0)
    test.MoveTo(0, test.top);
  if(-test.bottom <= 0)
    test.MoveTo(test.left, 0);
}
//-----------------------------------------------------------
void svmObject::re_InsideRect()
{
  PRect r;
  GetClientRect(*getParent(), r);
  HDC hdc = getDC();
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  releaseDC(hdc);
  getDataProject().calcFromScreen(r);
  reInsideRect(Prop->Rect, r);
}
//-----------------------------------------------------------
HDC svmObject::getDC() const
{
  HDC hdc = GetDC(*getParent());
  SetMapMode(hdc, MM_LOMETRIC);
  return hdc;
}
//-----------------------------------------------------------
void svmObject::releaseDC(HDC hdc) const
{
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(*getParent(), hdc);
}
//-----------------------------------------------------------
bool svmObject::isOutScreen()
{
  PRect r;
  GetClientRect(*getParent(), r);
  HDC hdc = getDC();
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  releaseDC(hdc);

  getDataProject().calcFromScreen(r);

  PRect& rTest = Prop->Rect;
  return rTest.right <= 0 || -rTest.bottom <= 0 || rTest.left >= r.Width() || -rTest.top >= -r.Height();
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
        replaceLast(this);
        modified = true;
        }
      break;
    }
  normalizeYNeg(Prop->Rect);
  Selected = signalAdd;
  return modified;
}
//-----------------------------------------------------------
void svmObject::restoreByHistoryObject(const svmObjHistory& history)
{
  if(svmObjHistory::Change == history.getAction())
    Clone(*history.getClone());
  else
    Prop->Rect = history.getRect();
}
//-----------------------------------------------------------
svmObjHistory* svmObject::makeHistoryObject(svmObjHistory::typeOfAction act, svmObject* prev, bool useOld)
{
  svmObjHistory* obj = new svmObjHistory(this, act, prev);
  if(obj && svmObjHistory::Change == act) {
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
Property* svmObject::getLast()
{
  lastPropertyInUse* manObj = getLastPropertyInUse();
  return manObj->getLast(getIdType());
}
//-----------------------------------------------------------
void svmObject::replaceLast(svmObject* obj)
{
  lastPropertyInUse* manObj = getLastPropertyInUse();
  manObj->replaceLast(obj);
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObject::dialogProperty(bool onCreate)
{
  if(onCreate) {
    Property* p = getLast();
    if(p && (p->Rect.Width() ||  p->Rect.Height())) {
      p->Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      copyPropFrom(p);
      }
    else
      replaceLast(this);
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(performDialogProperty()) {
      replaceLast(this);
      return svmObjHistory::Change;
      }
    }
  return svmObjHistory::None;;
}
//-----------------------------------------------------------
#define BITS_RECT ((1 << CHANGED_X) | (1 << CHANGED_Y) | (1 << CHANGED_W) | (1 << CHANGED_H))
//-----------------------------------------------------------
#define COPY_PROP(a) Prop->a = prop->a
//-----------------------------------------------------------
void svmObject::setCommonProperty(const unionProperty& prop)
{
  if(!oldProp)
    oldProp = allocProperty();
  oldProp->cloneMinus(*Prop);

  if(PROP_BIT_POS(eupX) & prop.flagBits)
    Prop->Rect.MoveTo(prop.Rect.left, Prop->Rect.top);
  if(PROP_BIT_POS(eupY) & prop.flagBits)
    Prop->Rect.MoveTo(Prop->Rect.left, prop.Rect.top);

  if(PROP_BIT_POS(eupW) & prop.flagBits)
    Prop->Rect.right = Prop->Rect.left + prop.Rect.right;
  if(PROP_BIT_POS(eupH) & prop.flagBits)
    Prop->Rect.bottom = Prop->Rect.top + prop.Rect.bottom;

  if(PROP_BIT_POS(eupDX) & prop.flagBits)
    Prop->Rect.Offset(prop.dX, 0);
  if(PROP_BIT_POS(eupDY) & prop.flagBits)
    Prop->Rect.Offset(0, prop.dY);
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
//-----------------------------------------------------------
template <typename T>
class svmFactoryObj_Page
{
  public:
    svmFactoryObj_Page() {}
    ~svmFactoryObj_Page() {}
    T& getObjPage()
    {
      const dataProject& dp = getDataProject();
      return ObjPage[dp.currPage];
    }

  private:
    T ObjPage[MAX_PAGES];
};
//-----------------------------------------------------------
static svmFactoryObj_Page<svmObjCount> staticObjCount;
//-----------------------------------------------------------
svmObjCount& getObjCount()
{
  return staticObjCount.getObjPage();
}
//-----------------------------------------------------------
svmObjCount::svmObjCount()
{
  reset();
}
//-----------------------------------------------------------
void svmObjCount::reset()
{
  for(int i = 0; i < SIZE_A(counts); ++i)
    counts[i] = 0;
}
//-----------------------------------------------------------
DWORD svmObjCount::getGlobalIdCount(DWORD add)
{
  counts[0] += add;
  return counts[0] - add;
}
//-----------------------------------------------------------
/*
DWORD svmObjCount::getIdCount(uint which, DWORD add)
{
  counts[which] += add;
  return counts[which] - add;
}
*/
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
        _T("Text"),
        _T("Line"),
        _T("Rect"),
        _T("Ellipse"),
        _T("Image"),
        _T("Oggetto sconosciuto")
     };
  if(obj >= SIZE_A(names))
    obj = SIZE_A(names) - 1;
  return names[obj];
}
