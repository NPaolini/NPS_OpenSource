//----------- lineEquation.h -------------------------------------------------
#ifndef lineEquation_H_
#define lineEquation_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
class lineEquation
{
  public:
    lineEquation() : is_vert(false), is_horz(false), A(0), K(0)
    { pt1.x = 0; pt1.y = 0; pt2.x = 0; pt2.y = 0;  }
    lineEquation(const POINT& pt1, const POINT& pt2) : pt1(pt1), pt2(pt2), is_vert(false), is_horz(false) { makeEquation(); }
    enum eSide { eLeft, eTop, eRight, eBottom, eTopLeft, eTopRight, eBottomLeft, eBottomRight, eInside };
    eSide whereIs(POINT pt) const;
    int getX(int y) const;
    int getY(int x) const;
    bool isVert() const { return is_vert; }
    bool isHorz() const { return is_horz; }

    double getAngle() const { return A; }
    double getKonst() const { return K; }
    void setPoints(const POINT& pt_1, const POINT& pt_2)
    {
      pt1 = pt_1;
      pt2 = pt_2;
      makeEquation();
    }
    const POINT& getPt1() const { return pt1; }
    const POINT& getPt2() const { return pt2; }
  private:
    void makeEquation();
    POINT pt1;
    POINT pt2;
    bool is_vert;
    bool is_horz;
    double A;
    double K;
};
//------------------------------------------------------------------------------
#endif
