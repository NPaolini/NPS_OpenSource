//-------------------- pRect.H ---------------------------
#ifndef PRECT_H_
#define PRECT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
typedef const RECT *cPRECT;
//-----------------------------------------------------------
class PRect
{
  public:
    PRect();
    PRect(const PRect& r);
    PRect(const RECT& r);
    PRect(int left, int top, int right, int bottom);

    void Inflate(int w, int h);
    void MoveTo(int x, int y);
    void Offset(int dx, int dy);
    void Normalize();

    bool IsIn(const POINT& pt) const;
    bool IsIn(const POINTS& pt) const;

    // sia il rettangolo interno che quello passato
    // devono essere normalizzati
    bool Intersec(const PRect& r) const;
    PRect operator |(const PRect& other) const;
    PRect operator |=(const PRect& other);
    PRect operator &(const PRect& other) const;
    PRect operator &=(const PRect& other);

    int Width() const;
    int Height() const;

//    const PRect& operator =(const PRect& r);

    operator cPRECT() const;
    operator PRECT();

    int left;
    int top;
    int right;
    int bottom;
};
//-----------------------------------------------------------
inline PRect::PRect() : left(0), top(0), right(0), bottom(0) {}
//-----------------------------------------------------------
inline PRect::PRect(const PRect& r) :
  left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}
//-----------------------------------------------------------
inline PRect::PRect(const RECT& r) :
  left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}
//-----------------------------------------------------------
inline PRect::PRect(int left, int top, int right, int bottom) :
  left(left), top(top), right(right), bottom(bottom) {}
//-----------------------------------------------------------
inline PRect::operator cPRECT() const { return reinterpret_cast<cPRECT>(this); }
//-----------------------------------------------------------
inline PRect::operator PRECT() { return reinterpret_cast<PRECT>(this); }
//-----------------------------------------------------------
/*
inline const PRect& PRect::operator =(const PRect& r)
{
  if(&r != this)
    *this = r;
  return *this;
}
*/
//-----------------------------------------------------------
inline void PRect::Inflate(int w, int h)
{
  left -= w;
  right += w;
  top -= h;
  bottom += h;
}
//-----------------------------------------------------------
inline void PRect::MoveTo(int x, int y)
{
  int w = Width();
  int h = Height();
  left = x;
  right = x + w;
  top = y;
  bottom = y + h;
}
//-----------------------------------------------------------
inline void PRect::Offset(int dx, int dy)
{
  left += dx;
  right += dx;
  top += dy;
  bottom += dy;
}
//-----------------------------------------------------------
inline int PRect::Width() const
{
  return right - left;
}
//-----------------------------------------------------------
inline int PRect::Height() const
{
  return bottom - top;
}
//-----------------------------------------------------------
inline void PRect::Normalize()
{
  if(right < left) {
    int t = right;
    right = left;
    left = t;
    }
  if(bottom < top) {
    int t = bottom;
    bottom = top;
    top = t;
    }
}
//-----------------------------------------------------------
inline bool PRect::IsIn(const POINT& pt) const
{
  return pt.x >= left && pt.y >= top && pt.x <= right && pt.y <= bottom;
}
//-----------------------------------------------------------
inline bool PRect::IsIn(const POINTS& pt) const
{
  return pt.x >= left && pt.y >= top && pt.x <= right && pt.y <= bottom;
}
//-----------------------------------------------------------
inline bool PRect::Intersec(const PRect& r) const
{
  if(r.left > right || r.top > bottom || r.right < left || r.bottom < top)
    return false;
  return true;
}
//-----------------------------------------------------------
#ifdef __BORLANDC__
//#if __TCPLUSPLUS__ <= 0x520
//#else
//#ifdef __BORLANDC__
template <class T>
inline T min(T a, T b) { return a < b ? a : b; }
template <class T>
inline T max(T a, T b) { return a > b ? a : b; }

int min(int, int);
int max(int, int);
//#endif
#endif

#define MIN_R(a) r.a = min(a, other.a)
#define MAX_R(a) r.a = max(a, other.a)
inline PRect PRect::operator |(const PRect& other) const
{
  PRect r(*this);
  MIN_R(left);
  MIN_R(top);
  MAX_R(right);
  MAX_R(bottom);
  return r;
}
//-----------------------------------------------------------
inline PRect PRect::operator &(const PRect& other) const
{
  PRect r(*this);
  MAX_R(left);
  MAX_R(top);
  MIN_R(right);
  MIN_R(bottom);
  if(r.left > r.right || r.top > r.bottom) {
    r.left = 0;
    r.right = 0;
    r.top = 0;
    r.bottom = 0;
    }
  return r;
}
#undef MIN_R
#undef MAX_R
//-----------------------------------------------------------
#define MIN_RE(a) a = min(a, other.a)
#define MAX_RE(a) a = max(a, other.a)
inline PRect PRect::operator |=(const PRect& other)
{
  MIN_RE(left);
  MIN_RE(top);
  MAX_RE(right);
  MAX_RE(bottom);
  return *this;
}
//-----------------------------------------------------------
inline PRect PRect::operator &=(const PRect& other)
{
  MAX_RE(left);
  MAX_RE(top);
  MIN_RE(right);
  MIN_RE(bottom);
  if(left > right || top > bottom) {
    left = 0;
    right = 0;
    top = 0;
    bottom = 0;
    }
  return *this;
}
#undef MIN_R
#undef MAX_R
#undef MIN_RE
#undef MAX_RE
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
