//----------- lineEquation.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "lineEquation.h"
#include "p_util.h"
//----------------------------------------------------------------------------
void lineEquation::makeEquation()
{
  if(pt1.x == pt2.x)
    is_vert = true;
  else {
    if(pt2.y == pt1.y) {
      A = 0;
      is_horz = true;
      }
    else {
      A = pt2.y - pt1.y;
      is_horz = false;
      }
    double tmp = pt2.x - pt1.x;
    A /= tmp;
    K = pt1.y - A * pt1.x;
    is_vert = false;
    }
}
//-----------------------------------------------------------
int lineEquation::getX(int y) const
{
  if(isVert())
    return pt1.x;
  if(isHorz())
    return y;
  double v = (y - K) / A;
  return ROUND_REAL(v);
}
//-----------------------------------------------------------
int lineEquation::getY(int x) const
{
  if(isVert())
    return x;
  double v = x * A + K;
  return ROUND_REAL(v);
}
//-----------------------------------------------------------
lineEquation::eSide lineEquation::whereIs(POINT pt) const
{
  if(isVert())
    return pt.x < pt1.x ? eLeft : pt1.x < pt.x ? eRight : eInside;
  if(isHorz())
    return pt.y > pt1.y ? eTop : pt1.y > pt.y ? eBottom : eInside;

  int y = getY(pt.x);
  if(y == pt.y)
    return eInside;

  int x = getX(pt.y);

  if(y < pt.y) {
    if(x < pt.x)
      return eBottomRight;
    if(x > pt.x)
      return eBottomLeft;
    return eBottom;
    }
  if(x < pt.x)
    return eTopRight;
  if(x > pt.x)
    return eTopLeft;
  return eTop;
}
//----------------------------------------------------------------------------
