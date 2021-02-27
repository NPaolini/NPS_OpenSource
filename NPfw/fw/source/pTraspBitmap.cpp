//-------------------- pTraspBitmap.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pTraspBitmap.h"
#include "p_Util.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
static void chgBkgBmp(HDC hdc, HBITMAP hbmpFrg, HBITMAP hbmpBkg);

//----------------------------------------------------------------------------
PTraspBitmapBase::PTraspBitmapBase(PWin* parent, const POINT& pos) :
  ScaleX(0.0), ScaleY(0.0), Pos(pos), Parent(parent), Enabled(true)
{ }
//----------------------------------------------------------------------------
PTraspBitmapBase::~PTraspBitmapBase() {  }
//----------------------------------------------------------------------------
void PTraspBitmapBase::moveTo(const POINT& newpos)
{
  bool old = isEnabled();
  setEnable(true);
  Invalidate();
  moveToSimple(newpos);
  setEnable(old);
  Invalidate();
}
//----------------------------------------------------------------------------
void PTraspBitmapBase::setScale(double scaleX, double scaleY)
{
  ScaleX = scaleX;
  ScaleY = SAME_SCALE == scaleY ? scaleX : scaleY;
}
//----------------------------------------------------------------------------
void PTraspBitmapBase::Invalidate()
{
  if(!isEnabled() || !Parent)
    return;
  PRect r = getRect();
  InvalidateRect(*Parent, r, 0);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void PTraspBitmapBase::moveToSimple(const POINT& newpos)
{
  Pos = newpos;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void PTraspBitmapBase::drawBitmap(HDC hdc, const PBitmap& Bmp)
{
  if(!isEnabled())
    return;
  Gdiplus::Graphics graphics(hdc);
  drawBitmap(graphics, Bmp);
}
//----------------------------------------------------------------------------
void PTraspBitmapBase::drawBitmap(Gdiplus::Graphics& graphics, const PBitmap& Bmp)
{
  if(!isEnabled())
    return;
  SIZE sz = Bmp.getSize();

  // se necessita di una scalatura
  if(ScaleX && ScaleX != 1.0)
    sz.cx = (long)(sz.cx * ScaleX + 0.5);
  if(ScaleY && ScaleY != 1.0)
    sz.cy = (long)(sz.cy * ScaleY + 0.5);

  PRect target(0, 0, sz.cx, sz.cy);
  target.MoveTo(Pos.x, Pos.y);
  Bmp.drawTransp(graphics, target);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PTraspBitmap::PTraspBitmap(PWin* parent, PBitmap *bmp, const POINT& pos, bool autodelete) :
  PTraspBitmapBase(parent, pos), Bmp(bmp), autoDelete(autodelete)  { }
//----------------------------------------------------------------------------
PTraspBitmap::~PTraspBitmap()
{
  if(autoDelete)
    delete Bmp;
}
//----------------------------------------------------------------------------
PTraspBitmap::PTraspBitmap(const PTraspBitmap& orig)
{
  // richiama l'assegnazione
  *this = orig;
}
//----------------------------------------------------------------------------
PTraspBitmap& PTraspBitmap::operator = (const PTraspBitmap& orig)
{
  if(&orig != this) {
    Bmp = orig.Bmp;
    Pos = orig.Pos;
    Parent = orig.Parent;
    ScaleX = orig.ScaleX;
    ScaleY = orig.ScaleY;
    Enabled = orig.Enabled;
    autoDelete = false;
    }
  return *this;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void PTraspBitmap::Draw(HDC hdc)
{
  if(!Bmp || !Bmp->isValid())
    return;
  drawBitmap(hdc, *Bmp);
}
//----------------------------------------------------------------------------
void PTraspBitmap::Draw(Gdiplus::Graphics& graphics)
{
  if(!Bmp || !Bmp->isValid())
    return;
  drawBitmap(graphics, *Bmp);
}
//----------------------------------------------------------------------------
PRect PTraspBitmap::getRect() const
{
  PRect r;
  if(Bmp && Bmp->isValid()) {
    SIZE sz = Bmp->getSize();
    if(ScaleX && ScaleX != 1.0)
      sz.cx = (long)(sz.cx * ScaleX);
    if(ScaleY && ScaleY != 1.0)
      sz.cy = (long)(sz.cy * ScaleX);
    r = PRect(Pos.x, Pos.y, sz.cx + Pos.x, sz.cy + Pos.y);
    }
  return r;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
