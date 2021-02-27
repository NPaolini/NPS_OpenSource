//-------- PVarXMeter.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Windowsx.h>
#include "p_basebody.h"
#include "mainclient.h"
#include "PVarXMeter.h"
#include "newnormal.h"
#include "p_util.h"
#include "p_file.h"
#include "pTraspBitmap.h"
//----------------------------------------------------------------------------
class calc_dist
{
  public:
    calc_dist(PWinActive* owner) : Owner(owner) {}
    virtual ~calc_dist() {}
    virtual bool isDiffPoint(const POINT& pt, const POINT& old) = 0;
    virtual double getdist(const POINT& pt, const POINT& old) = 0;
  protected:
    PWinActive* Owner;
};
//----------------------------------------------------------------------------
class calc_dist_horz : public calc_dist
{
private:
  typedef calc_dist baseClass;
public:
  calc_dist_horz(PWinActive* owner) : baseClass(owner) {}
  virtual bool isDiffPoint(const POINT& pt, const POINT& old) { return pt.x != old.x; }
  virtual double getdist(const POINT& pt, const POINT& old);
};
//----------------------------------------------------------------------------
class calc_dist_vert : public calc_dist
{
private:
  typedef calc_dist baseClass;
public:
  calc_dist_vert(PWinActive* owner) : baseClass(owner) {}
  virtual bool isDiffPoint(const POINT& pt, const POINT& old) { return pt.y != old.y; }
  virtual double getdist(const POINT& pt, const POINT& old);
};
//----------------------------------------------------------------------------
class PWinActive : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PWinActive(P_BaseBody* par, uint id, const PRect& r, PVarXMeter* owner, bool updateAlways, int mul, bool vert) :
        baseClass(par, id, r, 0), Owner(owner), updateAlways(updateAlways), pressed(false), Mul(mul), Rect(r)
      {
        Rect.MoveTo(0, 0);
        if(vert)
          CalcDist = new calc_dist_vert(this);
        else
          CalcDist = new calc_dist_horz(this);
      }
    virtual bool create() { if(!baseClass::create()) return false; enableCapture(true); return true; }
    virtual ~PWinActive() { delete CalcDist; }
    void setVisibility(bool set) { EnableWindow(*this, set); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evMouse(uint message, uint key, POINT pt);

    void EvMouseLUp(uint modKeys, POINT point);
    void EvMouseLDn(uint modKeys, POINT point);
    void EvMouseMove (uint modKeys, POINT point);

    bool check_point(const POINT& point);
    void calcDist(const POINT& pt, const POINT& old);
    void check_and_send(POINT pt, bool send);
    bool updateAlways;
    bool pressed;
    POINT oldPt;
    PVarXMeter* Owner;
    PRect Rect;
    int Mul;
    calc_dist* CalcDist;

    void killTm() { KillTimer(*this, 567); Owner->canUpdate = true; }
    void setTm() {  SetTimer(*this, 567, 500, 0); Owner->canUpdate = false;  }
    friend class calc_dist_vert;
    friend class calc_dist_horz;
};
//----------------------------------------------------------------------------
double calc_dist_horz::getdist(const POINT& pt, const POINT& old)
{
  double v = pt.x - old.x;
  v /= Owner->Rect.Width() * Owner->Mul / 100.0;
  return v;
}
//----------------------------------------------------------------------------
double calc_dist_vert::getdist(const POINT& pt, const POINT& old)
{
  double v = old.y - pt.y;
  v /= Owner->Rect.Height() * Owner->Mul / 100.0;
  return v;
}
//----------------------------------------------------------------------------
bool PWinActive::check_point(const POINT& point)
{
  return Rect.IsIn(point);
}
//----------------------------------------------------------------------------
LRESULT PWinActive::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        Rectangle(hdc, Rect.left, Rect.top, Rect.right, Rect.bottom);
#endif
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PWinActive::evMouse(uint message, uint key, POINT pt)
{
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
void PWinActive::EvMouseMove (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(pressed) {
    check_and_send(point, updateAlways);
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
void PWinActive::EvMouseLDn (uint modKeys, POINT point)
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
void PWinActive::EvMouseLUp (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(check_point(point))
    curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
  else
    curs = LoadCursor(0, IDC_ARROW);
  SetCursor(curs);
  if(pressed) {
    if(!updateAlways)
      Owner->sendCurrData();
    Owner->setCurrAngle();
    Owner->invalidate();
    }
  pressed = false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IS_DIFF(v1, v2) (fabs((v1) - (v2)) > dPRECISION)
#define DIFF_POINT(p1, p2) CalcDist->isDiffPoint(p1, p2)
//----------------------------------------------------------------------------
void PWinActive::check_and_send(POINT pt, bool send)
{
  if(DIFF_POINT(pt, oldPt)) {
    double old = Owner->valCurr;
    calcDist(pt, oldPt);
    if(IS_DIFF(old, Owner->valCurr)) {
      if(send)
        Owner->sendCurrData();
      Owner->setCurrAngle();
      Owner->invalidate();
      }
    }
}
//----------------------------------------------------------------------------
void PWinActive::calcDist(const POINT& pt, const POINT& old)
{
  double v = CalcDist->getdist(pt, old);
  v *= Owner->valMax - Owner->valMin;
  double n = v + Owner->valCurr;
  if(n > Owner->valMax)
    n = Owner->valMax;
  else if(n < Owner->valMin)
    n = Owner->valMin;
  Owner->valCurr = n;
}
//----------------------------------------------------------------------------
PVarXMeter::PVarXMeter(P_BaseBody* owner, uint id) :  baseVar(owner, id),
    Granularity(0), InitAngle(220), ClockWise(false), Range(120), idImage(0),
      Image(0), Angle(0), offRange(false), Toggle(0), Active(0), Center(false),
      valMin(0), valMax(1), valCurr(0), canUpdate(true)

{
  offsImage.x = 0;
  offsImage.y = 0;
}
//-----------------------------------------------------------------
PVarXMeter::~PVarXMeter()
{
  delete Image;
}
/*
l'immagine per la lancetta va disegnata sempre con il perno a sx e la punta rivolta
a dx, indicante zero gradi.
Nel svMaker si può disegnare anche il cerchio e/o l'arco per visualizzare l'ingombro
completo e/o il range specificato

id,x,y,w,h,idImage,offsX,offsY,initAngle,breadth,clockwise,granularity,idMin,idMax
id+300,prf,addr,type,norm
id+600,nbit,offs,ecc.
*/
//-----------------------------------------------------------------
bool PVarXMeter::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int offsX = 0;
  int offsY = 0;
  int clockWise;
  int idMin = 0;
  int idMax = 0;
  int active = 0;
  int updateAlways = 0;
  int center = 0;
  int vert = 1;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idImage, &offsX, &offsY,
                    &InitAngle, &Range,
                    &clockWise, &Granularity,
                    &idMin, &idMax, &active, &updateAlways, &center, &vert
                    );

  ClockWise = toBool(clockWise);
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));
  setRect(rect);
  ImRect = rect;

  PBitmap* image = getOwner()->getBmp4Btn(idImage - ID_INIT_BMP_4_BTN);
  if(!image)
    return false;
  offsImage.x = offsX;
  offsImage.y = offsY;
  Center = toBool(center);
  Image = new PBitmap(*image);
  calcRectBase();

  if(!makeStdVars())
    return false;

  makeOtherVar(vMax, idMax);
  makeOtherVar(vMin, idMin);
  int mul = 100;
  if(Granularity < 0) {
    mul = -Granularity;
    Granularity = 0;
    }
  static int idWin = 2255;
  if(active) {
    Active = new PWinActive(getOwner(), idWin++, get_Rect(), this, updateAlways, mul, vert);
    return Active->create();
    }
  return true;
}
//-----------------------------------------------------------------
void PVarXMeter::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vMax);
  addReqVar2(allBits, vMin);
}
//-----------------------------------------------------------------
static
BYTE seno[]=
{
    0,1,3,5,7,9,10,12,14,15,17,19,20,22,24,25,27,29,31,32,34,35,
    37,39,40,42,43,45,46,48,50,51,53,54,56,57,58,60,61,63,64,65,67,
    68,69,70,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,
    89,90,91,92,92,93,94,94,95,95,96,96,97,97,97,98,98,98,99,99,99,
    99,99,99,99,99,
    100
};
//-----------------------------------------------------------------
#define COS(i) (seno[90 - i])
#define SIN(i) (seno[i])
  // rotazione cartesiana intorno all'asse z dell'angolo ë
  // x' =   x * cos(ë) + y * sin(ë)
  // y' =  -x * sin(ë) + y * cos(ë)
//-----------------------------------------------------------------
static void getSinCos(int& sinX, int& cosX, int angle)
{
  int quad = 3;
  if(angle <= 90)
    quad = 0;
  else if(angle <= 180) {
    quad = 1;
    angle = 180 - angle;
    }
  else if(angle <= 270) {
    quad = 2;
    angle = angle - 180;
    }
  else
    angle = 360 - angle;

  cosX = COS(angle);
  sinX = SIN(angle);

  switch(quad & 3) {
    case 0:
      break;
    case 1:
      cosX = -cosX;
      break;
    case 2:
      cosX = -cosX;
      sinX = -sinX;
      break;
    case 3:
      sinX = -sinX;
      break;
    }
}
//-----------------------------------------------------------------
static
bool getCoordPix(POINT& target, const POINT& source, int angle, const PRect& check)
{
  LONG& x = target.x;
  LONG& y = target.y;
  int cosX;
  int sinX;
  getSinCos(sinX, cosX, angle);

  x = source.x * cosX + source.y * sinX;
  if(x < 0)
    x -= 50;
  else
    x += 50;
  x /= 100;
  y = -source.x * sinX + source.y * cosX;
  if(y < 0)
    y -= 50;
  else
    y += 50;
  y /= 100;

  if(x <= check.left)
    return false;
  if(x >= check.right)
    return false;
  if(y >= check.top)
    return false;
  if(y <= check.bottom)
    return false;

  return true;
}
//-----------------------------------------------------------------
static
void checkRect(PRect& target, const POINT& pt)
{
  if(target.left > pt.x)
    target.left = pt.x;
  if(target.top > pt.y)
    target.top = pt.y;

  if(target.right < pt.x)
    target.right = pt.x;
  if(target.bottom < pt.y)
    target.bottom = pt.y;
}
//-----------------------------------------------------------------
static
void calcRect(PRect& target, int angle, const PRect& source)
{
  PRect check;
  POINT ptT;
  POINT ptS = { source.left, source.top };
  getCoordPix(ptT, ptS, angle, check);
  checkRect(target, ptT);

  ptS.x = source.right;
  getCoordPix(ptT, ptS, angle, check);
  checkRect(target, ptT);

  ptS.y = source.bottom;
  getCoordPix(ptT, ptS, angle, check);
  checkRect(target, ptT);

  ptS.x = source.left;
  getCoordPix(ptT, ptS, angle, check);
  checkRect(target, ptT);
}
//-----------------------------------------------------------------
void PVarXMeter::calcRectBase()
{
  PRect r(ImRect);
  if(Center) {
    int w = r.Width();
    int h = r.Height();
#if 1
    if(w > h)
      r.Inflate(0, (w - h) / 2);
    else if(w < h)
      r.Inflate((h - w) / 2, 0);
    else
      return;
#else
    int add = w;
    if(w > h)
      r.Inflate(0, (w - h) / 2);
    else if(w < h) {
      add = h;
      r.Inflate((h - w) / 2, 0);
      }
    add = ROUND_REAL(add * 0.36); //ROUND_REAL(add * 1.4 - add) / 2;
    r.Inflate(add, add);
#endif
    setRect(r);
    return;
    }
  // centra il rettangolo sullo zero dato dall'offset
  r.MoveTo(0, 0);
  int offsX = offsImage.x;
  int offsY = offsImage.y;
  r.Offset(-offsX, -offsY);

  PRect rect(get_Rect());
  calcRect(rect, InitAngle, r);
  int end = InitAngle;
  if(ClockWise) {
    end -= Range;
    if(end < 0)
      end += 360;
    if(!end)
      end = 360;
    }
  else
    end += Range;

  calcRect(rect, end % 360, r);
  int init = InitAngle;
  if(ClockWise) {
    init = end;
    end = InitAngle;
    }

  while(init > end) {
    end += 360;
    }

  int q = 0;
  int i;
  for(i = 0; i < 4; ++i, q += 90)
    if(init < q)
      break;

  for(; i < 8; ++i, q += 90) {
    if(end < q)
      break;
    calcRect(rect, q % 360, r);
    }

  // poiché il rettangolo di base ha già la coordinata dx assegnata
  // gli si assegna un margine dovuto all'altezza del bitmap e la si salva
  // per riassegnarla
//  long right = get_Rect().right - offsImage.x + get_Rect().Height() / 2;
  long right = get_Rect().right + get_Rect().Height() / 2;
  // muove il rettangolo risultante per centrarlo sull'offset del perno
  rect.Offset(get_Rect().left + offsX, get_Rect().top + offsY);
  rect.right = right;
  setRect(rect);
}
//-----------------------------------------------------------------
void PVarXMeter::drawImageRotate(HDC hdc)
{
  SIZE szo = Image->getSize();
  float scaleX = ImRect.Width() / (float)szo.cx;
  float scaleY = ImRect.Height() / (float)szo.cy;

  int offsX = Center ? ImRect.Width() / 2 : offsImage.x;
  int offsY = Center ? ImRect.Height() / 2 : offsImage.y;
  float x = float(ImRect.left + offsX);
  float y = float(ImRect.bottom - offsY);
  Gdiplus::Graphics graphics(hdc);
  Gdiplus::Matrix matrix;
  matrix.Scale(scaleX, scaleY);
  matrix.Translate((Gdiplus::REAL)ImRect.left, (Gdiplus::REAL)ImRect.top, Gdiplus::MatrixOrderAppend);
  matrix.RotateAt((Gdiplus::REAL)-Angle, Gdiplus::PointF(x, y), Gdiplus::MatrixOrderAppend);
  graphics.SetTransform(&matrix);
  POINT pt = { 0, 0 };
  PTraspBitmap(getOwner(), Image, pt).Draw(graphics);
  graphics.ResetTransform();
}
//-----------------------------------------------------------------
void PVarXMeter::performDraw(HDC hdc)
{
  if(!hdc)
     return;
  drawImageRotate(hdc);
}
//-----------------------------------------------------------------
void PVarXMeter::setCurrAngle()
{
  setAngle(valMin, valMax, valCurr);
}
//-----------------------------------------------------------------
void PVarXMeter::setAngle(REALDATA v_min, REALDATA v_max, REALDATA vCurr)
{
  offRange = false;
  if(v_min > vCurr) {
    vCurr = v_min;
    offRange = true;
    }
  if(vCurr > v_max) {
    vCurr = v_max;
    offRange = true;
    }
  REALDATA range = v_max - v_min;
  if(range <= 0)
    return;
  REALDATA t = (vCurr - v_min) / range;
  int pos = (int)(t * Range);
  if(ClockWise)
    pos = -pos;
  int newAngle = InitAngle + pos;
  while(newAngle < 0)
    newAngle += 360;
  while(newAngle >= 360)
    newAngle -= 360;


  if(abs(newAngle - Angle) >= Granularity) {
    Angle = newAngle;
    invalidate();
    }
}
//-----------------------------------------------------------------
void PVarXMeter::sendCurrData()
{
  mainClient* mc = getMain();
  genericPerif* prph = mc->getGenPerif(BaseVar.getPrph());
  prfData data;
  data.lAddr = BaseVar.getAddr();
  data.typeVar = BaseVar.getType();

  getDenormalizedData(data, valCurr, getNorm(BaseVar.getNorm()));
  prph->set(data, false);
}
//-----------------------------------------------------------------
bool PVarXMeter::update(bool force)
{
  bool upd = baseVar::update(force);
  bool visible = isVisible();
  if(Active)
    Active->setVisibility(visible);
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
  force |= upd;
  bool changed = force || prfData::notModified != result;

  prfData dataMin;
  result = vMin.getData(getOwner(), dataMin, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vMin.getPrph(), dataMin, result);
    return upd;
    }
  changed |= prfData::notModified != result;

  prfData dataMax;
  result = vMax.getData(getOwner(), dataMax, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vMax.getPrph(), dataMax, result);
    return upd;
    }
  changed |= prfData::notModified != result;

  if(!changed)
    return upd;
  valMin = vMin.getNormalizedResult(dataMin);
  valMax = vMax.getNormalizedResult(dataMax);
  valCurr = BaseVar.getNormalizedResult(data);

  setAngle(valMin, valMax, valCurr);
  return true;
}
