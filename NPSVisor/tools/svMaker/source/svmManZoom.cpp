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
#define MUL_DIV_TO(a) a = MulDiv((a), Mul, Div)
#define MUL_DIV_FROM(a) a = MulDiv((a), Div, Mul)
//-----------------------------------------------------------
bool svmManZoom::calcFromScreen(PRect& r)
{
  if(zOne == CurrZoom)
    return false;

  MUL_DIV_FROM(r.left);
  MUL_DIV_FROM(r.right);
  MUL_DIV_FROM(r.top);
  MUL_DIV_FROM(r.bottom);
  return true;
}
//-----------------------------------------------------------
bool svmManZoom::calcFromScreen(SIZE& s)
{
  if(zOne == CurrZoom)
    return false;

  MUL_DIV_FROM(s.cx);
  MUL_DIV_FROM(s.cy);
  return true;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreen(PRect& r)
{
  if(zOne == CurrZoom)
    return false;

  MUL_DIV_TO(r.left);
  MUL_DIV_TO(r.right);
  MUL_DIV_TO(r.top);
  MUL_DIV_TO(r.bottom);
  return true;
}
//-----------------------------------------------------------
bool svmManZoom::calcToScreen(SIZE& s)
{
  if(zOne == CurrZoom)
    return false;

  MUL_DIV_TO(s.cx);
  MUL_DIV_TO(s.cy);
  return true;
}
//-----------------------------------------------------------
void svmManZoom::setCurrZoom(zoomX curr)
{
  CurrZoom = curr;
  getMulDiv(CurrZoom, Mul, Div);
}
