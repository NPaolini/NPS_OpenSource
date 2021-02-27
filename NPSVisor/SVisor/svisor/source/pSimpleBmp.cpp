//------ PSimpleBmp.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "PSimpleBmp.h"
#include "p_util.h"
//----------------------------------------------------------------------------
static void getCoord(PWin* win, int flag, PBitmap* bmp, POINT& pt, SIZE& scale);
//----------------------------------------------------------------------------
PSimpleBmp::PSimpleBmp(P_BaseBody* owner, uint id) : baseSimple(owner, id),
           Bmp(0), Flag(0), Transp(false) {}
//----------------------------------------------------------------------------
PSimpleBmp::~PSimpleBmp()
{
  delete Bmp;
}
//----------------------------------------------------------------------------
bool PSimpleBmp::allocObj(LPVOID)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int scaleX = 0;
  int scaleY = 0;
  Flag = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &scaleX, &scaleY, &Flag);

  Transp = false;
  if(Flag >= 1000) {
    Transp = true;
    Flag -= 1000;
    }

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  LPCTSTR name = findNextParamTrim(p, 5);

  if(!name || !*name)
    return false;

  Bmp = new PBitmap(name);

  if(!Bmp->isValid())
    return false;
  // se metafile le dimensioni riportate dipendono dalla risoluzione video,
  // per cui non si può fare affidamento sulla scala. Se i valori delle scale
  // sono negativi indicano che si tratta di dimensioni in pixel, allora si
  // calcola la scala da questi valori e la dimensione del bitmap.
  Sz = Bmp->getSize();
  if(scaleX < 0) {
    double tX = -scaleX;
    tX /= Sz.cx;
    scaleX = (int)(tX * 1000 + 0.5);
    }
  if(scaleY < 0) {
    double tY = -scaleY;
    tY /= Sz.cy;
    scaleY = (int)(tY * 1000 + 0.5);
    }
  // annulla il flag di trasparenza se non permesso
  switch(Flag) {
    case bTILED:
    case bFILLED:
      Transp = false;
      break;
    case bCENTERED:
    case bSCALED:
    case bUSE_COORD:
      break;
    }
  Pt.x = R__X(x);
  Pt.y = R__Y(y);
  Scale.cx = scaleX;
  Scale.cy = scaleY;
  if(!Scale.cx)
    Scale.cx = 1000;
  if(!Scale.cy)
    Scale.cy = 1000;
  // annulla le dimensioni per far calcolare il tutto quando la finestra sarà pronta
  Sz.cx = 0;
  Sz.cy = 0;

  uint id2 = getFirstExtendId();
  p = getOwner()->getPageString(id2);
  if(p) {
    DWORD tick = _ttoi(p);
    if(tick)
      allocBlink(tick);
    }
  allocVisibility(p);

  if(!Transp)
    Bmp->setQualityOnStretch(true);
  Bmp->addGifControl(getOwner());

  return true;
}
//----------------------------------------------------------------------------
bool PSimpleBmp::update(bool force)
{
  bool chg = checkVisibility(force);
  if(chg) {
    PRect target;
    getCurrRect(target);
    InvalidateRect(*getOwner(), target, 0);
    }
  return chg;
}
//----------------------------------------------------------------------------
void PSimpleBmp::getCurrRect(PRect& target)
{
  if(Transp) {
#if 1
    // per ottimizzare usa lo stesso codice presente nella classe PTraspBitmap
    SIZE sz = Sz;
    double ScaleX = bSCALED != Flag ? R__Xf(Scale.cx / 1000.0) : 1000.0 / Scale.cx;
    double ScaleY = bSCALED != Flag ? R__Yf(Scale.cy / 1000.0) : 1000.0 / Scale.cy;

    if(ScaleX && ScaleX != 1.0)
      sz.cx = (long)(sz.cx * ScaleX);
    if(ScaleY && ScaleY != 1.0)
      sz.cy = (long)(sz.cy * ScaleY);
     target = PRect(Pt.x, Pt.y, sz.cx + Pt.x, sz.cy + Pt.y);
#else
    PTraspBitmap tbmp(getOwner(), Bmp, Pt, false);
    if(bSCALED != Flag)
      tbmp.setScale(R__Xf(Scale.cx / 1000.0), R__Yf(Scale.cy / 1000.0));
    else
      tbmp.setScale(1000.0 / Scale.cx, 1000.0 / Scale.cy);

    target = tbmp.getRect();
#endif
    }
  else {
    if(bTILED != Flag)
      target = PRect(Pt.x, Pt.y, Pt.x + Sz.cx, Pt.y + Sz.cy);

    else
      GetClientRect(*getOwner(), target);
    }
}
//----------------------------------------------------------------------------
void PSimpleBmp::calcRect()
{
  // codice spostato dall'allocazione dell'oggetto (la window non è pronta all'avvio del programma)

  // ripristina
  Sz = Bmp->getSize();
  if(Transp) {
    switch(Flag) {
      case bUSE_COORD:
        break;

      case bCENTERED:
      case bSCALED:
        getCoord(getOwner(), Flag, Bmp, Pt, Scale);
        break;
      }
    }
  else {
    switch(Flag) {
      case bUSE_COORD:
      case bCENTERED:
        getCoord(getOwner(), Flag, Bmp, Pt, Scale);
        Sz.cx = R__X(Sz.cx / 1000.0 * Scale.cx);
        Sz.cy = R__Y(Sz.cy / 1000.0 * Scale.cy);
        break;
      case bSCALED:
        getCoord(getOwner(), Flag, Bmp, Pt, Scale);
        Sz.cx = (long)(Sz.cx * 1000.0 / Scale.cx);
        Sz.cy = (long)(Sz.cy * 1000.0 / Scale.cy);
        break;

      case bFILLED:
        do {
          PRect r;
          GetClientRect(*getOwner(), r);
          Sz.cx = r.Width();
          Sz.cy = r.Height();
          } while(false);
        // fall through
      case bTILED:
        Pt.x = 0;
        Pt.y = 0;
        break;
      }
    if(bTILED == Flag) {
      if(Scale.cx != 1000 && Scale.cx > 10 && Scale.cy != 1000 && Scale.cy > 10) {
        int width = (int)(Sz.cx / 1000.0 * Scale.cx);
        int height = (int)(Sz.cy / 1000.0 * Scale.cy);
        HDC hdc = GetDC(*getOwner());

        HDC memDC1 = CreateCompatibleDC(hdc);
        HBITMAP hBmp1 = CreateCompatibleBitmap(hdc, width , height);
        HGDIOBJ old1 = SelectObject(memDC1, (HGDIOBJ)hBmp1);

        HDC memDC2 = CreateCompatibleDC(hdc);
        HGDIOBJ old2 = SelectObject(memDC2, *Bmp);

        int stretchMode = HALFTONE;
        if(isWin95() || isWin98orLater())
          stretchMode = COLORONCOLOR;

        int oldMode = SetStretchBltMode(memDC1, stretchMode);
        StretchBlt(memDC1, 0, 0, width, height,
                           memDC2, 0, 0, Sz.cx, Sz.cy, SRCCOPY);

        SetStretchBltMode(memDC1, oldMode);

        SelectObject(memDC2, (HGDIOBJ)old2);

        delete Bmp;
        Bmp = new PBitmap(hBmp1, true);

        SelectObject(memDC1, (HGDIOBJ)old1);

        DeleteDC(memDC1);
        DeleteDC(memDC2);
        Sz.cx = width;
        Sz.cy = height;
        ReleaseDC(*getOwner(), hdc);
        }
      }
    }
  setRect(PRect(Pt.x, Pt.y, Pt.x + Sz.cx, Pt.y + Sz.cy));
  if(MoveInfo)
    MoveInfo->setOrig(Pt);
}
//----------------------------------------------------------------------------
void PSimpleBmp::draw(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(!Sz.cx && !Sz.cy)
    calcRect();
  baseSimple::draw(hdc, rect);
}
//----------------------------------------------------------------------------
void PSimpleBmp::drawAtZero(HDC hdc, const PRect& rect)
{
  POINT pt = Pt;
  Pt.x = 0; Pt.y = 0;
  draw(hdc, rect);
  Pt = pt;
}
//----------------------------------------------------------------------------
PRect PSimpleBmp::getTrueRect()
{
  // viene richiamata dal metodo per calcolare l'ingombro nelle modali/modeless
  // quindi nei casi seguenti (in cui il bmp è di sfondo) non deve calcolare già
  // il rettangolo (altrimenti non sarà calcolato al momento giusto e cioè quando
  // la window è dimensionata correttamente)
  switch(Flag) {
    case bCENTERED:
    case bFILLED:
    case bSCALED:
    case bTILED:
      return PRect(10000, 10000, 0, 0);
    };

  if(!Sz.cx && !Sz.cy)
    calcRect();
  PRect r;
  if(Transp) {
    r = PRect(0, 0, ROUND_REAL(Sz.cx * R__Xf(Scale.cx / 1000.0)), ROUND_REAL(Sz.cy * R__Yf(Scale.cy / 1000.0)));
    r.MoveTo(Pt.x, Pt.y);
    }
  else {
    r = PRect(0, 0, Sz.cx, Sz.cy);
    r.MoveTo(Pt.x, Pt.y);
    }
  return r;
}
//----------------------------------------------------------------------------
void PSimpleBmp::performDraw(HDC hdc)
{
  if(Transp) {
    PTraspBitmap tbmp(getOwner(), Bmp, Pt, false);
    if(bSCALED != Flag)
      tbmp.setScale(R__Xf(Scale.cx / 1000.0), R__Yf(Scale.cy / 1000.0));
    else
      tbmp.setScale(1000.0 / Scale.cx, 1000.0 / Scale.cy);

//    tbmp.setCanSaveBkg(false);
    tbmp.Draw(hdc);
    }
  else {
    if(bTILED != Flag)
      Bmp->draw(hdc, Pt, Sz);

    else {
      PRect r;
      GetClientRect(*getOwner(), r);
      Bmp->drawTiled(hdc, r);
      }
    }
}
//----------------------------------------------------------------------------
void PSimpleBmp::performDraw(HDC hdc, const POINT& offset)
{
  POINT old = Pt;
  Pt.x += offset.x;
  Pt.y += offset.y;
  performDraw(hdc);
  Pt = old;
}
//-----------------------------------------------------------
static void getCoord(PWin* win, int flag, PBitmap* bmp, POINT& pt, SIZE& scale)
{
  switch(flag) {
    case bUSE_COORD:
    case bTILED:
    // questo è valido solo per non trasparente, ed è banale il calcolo
    case bFILLED:
      break;

    case bCENTERED:
      do {
        PRect r;
        GetClientRect(*win, r);
        SIZE sz = bmp->getSize();

        sz.cx *= scale.cx;
        sz.cx /= 1000;
        sz.cy *= scale.cy;
        sz.cy /= 1000;

        pt.x = (r.Width() - R__X(sz.cx)) / 2;
        pt.y = (r.Height() - R__Y(sz.cy)) / 2;
        } while(false);
      break;

    case bSCALED:
      do {
        PRect r;
        GetClientRect(*win, r);
        SIZE sz = bmp->getSize();
        double rX = r.Width();
        rX /= sz.cx;
        double rY = r.Height();
        rY /= sz.cy;
        if(rX > rY) {
          scale.cx = sz.cy * 1000 / r.Height();
          scale.cy = scale.cx;
          pt.x = (LONG)((r.Width() - sz.cx * 1000.0 / scale.cx) / 2);
          pt.y = 0;
          }
        else {
          scale.cx = sz.cx * 1000 / r.Width();
          scale.cy = scale.cx;
          pt.y = (LONG)((r.Height() - sz.cy * 1000.0 / scale.cx) / 2);
          pt.x = 0;
          }
        } while(false);
      break;
    }

}
//----------------------------------------------------------------------------
