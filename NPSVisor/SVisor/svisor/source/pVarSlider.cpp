//------ PVarSlider.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windowsx.h>
//----------------------------------------------------------------------------
#include "PVarSlider.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "p_util.h"
#include "newnormal.h"
//----------------------------------------------------------------------------
class objSliderBase : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    objSliderBase(P_BaseBody* par, uint id, const PRect& r, const infoSlider& info);

    virtual bool create() { if(!baseClass::create()) return false; enableCapture(true); return true; }
    virtual ~objSliderBase() { destroy(); }
    PBitmap* makeBmp(PBitmap* bmp);
    POINT getCurrPoint();
    const infoSlider& getInfo() { return I_Slider; }
    void setVisible(bool set) { Visible = set; EnableWindow(*this, Visible); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evMouse(uint message, uint key, POINT pt);

    void EvMouseLUp(uint modKeys, POINT point);
    void EvMouseLDn(uint modKeys, POINT point);
    void EvMouseMove (uint modKeys, POINT point);

    virtual bool inside(const POINT& pt, uint dim, double valule);
    virtual PRect getCurrRect(uint dim, double value) = 0;
    bool check_point(const POINT& point);
    virtual void calcDist(const POINT& pt, const POINT& old, uint dim) = 0;
    void check_and_send(POINT pt, bool send);

    void setValue(double v);
    PBitmap* resizeBmp(PBitmap* bmp, const PRect& r);
    infoSlider I_Slider;
    bool pressed;
    bool Visible;
    POINT oldPt;
    PRect Rect;
    double getPerc(double add);
    double getCurrPerc();

    void killTm() { KillTimer(*this, 567); I_Slider.Owner->canUpdate = true; }
    void setTm() {  SetTimer(*this, 567, 500, 0); I_Slider.Owner->canUpdate = false;  }

};
//----------------------------------------------------------------------------
class objSliderVert : public objSliderBase
{
  private:
    typedef objSliderBase baseClass;
  public:
    objSliderVert(P_BaseBody* par, uint id, const PRect& r, const infoSlider& info) : baseClass(par, id, r, info) {}
    virtual ~objSliderVert() {}
  protected:
    virtual void calcDist(const POINT& pt, const POINT& old, uint dim);
    virtual PRect getCurrRect(uint dim, double value);
};
//----------------------------------------------------------------------------
class objSliderHorz : public objSliderBase
{
  private:
    typedef objSliderBase baseClass;
  public:
    objSliderHorz(P_BaseBody* par, uint id, const PRect& r, const infoSlider& info) : baseClass(par, id, r, info) {}
    virtual ~objSliderHorz() {}
  protected:
    virtual void calcDist(const POINT& pt, const POINT& old, uint dim);
    virtual PRect getCurrRect(uint dim, double value);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PVarSlider::PVarSlider(P_BaseBody* owner, uint id) :baseVar(owner, id), hrgn(0),
    valueMin(0), valueMax(1), valueCurr(0), Slider(0), dimImage(0), canUpdate(true)
{
}
//----------------------------------------------------------------------------
PVarSlider::~PVarSlider()
{
  if(hrgn)
    DeleteObject(hrgn);
}
//----------------------------------------------------------------------------
bool PVarSlider::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;

  int idMax = 0;
//  int idVal = 0;
  int idMin = 0;

  int idBmp = 0;
  int h_w = 0;

  int fullImage = 0;
  int vert = 0;
  int mirror = 0;
  int reverse = 0;

  int readOnly = 0;
  int updateAlways = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idMax, &idMin, &idBmp, &h_w,
                    &fullImage, &vert, &mirror, &reverse,
                    &readOnly, &updateAlways
                    );
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  bool horz = true;
  if(vert) {
    // se verticale scambia le dimensioni
    int t = w;
    w = h;
    h = t;
    }


  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  makeOtherVar(vMax, idMax);
  makeOtherVar(vMin, idMin);
  makeStdVars();
//  makeOtherVar(BaseVar, idVal);

  LPCTSTR p1 = getOwner()->getPageString(getSecondExtendId());
  allocVisibility(p1);


  PBitmap* bmp = getOwner()->getBmp4Btn(idBmp - ID_INIT_BMP_4_BTN);
  infoSlider info(this, h_w, fullImage, vert, mirror, reverse, readOnly, updateAlways);
  static int idWin = 2355;
  if(vert)
    Slider = new objSliderVert(getOwner(), idWin++, rect, info);
  else
    Slider = new objSliderHorz(getOwner(), idWin++, rect, info);
  Slider->create();
  Image = Slider->makeBmp(new PBitmap(*bmp));
  SIZE sz = Image->getSize();
  if(info.fullImage) {
    if(!h_w)
      h_w = 4;
    sz.cx = sz.cy = h_w;
    }
  if(vert)
    dimImage = sz.cy;
  else
    dimImage = sz.cx;
  return true;
}
//----------------------------------------------------------------------------
void PVarSlider::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vMax);
  addReqVar2(allBits, vMin);
}
//----------------------------------------------------------------------------
#define IS_DIFF(v1, v2) (fabs((v1) - (v2)) > dPRECISION)
//-----------------------------------------------------------
bool PVarSlider::update(bool force)
{
  bool upd = baseVar::update(force);
  bool visible = isVisible();
  Slider->setVisible(visible);
  if(!visible)
    return upd;
  if(!canUpdate)
    return upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }
  valueCurr = BaseVar.getNormalizedResult(data);
  if(prfData::notModified != result)
    force = true;

  prfData dataMax;
  result = vMax.getData(getOwner(), dataMax, 0);

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vMax.getPrph(), dataMax, result);
    return upd;
    }
  valueMax = vMax.getNormalizedResult(dataMax);
  if(prfData::notModified != result)
    force = true;

  if(vMin.getPrph()) {
    prfData dataMin;
    result = vMin.getData(getOwner(), dataMin, 0);

    if(prfData::failed >= result) {
      getOwner()->ShowErrorData(vMin.getPrph(), dataMin, result);
      return upd;
      }
    if(prfData::notModified != result)
      force = true;
    valueMin = vMin.getNormalizedResult(dataMin);
    }
  force |= upd;

  bool reload = false;
  if (valueMin > valueCurr) {
    valueCurr = valueMin;
    reload = true;
    }
  if (valueMax < valueCurr) {
    valueCurr = valueMax;
    reload = true;
    }
  if(reload)
    sendCurrData();
  if(!force)
    return upd;

  invalidate();
  return true;
}
//----------------------------------------------------------------------------
void PVarSlider::sendCurrData()
{
  if(Slider->getInfo().readOnly || !BaseVar.getPrph())
    return;
  mainClient* mc = getMain();
  genericPerif* prph = mc->getGenPerif(BaseVar.getPrph());
  prfData data;
  data.lAddr = BaseVar.getAddr();
  data.typeVar = BaseVar.getType();

  getDenormalizedData(data, valueCurr, getNorm(BaseVar.getNorm()));
  prph->set(data, false);
}
//-----------------------------------------------------------------
void PVarSlider::performDraw(HDC hdc)
{
  if(!hdc || !Image || !isVisible())
     return;

  PRect r = get_Rect();
  POINT pt = Slider->getCurrPoint();
  SIZE sz = Image->getSize();
  pt.y += r.top;

  if(Slider->getInfo().fullImage) {
    switch(Slider->getInfo().vert) {
      case false:
        pt.x += r.left - r.Width() + getdimImg();
        break;
      case true:
        pt.x += r.left;
        break;
      }
    }

  else
  pt.x += r.left;
  if(!hrgn)
    hrgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
  SelectClipRgn(hdc, hrgn);
  r = PRect(0, 0, sz.cx, sz.cy);
  r.MoveTo(pt.x, pt.y);
  Image->drawTransp(hdc, r);
  SelectClipRgn(hdc, 0);
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
objSliderBase::objSliderBase(P_BaseBody* par, uint id, const PRect& r, const infoSlider& info) :
        baseClass(par, id, r, 0), pressed(false), Rect(r),  I_Slider(info), Visible(false)
{
  Rect.MoveTo(0, 0);
}
//----------------------------------------------------------------------------
bool objSliderBase::check_point(const POINT& point)
{
  PVarSlider* ps = I_Slider.Owner;
  uint dim = ps->getdimImg();
  double range = ps->valueMax - ps->valueMin;
  if (!range) {
    range = ps->valueCurr;
    if (!range)
      range = 1;
  }
  double value = (ps->valueCurr - ps->valueMin) / range;
  return inside(point, dim, value);
}
//----------------------------------------------------------------------------
POINT objSliderBase::getCurrPoint()
{
  PVarSlider* ps = I_Slider.Owner;
  uint dim = ps->getdimImg();
  double range = ps->valueMax - ps->valueMin;
  double value = getCurrPerc();
  PRect r = getCurrRect(dim, value);
  POINT pt = { r.left, r.top };
  return pt;
}
//----------------------------------------------------------------------------
double objSliderBase::getCurrPerc()
{
  PVarSlider* ps = I_Slider.Owner;
  double range = ps->valueMax - ps->valueMin;
  if(!range) {
    range = ps->valueCurr;
    if(!range)
      range = 1;
    }
  double value = (ps->valueCurr - ps->valueMin) / range;
  return value;
}
//----------------------------------------------------------------------------
double objSliderBase::getPerc(double add)
{
  double value = getCurrPerc();
  value += add;
  if(value > 1)
    value = 1;
  else if(value < 0)
    value = 0;
  return value;
}
//----------------------------------------------------------------------------
bool objSliderBase::inside(const POINT& pt, uint dim, double value)
{
  PRect r = getCurrRect(dim, value);
  return r.IsIn(pt);
}
//----------------------------------------------------------------------------
LRESULT objSliderBase::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      do {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        evMouse(message, wParam, pt);
        } while(false);
      break;

    case WM_ERASEBKGND:
      return 1;
    case WM_TIMER:
      killTm();
      break;

    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
#if 0
        HGDIOBJ old = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, Rect.left, Rect.top, Rect.right, Rect.bottom);
        SelectObject(hdc, old);
#endif
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void objSliderBase::evMouse(uint message, uint key, POINT pt)
{
  if(!Visible)
    return;
  switch(message) {
    case WM_LBUTTONUP:
      EvMouseLUp(key, pt);
      break;
    case WM_LBUTTONDOWN:
      EvMouseLDn(key, pt);
      break;
    case WM_MOUSEMOVE:
      EvMouseMove(key, pt);
    }
}
//----------------------------------------------------------------------------
void objSliderBase::EvMouseMove (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(pressed) {
    check_and_send(point, I_Slider.updateAlways);
    oldPt = point;
    curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
    setTm();
    }
  else if(check_point(point))
    curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
  else
    curs = LoadCursor(0, IDC_ARROW);
  SetCursor(curs);
}
//----------------------------------------------------------------------------
void objSliderBase::EvMouseLDn (uint modKeys, POINT point)
{
  if(check_point(point)) {
    oldPt = point;
    setTm();
    HCURSOR curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
    SetCursor(curs);
    pressed = true;
    }
}
//----------------------------------------------------------------------------
void objSliderBase::EvMouseLUp (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(check_point(point))
    curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
  else
    curs = LoadCursor(0, IDC_ARROW);
  SetCursor(curs);
  if(pressed) {
    if(!I_Slider.updateAlways)
      I_Slider.Owner->sendCurrData();
    I_Slider.Owner->invalidate();
    }
  pressed = false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IS_DIFF(v1, v2) (fabs((v1) - (v2)) > dPRECISION)
#define DIFF_POINT(p1, p2) (p1.x != p2.x || p1.y != p2.y)
//----------------------------------------------------------------------------
void objSliderBase::check_and_send(POINT pt, bool send)
{
  if(DIFF_POINT(pt, oldPt)) {
    double old = I_Slider.Owner->valueCurr;
    uint dim = I_Slider.Owner->getdimImg();
    calcDist(pt, oldPt, dim);
    if(IS_DIFF(old, I_Slider.Owner->valueCurr)) {
      if(send)
        I_Slider.Owner->sendCurrData();
      I_Slider.Owner->invalidate();
      }
    }
}
//----------------------------------------------------------------------------
void objSliderBase::setValue(double v)
{
  PVarSlider* ps = I_Slider.Owner;
  ps->valueCurr = v * (ps->valueMax - ps->valueMin) + ps->valueMin;
}
//-----------------------------------------------------------
PBitmap* objSliderBase::resizeBmp(PBitmap* bmp, const PRect& r)
{
  SIZE sz = bmp->getSize();
  double perc = 0;
  if(I_Slider.vert) {
    if(sz.cx == r.Width() || !sz.cx)
      return 0;
    perc = (double)r.Width() / sz.cx;
    }
  else {
    if(sz.cy == r.Height() || !sz.cy)
       return 0;
    perc = (double)r.Height() / sz.cy;
    }
  return PBitmap::resizeBmp(bmp, perc);
}
//-----------------------------------------------------------
PBitmap* objSliderBase::makeBmp(PBitmap* bmp)
{
  if(!bmp)
    return 0;
  bool old = bmp->setQualityOnStretch(true);
  if(I_Slider.vert)
    PBitmap::rotateInside(bmp);
  PBitmap* bmp2 = resizeBmp(bmp, Rect);
  if(bmp2) {
    bmp->setQualityOnStretch(old);
    delete bmp;
    bmp = bmp2;
    }
  return bmp;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRect objSliderVert::getCurrRect(uint dim, double value)
{
  PRect r(Rect);
  r.bottom -= dim;
  double t = I_Slider.reverse ? value : 1 - value;
  r.top = ROUND_REAL(r.Height() * t);
  r.bottom = r.top + dim;
  return r;
}
//----------------------------------------------------------------------------
void objSliderVert::calcDist(const POINT& pt, const POINT& old, uint dim)
{
  double v = old.y - pt.y;
  if(I_Slider.reverse)
    v = -v;
  PRect r(Rect);
  r.bottom -= dim;
  v /= r.Height();

  double value = getPerc(v);

  setValue(value);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRect objSliderHorz::getCurrRect(uint dim, double value)
{
  PRect r(Rect);
  r.right -= dim;
  double t = I_Slider.reverse ? 1 - value : value;
  r.left = ROUND_REAL(r.Width() * t);
  r.right = r.left + dim;
  return r;
}
//----------------------------------------------------------------------------
void objSliderHorz::calcDist(const POINT& pt, const POINT& old, uint dim)
{
  double v = pt.x - old.x;
  if(I_Slider.reverse)
    v = -v;
  PRect r(Rect);
  r.right -= dim;
  v /= r.Width();

  double value = getPerc(v);

  setValue(value);
}
