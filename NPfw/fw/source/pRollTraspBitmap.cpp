//-------------------- pRollTraspBitmap.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pRollTraspBitmap.h"
//-----------------------------------------------------------
//----------------------------------------------------------------------------
//-----------------------------------------------------------
PRollTraspBitmap::PRollTraspBitmap(PWin* parent, PBitmap *bmp, const POINT& pt,
            const POINT& step, bool autodelete) :
       PTraspBitmap(parent, bmp, pt, autodelete), Step(step)
{
  ptCurrent.x = 0;
  ptCurrent.y = 0;
  const p_gdiplusBitmap pBmp = bmp->getManBmp();
  if(pBmp)
    pBmp->GetPixel(0, 0, &TranspPix);
}
//-----------------------------------------------------------
//----------------------------------------------------------------------------
void PRollTraspBitmap::Draw(HDC hdc)
{
  const p_gdiplusBitmap pBmp = getBmp()->getManBmp();
  if(!pBmp)
    return;
#if 0
  bool needTransp = !(Gdiplus::ImageFlagsHasAlpha & pBmp->GetFlags());
#else
  Gdiplus::PixelFormat pxf = pBmp->GetPixelFormat();
  bool needTransp = true;
  switch(pxf) {
    case PixelFormat16bppARGB1555:
    case PixelFormat32bppARGB:
    case PixelFormat32bppPARGB:
    case PixelFormat64bppARGB:
    case PixelFormat64bppPARGB:
      needTransp = false;
      break;
    }
#endif
  SIZE sz = { pBmp->GetWidth(), pBmp->GetHeight() };

  ptCurrent.x %= sz.cx;
  ptCurrent.y %= sz.cy;

  int tx = ptCurrent.x;
  int ty = ptCurrent.y;
  if(tx < 0)
    tx += sz.cx;
  if(ty < 0)
    ty += sz.cy;

  Gdiplus::Graphics graphics(hdc);
  Gdiplus::ImageAttributes* pImAtt = 0;
  Gdiplus::ImageAttributes imAtt;
  if(needTransp) {
    imAtt.SetColorKey( TranspPix, TranspPix, Gdiplus::ColorAdjustTypeBitmap);
    pImAtt = &imAtt;
    }
#if 0
    Gdiplus::Rect target(Pos.x, Pos.y, sz.cx,  sz.cy);
    graphics.DrawImage(pBmp, target, 0, 0,  sz.cx,  sz.cy, Gdiplus::UnitPixel, pImAtt);

#else

  int x[] = { 0, tx, sz.cx - tx };
  int y[] = { 0, ty, sz.cy - ty };
  if(x[1] && y[1]) {
    Gdiplus::Rect target(x[0], y[0], x[1], y[1]);
    graphics.DrawImage(pBmp, target, x[2], y[2], x[1], y[1], Gdiplus::UnitPixel, pImAtt);
    }
  if(x[2] && y[1]) {
    Gdiplus::Rect target(x[1], y[0], x[2], y[1]);
    graphics.DrawImage(pBmp, target, x[0], y[2], x[2], y[1], Gdiplus::UnitPixel, pImAtt);
    }
  if(x[1] && y[2]) {
    Gdiplus::Rect target(x[0], y[1], x[1], y[2]);
    graphics.DrawImage(pBmp, target, x[2], y[0], x[1], y[2], Gdiplus::UnitPixel, pImAtt);
    }
  if(x[2] && y[2]) {
    Gdiplus::Rect target(x[1], y[1], x[2], y[2]);
    graphics.DrawImage(pBmp, target, x[0], y[0], x[2], y[2], Gdiplus::UnitPixel, pImAtt);
    }
#endif
}
//----------------------------------------------------------------------------
