//------ PVarBtnStd.h --------------------------------------------------------
//-----------------------------------------------------------
#ifndef PVarBtnStd_H_
#define PVarBtnStd_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pmanageBtnSwitch.h"
#include "POwnBtnImageStd.h"
#include "id_Btn.h"
#include "1.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class POwnBtnStdFocus : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
  public:


    POwnBtnStdFocus(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, fPos, false, text, hinst)
        {
          enableCapture(true);
        }

    virtual ~POwnBtnStdFocus() { destroy(); }

  protected:
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
};
//----------------------------------------------------------------------------
class POwnBtnStdSwitchFocus : public POwnBtnStdFocus
{
  private:
    typedef POwnBtnStdFocus baseClass;
  public:


    POwnBtnStdSwitchFocus(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, fPos, text, hinst), ManagedSwitch(0)
        {
          ManagedSwitch = makeManagedSwitch();
        }

    virtual ~POwnBtnStdSwitchFocus() { destroy();  delete ManagedSwitch;  }

    manageBtnSwitch::state getState() const;
    virtual void setState(manageBtnSwitch::state stat);
    virtual void switchState();

    bool preProcessMsg(MSG& msg);

    void setManageBtnSwitch(manageBtnSwitch* obj) { ManagedSwitch = obj; }
    manageBtnSwitch* getManageBtnSwitch() { return ManagedSwitch; }
    bool isPressed() const { return ManagedSwitch ? ManagedSwitch->isPressed() : false; }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);
    virtual void drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual bool setPushed(uint msg, uint key, bool previous);

    manageBtnSwitch* ManagedSwitch;
    virtual manageBtnSwitch* makeManagedSwitch() { return new manageBtnSwitch(this); }
};
//----------------------------------------------------------------------------
class POwnBtnStdSelectFocus : public POwnBtnStdSwitchFocus
{
  private:
    typedef POwnBtnStdSwitchFocus baseClass;
  public:
    POwnBtnStdSelectFocus(cntGroup* group, PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, fPos, text, hinst), Group(group)
        { addToGroup(); }

    virtual ~POwnBtnStdSelectFocus() { destroy(); removeFromGroup(); }

    void setState(manageBtnSwitch::state stat);
    void switchState();
    const cntGroup* getGroup() { return Group; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    cntGroup* Group;
    virtual void unselectOther();
  private:
    void addToGroup();
    void removeFromGroup();
};
//----------------------------------------------------------------------------
class POwnBtnStdPress : public POwnBtnStdSwitchFocus
{
  private:
    typedef POwnBtnStdSwitchFocus baseClass;
  public:
    POwnBtnStdPress(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, fPos, text, hinst)
        {
        }

    ~POwnBtnStdPress() { destroy(); }

    bool preProcessMsg(MSG& msg);
    bool isPressed() const { return ManagedSwitch ? ManagedSwitch->isPressed() : false; }
    void setPressed(bool set)  { if(ManagedSwitch) ManagedSwitch->setPressed(set); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool setPushed(uint msg, uint key, bool previous);

  private:
    void verifyFocus();
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
