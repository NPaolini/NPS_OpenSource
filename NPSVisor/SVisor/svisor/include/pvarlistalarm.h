//-------- pVarListAlarm.h -----------------------------------------------------
#ifndef pVarListAlarm_H_
#define pVarListAlarm_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
#include "pVarListView.h"
#include "currAlrm.h"
//----------------------------------------------------------------------------
struct fullInfoAlarm;
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class pBaseListAlarm;
//----------------------------------------------------------------------------
class pVarListAlarm : public baseActive
{
  private:
    typedef baseActive baseClass;
  public:
    pVarListAlarm(P_BaseBody* owner, uint id);
    virtual ~pVarListAlarm();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    bool create();

    virtual void draw(HDC hdc, const PRect& rect) {}
    virtual void setVisibility(bool set);
    bool setFocus(pAround::around where);
    operator HWND() { return getHwnd(); }

    virtual void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual void performDraw(HDC hdc) {};
    virtual HWND getHwnd();
    pBaseListAlarm* listObj;
    virtual uint getBaseId() const { return ID_INIT_VAR_ALARM; }
    uint idObj;
    pVariable vFilter;

    typedef unsigned __int64 tFilter;

    tFilter filterFix;
    tFilter currFilter;
    pVariable vFilterGroup;
    DWORD currFilterGroup;

    pVariable vFilterTxt;

    static uint idHeadTitle;
    bool setBaseData(const prfData& data);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

