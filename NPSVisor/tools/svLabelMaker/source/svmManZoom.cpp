//------------------ svmManZoom.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmManZoom.h"
//-----------------------------------------------------------
static bool getMulDiv(svmManZoom::zoomX curr, int& m, int& d)
{
  m = 1;
  d = 1;
  switch(curr) {
    case svmManZoom::zOne:
    default:
      return false;
    case svmManZoom::z_1_2:
      d = 2;
      break;
    case svmManZoom::z_2_3:
      m = 2;
      d = 3;
      break;
    case svmManZoom::z_3_2:
      m = 3;
      d = 2;
      break;
    case svmManZoom::z_2:
      m = 2;
      break;
    case svmManZoom::z_3:
      m = 3;
      break;
    case svmManZoom::z_4:
      m = 4;
      break;
    }
  return true;
}
//-----------------------------------------------------------
infoAdjusct svmManZoom::Adjuct;
//-----------------------------------------------------------
#define MUL_DIV_TO(a, v) a = long(MulDiv((a), Mul, Div) * Adjuct.v)
#define MUL_DIV_FROM(a, v) a = long(MulDiv((a), Div, Mul) / Adjuct.v)
//-----------------------------------------------------------
#define MUL_DIV_TO_R(t, a, v) do { (a) *= Mul; (a) /= Div; (a) = t((a) * Adjuct.v); } while(false)
#define MUL_DIV_FROM_R(t, a, v) do { (a) *= Div; (a) /= Mul; (a) = t((a) / Adjuct.v); } while(false)
//-----------------------------------------------------------
bool svmManZoom::calcFromScreen(PRect& r)
{
  MUL_DIV_FROM(r.left, H);
  MUL_DIV_FROM(r.right, H);
  MUL_DIV_FROM(r.top, V);
  MUL_DIV_FROM(r.bottom, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreen(SIZE& s)
{
  MUL_DIV_FROM(s.cx, H);
  MUL_DIV_FROM(s.cy, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenV(LONG& v)
{
  MUL_DIV_FROM(v, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenV(FLOAT& s)
{
  MUL_DIV_FROM_R(FLOAT, s, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenV(double& s)
{
  MUL_DIV_FROM_R(double, s, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenH(LONG& v)
{
  MUL_DIV_FROM(v, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenH(FLOAT& s)
{
  MUL_DIV_FROM_R(FLOAT, s, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreenH(double& s)
{
  MUL_DIV_FROM_R(double, s, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreen(PRect& r)
{
  MUL_DIV_TO(r.left, H);
  MUL_DIV_TO(r.right, H);
  MUL_DIV_TO(r.top, V);
  MUL_DIV_TO(r.bottom, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreen(SIZE& s)
{
  MUL_DIV_TO(s.cx, H);
  MUL_DIV_TO(s.cy, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenH(LONG& v)
{
  MUL_DIV_TO(v, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenH(FLOAT& s)
{
  MUL_DIV_TO_R(FLOAT, s, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenH(double& s)
{
  MUL_DIV_TO_R(double, s, H);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenV(LONG& v)
{
  MUL_DIV_TO(v, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenV(FLOAT& s)
{
  MUL_DIV_TO_R(FLOAT, s, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreenV(double& s)
{
  MUL_DIV_TO_R(double, s, V);
  return zOne != CurrZoom;
}
//-----------------------------------------------------------
void svmManZoom::setCurrZoom(zoomX curr)
{
  CurrZoom = curr;
  getMulDiv(CurrZoom, Mul, Div);
}
//-----------------------------------------------------------
