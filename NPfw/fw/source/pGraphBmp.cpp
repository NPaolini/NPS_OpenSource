//-------------------- pGraphBmp.cpp ------------------------
#include "precHeader.h"

#include "pGraphBmp.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PGraphBmp::PGraphBmp(const PRect& rect, COLORREF colorBkg, bStyle type,
            COLORREF colorFrg, PBitmap* bar, bmpstyle barStyle,
            PBitmap* bkg, bmpstyle bkgStyle, bool horz, bool to_right) :
    baseClass(rect, colorBkg, type, colorFrg, horz, to_right), BarBmp(0), BkgBmp(0),
    BarStyle(barStyle), BkgStyle(bkgStyle), BarBmpOrig(0), BkgBmpOrig(0)

{
  if(bar)
    BarBmpOrig = new PBitmap(*bar);
  if(bkg)
    BkgBmpOrig = new PBitmap(*bkg);
  BarBmp = makeBmp(bar, BarStyle);
  BkgBmp = makeBmp(bkg, BkgStyle);
  if(BkgBmp)
    setStyle(PPanel::NO, PPanel::FILL);
}
//-----------------------------------------------------------
PGraphBmp::~PGraphBmp()
{
  delete BarBmp;
  delete BkgBmp;
  delete BarBmpOrig;
  delete BkgBmpOrig;
}
//-----------------------------------------------------------
void PGraphBmp::resize(const PRect& newRect)
{
  PRect r = getRect();
  setRect(newRect);
  if(r.Width() != newRect.Width() || r.Height() != newRect.Height()) {
    delete BarBmp;
    BarBmp = makeBmp(BarBmpOrig, BarStyle);
    delete BkgBmp;
    BkgBmp = makeBmp(BkgBmpOrig, BkgStyle);
    }
}
//-----------------------------------------------------------
void PGraphBmp::draw(HDC hdc)
{
  drawBkg(hdc);
  PRect rect = getRect();
  rect.Inflate(-1, -1);
  if(BkgBmp) {
    POINT pt = { rect.left, rect.top };
    SIZE sz = { rect.Width(), rect.Height() };
    if(bmpMirror & BkgStyle) {
      if(isHorz()) {
        pt.x += sz.cx - 1;
        sz.cx = -sz.cx;
        }
      else {
        pt.y += sz.cy - 1;
        sz.cy = -sz.cy;
        }
      }
    BkgBmp->draw(hdc, pt, sz);
    }
  if(!BarBmp) {
    drawBar(hdc);
    return;
    }
  if(!getPerc())
    return;

  PRect rS(rect);
  rS.MoveTo(0, 0);
  double p = getPerc() / 100.0;

  if(isHorz()) {
    int len = (int)(rect.Width() * p + 0.5);
    if(isToRight()) {
      rect.right = rect.left + len;
      rS.right = len;
      if(bmpMirror & BarStyle) {
        rS.right = 0;
        rS.left = len;
        }
      }
    else {
      rect.left = rect.right - len;
      rS.left = rS.right - len;
      if(bmpMirror & BarStyle) {
        rS.right = 0;
        rS.left = len;
        }
      }
    }
  else { // in verticale le coordinate vanno rovesciate
    int len = (int)(rect.Height() * p + 0.5);
    if(isToRight()) {
      rect.top = rect.bottom - len;
      rS.top = rS.bottom - len;
      if(!(bmpMirror & BarStyle)) {
        rS.bottom = 0;
        rS.top = len;
        }
      }
    else  {
      rect.bottom = rect.top + len;
      rS.bottom = len;
      if(!(bmpMirror & BarStyle)) {
        rS.bottom = 0;
        rS.top = len;
        }
      }
    }
  BarBmp->drawSlice(hdc, rect, rS);
}
//-----------------------------------------------------------
PBitmap* PGraphBmp::resizeBmp(PBitmap* bmp, const PRect& r)
{
  SIZE sz = bmp->getSize();
  double perc = 0;
  if(isHorz()) {
    if(sz.cy == r.Height() || !sz.cy)
      return 0;
    perc = (double)r.Height() / sz.cy;
    }
  else {
    if(sz.cx == r.Width() || !sz.cx)
      return 0;
    perc = (double)r.Width() / sz.cx;
    }
  return PBitmap::resizeBmp(bmp, perc);
}
//-----------------------------------------------------------
PBitmap* PGraphBmp::makeBmp(PBitmap* bmp, DWORD style)
{
  if(!bmp)
    return 0;
  bool old = bmp->setQualityOnStretch(true);
  if(bmpRotate & style)
    bmp = PBitmap::rotate(bmp);
  PRect r(getRect());
  r.Inflate(-1, -1);
  const p_gdiplusBitmap m_pBitmap = bmp->getManBmp();
  Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(r.Width(), r.Height(), m_pBitmap->GetPixelFormat());
  Gdiplus::Graphics graphics(newBitmap);

  if(bmpTiled & style) {
    PBitmap* bmp2 = resizeBmp(bmp, r);
    r.MoveTo(0, 0);
    if(bmp2) {
      bmp2->drawTiled(graphics, r);
      delete bmp2;
      }
    else
      bmp->drawTiled(graphics, r);
    }
  else {
    POINT pt = { 0, 0 };
    SIZE sz = { r.Width(), r.Height() };
    bmp->draw(graphics, pt, sz);
    }
  bmp->setQualityOnStretch(old);
  PBitmap* result = new PBitmap(newBitmap, true);
  if(bmpRotate & style)
    delete bmp;

  result->setQualityOnStretch(old);
  return result;
}
//-----------------------------------------------------------
