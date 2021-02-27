//-------- PVarGraph.h -------------------------------------------------------
#ifndef PVARGRAPH_H_
#define PVARGRAPH_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#ifndef PGRAPH_H_
  #include "pGraph.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarGraph : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarGraph(P_BaseBody* owner, uint id);
    ~PVarGraph();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);
    virtual PRect getRectMove();

    virtual void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual void performDraw(HDC hdc);
    void performDraw(HDC hdc, const POINT& offset);
    virtual uint getBaseId() const { return ID_INIT_BARGRAPH; }

  private:
    PGraph* Graph;
    PGraph* GraphUp;
    PGraph* GraphDown;
    pVariable vMax;
    pVariable vMin;
    PRect baseRect;
    int centerZero;
    REALDATA oldValMax;
    int oldPerc;
    baseVarColor Colors;

    int Truncate;

    int getTruncType() const { return Truncate; }
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

