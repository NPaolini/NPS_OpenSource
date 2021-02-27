//-------- PVarSpin.h --------------------------------------------------------
#ifndef PVarSpin_H_
#define PVarSpin_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarSpin : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarSpin(P_BaseBody* owner, uint id);
    ~PVarSpin() {  }

    virtual bool allocObj(LPVOID param = 0);

    virtual bool update(bool force);
    void action(int up);
    uint getIdSpin() const { return idSpin; }
  protected:
    virtual void performDraw(HDC hdc) {  }
    virtual uint getBaseId() const { return ID_INIT_VAR_SPIN_UPDN; }
  private:
    DWORD lastTime;
    pVariable memVar;
    enum state { idleSpin, waitTimeout, waitChange };
    state Stat;
    union U {
      int iv;
      float fv;
      };
    U StepUp;
    U StepDn;
    bool integerStep;
    DWORD timeBeforeSend;
    uint idSpin;
    bool firstStart;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

