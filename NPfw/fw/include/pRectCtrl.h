//-------------------- pRectCtrl.H ---------------------------
#ifndef PRECTCTRL_H_
#define PRECTCTRL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
class PRectCtrl;
#ifndef PRECT_H_
  #include "pRect.h"
#endif
//-----------------------------------------------------------
typedef const SIZE *cPSIZE;
typedef const POINT *cPPOINT;
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class PRectCtrl
{
  public:
    PRectCtrl();
    PRectCtrl(const PRect& r);
    PRectCtrl(const PRectCtrl& r);
    PRectCtrl(const RECT& r);
    PRectCtrl(const POINT& p, const SIZE& s);
    PRectCtrl(int left, int top, int width, int height);

    void Resize(int w, int h);
    void MoveTo(int x, int y);
    void Offset(int dx, int dy);

    void Resize(const SIZE& size);
    void MoveTo(const POINT& point);
    void Offset(const POINT& point);

    bool IsIn(const POINT& pt);

    int Left() const;
    int Top() const;
    int Width() const;
    int Height() const;

    const PRectCtrl& operator =(const PRectCtrl& r);

    operator cPPOINT() const;
    operator PPOINT();

    operator cPSIZE() const;
    operator PSIZE();

    POINT Pt;
    SIZE Sz;
};
//-----------------------------------------------------------
inline PRectCtrl::PRectCtrl()
{
  Pt.x = 0;
  Pt.y = 0;
  Sz.cx = 0;
  Sz.cy = 0;
}
//-----------------------------------------------------------
inline PRectCtrl::PRectCtrl(const PRectCtrl& r) :
  Pt(r.Pt), Sz(r.Sz) {}
//-----------------------------------------------------------
inline PRectCtrl::PRectCtrl(const PRect& r)
{
  Pt.x = r.left;
  Pt.y = r.top;
  Sz.cx = r.Width();
  Sz.cy = r.Height();
}
//-----------------------------------------------------------
inline PRectCtrl::PRectCtrl(const RECT& r)
{
  *this = PRect(r);
//  Pt.x = r.left;
//  Pt.y = r.top;
//  Sz.cx = PRect(r).Width();
//  Sz.cy = PRect(r).Height();
}
//-----------------------------------------------------------
inline PRectCtrl::PRectCtrl(int left, int top, int width, int height)
{
  Pt.x = left;
  Pt.y = top;
  Sz.cx = width;
  Sz.cy = height;
}
//-----------------------------------------------------------
inline PRectCtrl::operator cPPOINT() const {  return &Pt; }
//-----------------------------------------------------------
inline PRectCtrl::operator PPOINT() { return &Pt; }
//-----------------------------------------------------------
inline PRectCtrl::operator cPSIZE() const { return &Sz; }
//-----------------------------------------------------------
inline PRectCtrl::operator PSIZE() { return &Sz; }
//-----------------------------------------------------------
inline const PRectCtrl& PRectCtrl::operator =(const PRectCtrl& r)
{
  if(&r != &*this)
    *this = r;
  return *this;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
inline int PRectCtrl::Left() const { return Pt.x; }
//-----------------------------------------------------------
inline int PRectCtrl::Top() const { return Pt.y; }
//-----------------------------------------------------------
inline int PRectCtrl::Width() const { return Sz.cx; }
//-----------------------------------------------------------
inline int PRectCtrl::Height() const { return Sz.cy; }
//-----------------------------------------------------------
//-----------------------------------------------------------
inline void PRectCtrl::Resize(int w, int h)
{
  Sz.cx = w;
  Sz.cy = h;
}
//-----------------------------------------------------------
inline void PRectCtrl::MoveTo(int x, int y)
{
  Pt.x = x;
  Pt.y = y;
}
//-----------------------------------------------------------
inline void PRectCtrl::Offset(int dx, int dy)
{
  Pt.x += dx;
  Pt.y += dy;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
inline void PRectCtrl::Resize(const SIZE& size) { Sz = size; }
//-----------------------------------------------------------
inline void PRectCtrl::MoveTo(const POINT& point) { Pt = point; }
//-----------------------------------------------------------
inline void PRectCtrl::Offset(const POINT& point)
{
  Pt.x += point.x;
  Pt.y += point.y;
}
//-----------------------------------------------------------
inline bool PRectCtrl::IsIn(const POINT& pt)
{
  POINT pt2 = { Pt.x + Sz.cx, Pt.y + Sz.cy };
  return pt.x >= Pt.x && pt.y >= pt.y && pt.x <= pt2.x && pt.y <= pt2.y;
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif