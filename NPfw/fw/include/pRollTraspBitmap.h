//-------------------- pRollTraspBitmap.h ---------------------------
//-----------------------------------------------------------
#ifndef PROLLTRASPBITMAP_H_
#define PROLLTRASPBITMAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PTRASPBITMAP_H_
  #include "pTraspBitmap.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
// esegue un rolling del bitmap, valori positivi di x lo fanno muovere
// verso sinistra, valori positivi di y lo mandano verso l'alto
class IMPORT_EXPORT PRollTraspBitmap : public PTraspBitmap
{
  public:
    PRollTraspBitmap(PWin* parent, PBitmap *bmp, const POINT& pt,
            const POINT& step, bool autodelete = false);

    virtual void Draw(HDC hdc);
    void next();

    const POINT& getCurr() const;
    const POINT& getStep() const;

    void setCurr(const POINT& pt);
    void setStep(const POINT& step);
  private:
    POINT ptCurrent;
    POINT Step;
    Gdiplus::Color TranspPix;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline const POINT& PRollTraspBitmap::getCurr() const
{
  return ptCurrent;
}
//-----------------------------------------------------------
inline const POINT& PRollTraspBitmap::getStep() const
{
  return Step;
}
//-----------------------------------------------------------
inline void PRollTraspBitmap::setCurr(const POINT& pt)
{
  ptCurrent = pt;
}
//-----------------------------------------------------------
inline void PRollTraspBitmap::setStep(const POINT& step)
{
  Step = step;
}
//-----------------------------------------------------------
inline void PRollTraspBitmap::next()
{
  ptCurrent.x -= Step.x;
  ptCurrent.y += Step.y;
  PTraspBitmapBase::Invalidate();
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
