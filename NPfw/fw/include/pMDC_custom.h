//------------------ pMDC_custom.h ------------------------
//-----------------------------------------------------------
#ifndef pMDC_custom_H_
#define pMDC_custom_H_
//-----------------------------------------------------------
#include "precHeader.h"
#define BKG_COLOR_MDC_CUSTOM RGB(255, 255, 240)
//-----------------------------------------------------------
class pMDC_custom
{
  public:
    pMDC_custom() : mdc(0), hBmpTmp(0),
      bkg(0), cBkg(BKG_COLOR_MDC_CUSTOM),
      pen((HPEN)GetStockObject(BLACK_PEN)),
      oldMode(0), oldBkg(0), oldColor(0), oldAlign(0), oldPen(0)
      {}
    ~pMDC_custom() { clear(); }

    void clear();
    HDC getMdc(PWin* owner, HDC hdc);

    void setBkg(COLORREF c) { cBkg = c; }
  private:
    PWin* Owner;
    HDC mdc;
    HBITMAP hBmpTmp;
    HGDIOBJ oldBmp;

    HBRUSH bkg;
    uint oldBkg;
    COLORREF cBkg;

    HPEN pen;
    HGDIOBJ oldPen;

    uint oldAlign;
    uint oldColor;
    uint oldMode;
};
//-----------------------------------------------------------
#endif
