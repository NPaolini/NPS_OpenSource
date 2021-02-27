//-------- pVarListViewAlarm.h -----------------------------------------------------
#ifndef pVarListViewAlarm_H_
#define pVarListViewAlarm_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "pBaseListAlarm.h"
#include "setPack.h"

//----------------------------------------------------------------------------
class pVarListViewAlarm : public pBaseListAlarm, public PCustomListView
{
  private:
    typedef pBaseListAlarm base_Active;
    typedef PCustomListView baseClass;
  public:
    pVarListViewAlarm(P_BaseBody* owner, uint id, pAround& Around);
    virtual ~pVarListViewAlarm();
    virtual bool create();

    virtual int getCount();
    virtual int getCurrSel();
    virtual void setCurrSel(int sel);
    virtual void removeAllItem();

    virtual void formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat);
    virtual void formatHeaderAlarm(uint idHeadTitle);

    virtual bool ProcessNotify(LPARAM lParam, LRESULT& result);
    virtual bool getColorBkg(COLORREF& c);
    virtual bool getColorTextBkg(COLORREF& c);
    virtual bool getColorText(COLORREF& c);
    virtual bool getColorTextBkgSel(COLORREF& c);
    virtual bool getColorTextSel(COLORREF& c);

    virtual HWND getHwnd() { return getHandle(); }

  protected:
    int verifyKey();
    void handleLBDblClick();
    virtual void performDraw(HDC hdc) {};
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool evKeyDown(UINT& key);
    virtual SIZE getSizeFont() { return baseClass::getSizeFont(); }
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

