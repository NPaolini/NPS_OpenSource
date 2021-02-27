//-------- PVarXMeter.h -----------------------------------------------------
#ifndef PVarXMeter_H_
#define PVarXMeter_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
class P_BaseBody;
class PWinActive;
//----------------------------------------------------------------------------
class PVarXMeter : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarXMeter(P_BaseBody* owner, uint id);
    virtual ~PVarXMeter();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_X_METER; }
  private:
    // by param
    int Granularity;
    int InitAngle;
    int Range;
    POINT offsImage;
    int idImage;
    bool ClockWise;
    uint Id;
    PRect ImRect;

    bool offRange;
    DWORD Toggle;

    // calculate
    int Angle;
    PBitmap* Image;
    void calcRectBase();
    void drawImageRotate(HDC hdc);

    PWinActive* Active;
    friend class PWinActive;
    pVariable vMin;
    pVariable vMax;
    double valMin;
    double valMax;
    double valCurr;
    bool canUpdate;
    void setAngle(REALDATA v_min, REALDATA v_max, REALDATA vCurr);
    void setCurrAngle();
    void sendCurrData();
    bool Center;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

