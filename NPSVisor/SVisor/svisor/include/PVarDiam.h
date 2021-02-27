//-------- PVarDiam.h -------------------------------------------------------
#ifndef PVARDIAM_H_
#define PVARDIAM_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
#ifndef PPANEL_H_
  #include "ppanel.h"
#endif
//----------------------------------------------------------------------------
#define MAX_TYPE_COLOR 4

#include "setPack.h"
//----------------------------------------------------------------------------
class PVarDiam : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarDiam(P_BaseBody* owner, uint id);
    ~PVarDiam();

    enum colorType {
        cCROWN,
        cPIVOT,
        cAXIS,
        cBKG,
        MAX_COLOR_TYPE
        };

    enum diamStyle {
        ONLY_DIAM,
        AXIS = 1,
        TRANSP_PIVOT = 1 << 1,
        DRAW_MAX = 1 << 2,
        INVERT = 1 << 3,
        };

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_DIAM; }

    virtual void drawClipped(HDC hdc, REALDATA* vals);

    virtual void drawFilled(HDC hdc, REALDATA* vals);
    // disegna il diametro massimo (o minimo se INVERT)
    virtual void drawMaxVal(HDC hdc);
    virtual void drawAxis(HDC hdc);

    enum { idMin, idMax, idCurr, idColor, MAX_DATA_DIAM };

    struct colors {
      COLORREF type[MAX_COLOR_TYPE];
      colors() { memset(this, 0, sizeof(*this)); }
      };

    bool loadVal(REALDATA* vals);

    const PVect<colors>& getColors() const { return Colors; }
    DWORD getType() { return Type; }

    PRect minRect;
    void setColors();
    PRect getCurrRect(REALDATA* vals, PRect& rMax);
    void loadColors(LPCTSTR p, colors& c);

  private:
    pVariable vMin;
    pVariable vMax;
    pVariable vColor;

    PPanel* Panel;

    DWORD Type;

    PVect<colors> Colors;
    uint currColor;

    bool isChanged();

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
