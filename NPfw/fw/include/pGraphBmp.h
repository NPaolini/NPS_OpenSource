//-------------------- pGraphBmp.H --------------------------
#ifndef PGRAPHBMP_H_
#define PGRAPHBMP_H_
#include "precHeader.h"
//-----------------------------------------------------------
#include "PGraph.h"
#include "PBitmap.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PGraphBmp : public PGraph
{
  private:
    typedef PGraph baseClass;
  public:
    enum bmpstyle { bmpStretch, bmpTiled, bmpRotate, bmpMirror = 4 };
    PGraphBmp(const PRect& rect, COLORREF colorBkg, bStyle type,
            COLORREF colorFrg, PBitmap* bar, bmpstyle barStyle,
            PBitmap* bkg, bmpstyle bkgStyle, bool horz = true, bool to_right = true);
    virtual ~PGraphBmp();
    virtual void draw(HDC hdc);

    virtual void resize(const PRect& newRect);
  protected:
    PBitmap* makeBmp(PBitmap* bmp, DWORD style);

  private:
    PBitmap* BarBmp;
    PBitmap* BkgBmp;
    PBitmap* BarBmpOrig;
    PBitmap* BkgBmpOrig;
    DWORD BarStyle;
    DWORD BkgStyle;
    PBitmap* resizeBmp(PBitmap* bmp, const PRect& r);

};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
