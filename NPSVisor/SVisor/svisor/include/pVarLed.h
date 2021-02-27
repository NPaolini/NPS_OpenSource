//-------- PVarLed.h ---------------------------------------------------------
#ifndef PVARLED_H_
#define PVARLED_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
#include "pSeqTraspBitmap.h"
#include "pPanel.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
#define MAX_NUM_LED 11
//----------------------------------------------------------------------------
class pLed : public PSeqTraspBitmap
{
  public:
    pLed(PWin* owner, PBitmap **bmp, uint nBmp, const POINT& pt);
    // se il numero eccede MAX_NUM_LED, nasconde
    void setNum(int n);
};
//----------------------------------------------------------------------------
class PVarLed : public baseVar
{
  public:

    PVarLed(P_BaseBody* owner, uint id);

    ~PVarLed();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);


  protected:
    void performDraw(HDC hdc, const POINT& offset);
    virtual void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_LED; }
  private:
    PBitmap** aBmp;
    pLed** Led;
    PTraspBitmap* pointBmp;

    PPanel* Bkg;
    int nDigit;
    int nDec;
    bool showZero;

    void showEmpty();

    void Set(long val);
    void Set(REALDATA val);

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

