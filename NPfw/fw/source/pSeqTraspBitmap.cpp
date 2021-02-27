//-------------------- pSeqTraspBitmap.cpp -----------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pSeqTraspBitmap.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PSeqTraspBitmap::PSeqTraspBitmap(PWin* parent, PBitmap **bmp, UINT nbmp, const POINT& pos) :
    PTraspBitmapBase(parent, pos), Bmp(bmp), nBmp(nbmp), currBmp(0), dontUseTransp(false)  { }
//----------------------------------------------------------------------------
PSeqTraspBitmap::~PSeqTraspBitmap() {  }
//----------------------------------------------------------------------------
PSeqTraspBitmap::PSeqTraspBitmap(const PSeqTraspBitmap& orig)
{
  *this = orig;
}
//----------------------------------------------------------------------------
PSeqTraspBitmap& PSeqTraspBitmap::operator = (const PSeqTraspBitmap& orig)
{
  if(&orig != this) {
    Bmp = orig.Bmp;
    nBmp = orig.nBmp;
    Pos = orig.Pos;
    Parent = orig.Parent;
    ScaleX = orig.ScaleX;
    ScaleY = orig.ScaleY;
    currBmp = orig.currBmp;
    Enabled = orig.Enabled;
    dontUseTransp = orig.dontUseTransp;
    }
  return *this;
}
//----------------------------------------------------------------------------
void PSeqTraspBitmap::setCurr(UINT curr, bool repaint)
{
  if(curr < nBmp) {
    currBmp = curr;
    if(repaint)
      PTraspBitmapBase::Invalidate();
    }
}
//----------------------------------------------------------------------------
void PSeqTraspBitmap::setCurr(UINT curr, HDC hdc)
{
  if(curr < nBmp) {
    currBmp = curr;
    Draw(hdc);
    }
}
//----------------------------------------------------------------------------
void PSeqTraspBitmap::FullDraw(HDC hdc)
{
  SIZE sz = Bmp[currBmp]->getSize();
  if(ScaleX)
    sz.cx = (LONG)(sz.cx * ScaleX);
  if(ScaleY)
    sz.cy = (LONG)(sz.cy * ScaleY);
  Bmp[currBmp]->draw(hdc, Pos, sz);
}
//----------------------------------------------------------------------------
void PSeqTraspBitmap::Draw(HDC hdc)
{
  if(dontUseTransp)
     FullDraw(hdc);
  else
    drawBitmap(hdc, *Bmp[currBmp]);
}
//----------------------------------------------------------------------------
PBitmap* PSeqTraspBitmap::replaceBmp(PBitmap* other, uint pos)
{
  if(pos >= nBmp)
    return 0;
  SIZE sz1 = other->getSize();
  PBitmap* tmp = Bmp[pos];
  SIZE sz2 = tmp->getSize();
  if(sz1.cx == sz2.cx && sz1.cy == sz2.cy) {
    Bmp[pos] = other;
//    if(pos == currBmp)
//      PTraspBitmapBase::Draw();
    return tmp;
    }
  return 0;
}
//----------------------------------------------------------------------------
PBitmap* PSeqTraspBitmap::getBmp(uint pos)
{
  if(pos >= nBmp)
    return 0;
  return Bmp[pos];
}
//----------------------------------------------------------------------------
PRect PSeqTraspBitmap::getRect() const
{
  PRect r;
  if(nBmp) {
    SIZE sz = Bmp[0]->getSize();
    if(ScaleX && ScaleX != 1.0)
      sz.cx = (long)(sz.cx * ScaleX);
    if(ScaleY && ScaleY != 1.0)
      sz.cy = (long)(sz.cy * ScaleX);
    r = PRect(Pos.x, Pos.y, sz.cx + Pos.x, sz.cy + Pos.y);
    }
  return r;
}
