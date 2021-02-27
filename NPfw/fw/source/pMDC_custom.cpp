//------------------ pMDC_custom.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pMDC_custom.h"
//-----------------------------------------------------------
void pMDC_custom::clear()
{
  if(mdc) {
    SetBkMode(mdc, oldMode);
    SetBkColor(mdc, oldBkg);
    SetTextColor(mdc, oldColor);
    SetTextAlign(mdc, oldAlign);
    SelectObject(mdc, oldPen);
    SelectObject(mdc, oldBmp);
    SetGraphicsMode(mdc, GM_COMPATIBLE);
    DeleteDC(mdc);
    DeleteObject(hBmpTmp);
    DeleteObject(bkg);
    mdc = 0;
    }
}
//-----------------------------------------------------------
HDC pMDC_custom::getMdc(PWin* owner, HDC hdc)
{
  PRect r;
  GetClientRect(*owner, r);
  if(!mdc && r.Width()) {
    mdc = CreateCompatibleDC(hdc);
    SetGraphicsMode(mdc, GM_ADVANCED);
    hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
    oldBmp = SelectObject(mdc, hBmpTmp);
    oldPen = SelectObject(mdc, pen);
    oldAlign = SetTextAlign(mdc, TA_BOTTOM | TA_CENTER);
    oldColor = SetTextColor(mdc, RGB(0, 0, 220));
    if((DWORD)-1 == cBkg) {
      oldBkg = SetBkColor(mdc, 0);
      oldMode = SetBkMode(mdc, TRANSPARENT);
      bkg = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
      }
    else {
      oldBkg = SetBkColor(mdc, cBkg);
      oldMode = SetBkMode(mdc, OPAQUE);
      bkg = CreateSolidBrush(cBkg);
      }
    }
  if(mdc)
    FillRect(mdc, r, bkg);
  return mdc;
}
//-----------------------------------------------------------
