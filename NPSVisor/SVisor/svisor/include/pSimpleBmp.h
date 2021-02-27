//-------- PSimpleBmp.h ---------------------------------------------------------
#ifndef PSimpleBmp_H_
#define PSimpleBmp_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "PPanel.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PSimpleBmp : public baseSimple
{
  public:
    PSimpleBmp(P_BaseBody* owner, uint id);

    ~PSimpleBmp();

    virtual bool allocObj(LPVOID param = 0);
    virtual void draw(HDC hdc, const PRect& rect);
    virtual bool update(bool force);
    bool isTransp() const { return Transp; }
    virtual PRect getRectMove() { return getTrueRect(); }
    PRect getTrueRect();
    virtual void drawAtZero(HDC hdc, const PRect& rect);
  protected:
    void performDraw(HDC hdc);
    void performDraw(HDC hdc, const POINT& offset);
    void getCurrRect(PRect& target);
    void calcRect();
    virtual uint getBaseId() const { return ID_INIT_BMP; }
  private:
    PBitmap* Bmp;
    POINT Pt;
    SIZE Scale;
    SIZE Sz;
    bool Transp;
    uint Flag;

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

