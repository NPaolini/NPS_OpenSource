//-------------------- pOwnBtnImage.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnImage.h"
//-----------------------------------------------------------------
void trasformBmp(HDC hdc, PBitmap* bmp, float mH, float mS, float mV, bool grayed)
{
#if 1
  #if 0
  // implementato solo da vista in poi ...
  Gdiplus::HueSaturationLightnessParams myHSLParams;

  myHSLParams.hueLevel = 0;
  myHSLParams.lightnessLevel = grayed ? 40: 30;
  myHSLParams.saturationLevel = grayed ? -40: -30;

  Gdiplus::HueSaturationLightness myHSL;
  myHSL.SetParameters(&myHSLParams);
  const p_gdiplusBitmap m_pBitmap = bmp->getManBmp();

  m_pBitmap->ApplyEffect(&myHSL, 0);
  #else
  const float alphaNorm = grayed ? 0.3f : 0.5f;
  const float pClr = 1.0f;
  const float subClr = 0.2f;//-0.2f;
  Gdiplus::ColorMatrix ClrMatrix =
  {
    pClr, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, pClr, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, pClr, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, alphaNorm, 0.0f,
    subClr, subClr, subClr, 0.0f, 1.0f
  };
  Gdiplus::ImageAttributes ImgAttr;
  ImgAttr.SetColorMatrix(&ClrMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
  PBitmap tmp(*bmp);
  const p_gdiplusBitmap m_pBitmap = bmp->getManBmp();

  Gdiplus::Graphics graphics(m_pBitmap);
  Gdiplus::Color color(192, 255, 255, 255);
  graphics.Clear(color);
  SIZE sz = bmp->getSize();
  Gdiplus::Rect r(0, 0, sz.cx, sz.cy);
  const p_gdiplusBitmap m_pBitmap2 = tmp.getManBmp();
  graphics.DrawImage(m_pBitmap2, r, 0, 0, sz.cx, sz.cy, Gdiplus::UnitPixel, &ImgAttr);
  #endif
#else
  HDC mdcS = CreateCompatibleDC(0);
  HDC mdcT = CreateCompatibleDC(0);
  SIZE sz = bmp->getSize();
  HBITMAP hbmpT = CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
  HBITMAP hbmpS = *bmp;
  HGDIOBJ oldS = SelectObject(mdcS, hbmpS);
  HGDIOBJ oldT = SelectObject(mdcT, hbmpT);
  HBRUSH br = (HBRUSH)GetStockObject(WHITE_BRUSH);
  FillRect(mdcT, PRect(0, 0, sz.cx, sz.cy), br);
  BLENDFUNCTION bf;
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = grayed ? 0x7f : 0x9f;  // half of 0xff = 50% transparency
  bf.AlphaFormat = 0;             // ignore source alpha channel

  AlphaBlend(mdcT, 0, 0, sz.cx, sz.cy, mdcS, 0, 0, sz.cx, sz.cy, bf);
  SelectObject(mdcS, oldS);
  SelectObject(mdcT, oldT);
  DeleteObject(hbmpS);
  DeleteDC(mdcS);
  DeleteDC(mdcT);
  bmp->chgHandle(hbmpT);
#endif
}
//-----------------------------------------------------------
POwnBtnImage::~POwnBtnImage()
{
  destroy();
  delete Image;
  delete []Bmp;
}
//-----------------------------------------------------------
void POwnBtnImage::allocImage(const PVect<PBitmap*>& image)
{
  int n = image.getElem();

  if(1 >= n)
    SizeByImage = false;
  if(!n || !image[0]) {
    Image = 0;
    Bmp = 0;
    return;
    }

  Bmp = new PBitmap*[n];
  for(int i = 0; i < n; ++i)
    Bmp[i] = image[i];

  POINT pt = { 0, 0 };
  Image = new PSeqTraspBitmap(this, Bmp, n, pt);

  if(SizeByImage) {
    SIZE sz = Bmp[0]->getSize();
    Attr.w = sz.cx;
    Attr.h = sz.cy;
    }
  else if(!isFlagSet(pfFromResource)) {
    SIZE sz = { Attr.w, Attr.h };
    adjuctImage(sz);
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnBtnImage::setCoord(PRect& rect)
{
  if(SizeByImage) {
    SIZE sz = Bmp[0]->getSize();
    rect.right = rect.left + sz.cx;
    rect.bottom = rect.top + sz.cy;
    }
  else {
    SIZE sz = { rect.Width(), rect.Height() };
    adjuctImage(sz);
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnBtnImage::drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(Image && Image->getNBmp() > 2) {
    int ix = 2;
    if(lpDIS->itemState & ODS_SELECTED)
      if(Image->getNBmp() > 3)
        ix = 3;
    POINT pt = Image->getPoint();
    PTraspBitmap focus(getParent(), Bmp[ix], pt);
    focus.setScale(Image->getScaleX(), Image->getScaleY());
    focus.Draw(hdc);
    PRect r = getRect();
    r.MoveTo(0, 0);
    drawText(hdc, r, (ODS_SELECTED & lpDIS->itemState) ? POwnBtn::pushed : POwnBtn::released);
    }
  else {
    if(Image && Image->getNBmp() > 1) {
      PRect r = getRect();
      r.MoveTo(0, 0);
      drawText(hdc, r, (ODS_SELECTED & lpDIS->itemState) ? POwnBtn::pushed : POwnBtn::released);
      }
    POwnBtn::drawFocusAfter(hdc, lpDIS);
    }
}
//-----------------------------------------------------------
#define INFLATE_BMP_WIDTH 10
#define INFLATE_BMP_HEIGHT 10
//-----------------------------------------------------------
#define EDGE_BMP_X (INFLATE_BMP_WIDTH / 2)
#define EDGE_BMP_Y (INFLATE_BMP_HEIGHT / 2)
//-----------------------------------------------------------
#define EDGE_BTN_X (rectColor.tickness)
#define EDGE_BTN_Y EDGE_BTN_X
//-----------------------------------------------------------
#define RESCALE 0.97
//-----------------------------------------------------------
#define DIM_BORDER GetSystemMetrics(SM_CXEDGE)
//-----------------------------------------------------------
#define DEF_POINT  { EDGE_BMP_X + EDGE_BTN_X, EDGE_BMP_Y + EDGE_BTN_Y }
//#define DEF_POINT  { EDGE_BMP_X + EDGE_BTN_X, EDGE_BMP_Y + EDGE_BTN_Y }
//-----------------------------------------------------------
void POwnBtnImage::drawSelected(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(Image && Image->getNBmp() > 1) {
    Image->setCurr(1, hdc);
    if(!(lpDIS->itemState & ODS_FOCUS)) {
      PRect r(getRect());
      r.MoveTo(0, 0);
      drawText(hdc, r, POwnBtn::pushed);
      }
    }
  else
    POwnBtn::drawSelected(hdc, lpDIS);
}
//-----------------------------------------------------------
void POwnBtnImage::drawUnselected(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(Image && Image->getNBmp() > 1) {
    Image->setCurr(0, hdc);
    if(!(lpDIS->itemState & ODS_FOCUS)) {
      PRect r(getRect());
      r.MoveTo(0, 0);
      drawText(hdc, r, POwnBtn::released);
      }
    }
  else
    POwnBtn::drawUnselected(hdc, lpDIS);
}
//-----------------------------------------------------------
#define EDGE_BTN_X (rectColor.tickness)
#define EDGE_BTN_Y EDGE_BTN_X
//-----------------------------------------------------------
void POwnBtnImage::drawDisabled(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(Image && Image->getNBmp() > 1) {
    uint ix = 0;
    if(ODS_SELECTED & lpDIS->itemState)
      ix = 1;
    PBitmap* orig = Image->getBmp(ix);
    PBitmap* tmp = new PBitmap(orig->clone(hdc), true);
    const float mH = 1.0f;
    const float mS = 0.3f;
    const float mV = 1.02f;
//    trasformBmp(hdc, tmp, mH, mS, mV, true);
    trasformBmp(hdc, tmp, mH, mS, mV, false);
    Image->replaceBmp(tmp, ix);
    Image->setCurr(ix, hdc);

    PRect r(getRect());
    r.MoveTo(0, 0);
    drawText(hdc, r, POwnBtn::disabled);

    delete Image->replaceBmp(orig, ix);
    }
  else
    POwnBtn::drawDisabled(hdc, lpDIS);
}
//-----------------------------------------------------------
void POwnBtnImage::drawText(HDC hdc, const PRect& rect, POwnBtn::howDraw style)
{
  if(getCaption()) {
    if(Image  && Image->getNBmp() == 1) {
      SIZE sz = Bmp[0]->getSize();
      double scale = style == pushed ? oldScale * RESCALE : oldScale;
      sz.cx = (LONG)(sz.cx * scale);
      int x1 = sz.cx + EDGE_BTN_X + EDGE_BMP_X;
      int y1 = EDGE_BTN_Y + EDGE_BMP_Y;
      PRect r;
      GetWindowRect(*this, r);
      PRect r2(x1, y1, r.Width() - EDGE_BTN_X, r.Height() - EDGE_BTN_Y);
      POwnBtn::drawText(hdc, r2, style);
      }
    else
      POwnBtn::drawText(hdc, rect, style);
    }
}
//-----------------------------------------------------------
void POwnBtnImage::drawCustom(HDC hdc, const PRect& /*rect*/, POwnBtn::howDraw style)
{
  if(Image && Image->getNBmp() == 1) {
    double scale = oldScale;
    POINT pt = DEF_POINT;
    PBitmap* orig = 0;
    if(style == pushed) {

      scale *= RESCALE;
      SIZE sz = Bmp[0]->getSize();
      double width = sz.cx * oldScale;
      int width2 = (int)(width * RESCALE);
      int move = (int)(width - width2);
      move += EDGE_BMP_X + EDGE_BTN_X + 1;
      pt.x = move;
      pt.y = move;
      }
    else if(POwnBtn::disabled == style) {
      // disegnare l'immagine in grayed
      orig = Image->getBmp(0);
      PBitmap* tmp = new PBitmap(*orig);
      const float mH = 1.2f;
      const float mS = 0.2f;
      const float mV = 3;
      trasformBmp(hdc, tmp, mH, mS, mV, true);
      Image->replaceBmp(tmp, 0);
      }
    Image->setScale(scale);
#if 1
    Image->moveToSimple(pt);
    Image->Draw(hdc);
#else
    Image->moveTo(pt, hdc);
#endif
    if(orig)
      delete Image->replaceBmp(orig, 0);
    }
}
//-----------------------------------------------------------
double scaleImage(const SIZE& winSz, SIZE& bmpSz)
{
  if(!bmpSz.cx || !bmpSz.cy)
    bmpSz = winSz;

  SIZE wSz = winSz;
  wSz.cx -= INFLATE_BMP_WIDTH + (INFLATE_BMP_WIDTH / 2);
  wSz.cy -= INFLATE_BMP_HEIGHT + (INFLATE_BMP_HEIGHT / 2);

  double scaleX = wSz.cx;
  scaleX /= bmpSz.cx;

  double scaleY = wSz.cy;
  scaleY /= bmpSz.cy;

  double scale = scaleX > scaleY ? scaleY : scaleX;

  while(wSz.cy < scale * bmpSz.cy || wSz.cx < scale * bmpSz.cx) {
    if(scale < 0.05)
      break;
    scale -= 0.01;
    }
  if(scale != 1.0) {
    bmpSz.cy = (LONG)(bmpSz.cy * scale);
    bmpSz.cx = (LONG)(bmpSz.cx * scale);
    }
  return scale;
}
//-----------------------------------------------------------
void POwnBtnImage::adjuctImage(const SIZE& winSz)
{
  if(Image) {
    SIZE sz = Bmp[0]->getSize();
    if(!sz.cx || !sz.cy)
      sz = winSz;

    POINT pt = DEF_POINT;
    SIZE wSz = winSz;
    if(Image->getNBmp() > 1) {
/*
      pt.x = (wSz.cx - sz.cx) / 2;
      pt.y = (wSz.cy - sz.cy) / 2;
      if(pt.x < 0)
        pt.x = 0;
      if(pt.y < 0)
        pt.y = 0;
*/
      pt.x = 0;
      pt.y = 0;
      }
    else {
      wSz.cx -= INFLATE_BMP_WIDTH + EDGE_BTN_X * 2;
      wSz.cy -= INFLATE_BMP_HEIGHT + EDGE_BTN_Y * 2;
      }

    double scaleX = wSz.cx;
    scaleX /= sz.cx;

    double scaleY = wSz.cy;
    scaleY /= sz.cy;

    oldScale = scaleX > scaleY ? scaleY : scaleX;

    if(1 == Image->getNBmp()) {
      while(wSz.cy < oldScale * sz.cy || wSz.cx < oldScale * sz.cx) {
        if(oldScale < 0.05)
          break;
        oldScale -= 0.01;
        }
      }
    if(oldScale != 1.0)
      Image->setScale(oldScale);

    Image->moveToSimple(pt);
    }
}
//-----------------------------------------------------------
void POwnBtnImage::setColorRect(const colorRect& color)
{
  uint oldTick = rectColor.tickness;
  POwnBtn::setColorRect(color);
  if(oldTick != rectColor.tickness) {
    PRect r;
    GetWindowRect(*this, r);
    SIZE sz = { r.Width(), r.Height() };
    adjuctImage(sz);
    }
}
//-----------------------------------------------------------

