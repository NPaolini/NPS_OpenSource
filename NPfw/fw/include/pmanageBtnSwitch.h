//-------------------- pmanageBtnSwitch.h ---------------------------
//-----------------------------------------------------------
#ifndef pmanageBtnSwitch_H_
#define pmanageBtnSwitch_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtn.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class cntGroup
{
  public:
    cntGroup() : lastSel(0) {}
    PVect<POwnBtn*> Group;
    POwnBtn* lastSel;
};
//-----------------------------------------------------------
class manageBtnSwitch
{
  public:
    enum state { OFF, ON };

    manageBtnSwitch(POwnBtn* owner) : Owner(owner), Status(OFF), pressed(false) {}
    virtual ~manageBtnSwitch() {}

    virtual state getState() const { return Status; }
    virtual void setState(state stat);
    virtual void switchState();

    virtual bool preProcessMsg(MSG& msg);

    virtual bool windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& res);
    virtual bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);
    virtual void drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual bool setPushed(uint msg, uint key, bool previous);
    virtual bool isPressed() const { return pressed; }
    virtual void setPressed(bool set) { pressed = set; }

  protected:
    bool pressed;
    state Status;
    POwnBtn* Owner;
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
