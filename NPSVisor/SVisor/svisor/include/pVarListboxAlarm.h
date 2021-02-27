//-------- pVarListBoxAlarm.h -----------------------------------------------------
#ifndef pVarListBoxAlarm_H_
#define pVarListBoxAlarm_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "pBaseListAlarm.h"
#include "PListBox.h"
#include "setPack.h"
//----------------------------------------------------------------------------
class pVarListBoxAlarm : public pBaseListAlarm, public PListBox
{
  private:
    typedef pBaseListAlarm base_Active;
    typedef PListBox baseClass;
  public:
    pVarListBoxAlarm(P_BaseBody* owner, uint id, pAround& Around);
    virtual ~pVarListBoxAlarm();
    virtual bool create();

    virtual int getCount();
    virtual int getCurrSel();
    virtual void setCurrSel(int sel);
    virtual void removeAllItem();

    virtual void formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat);
    virtual void formatHeaderAlarm(uint idHeadTitle);

    virtual HWND getHwnd() { return getHandle(); }

    virtual void calcWidth();

  protected:
    int tWidth[MAX_SHOW_AL];
    int totElem;
    int verifyKey();
    void handleLBDblClick();
    virtual void performDraw(HDC hdc) {};
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool evKeyDown(WPARAM& key);
    virtual SIZE getSizeFont() { return baseClass::getSizeFont(); }

    enum colorRef { crBase, crEvent, crActive, crAcknoledged, crAcknoledgedEvent };
    virtual const pBaseListAlarm::infoColorLB* getColor(colorRef ix);
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    virtual bool evChar(WPARAM& key);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

