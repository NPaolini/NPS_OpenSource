//-------------------- POwnBtnStdSwitch.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNBTN_STD_SWITCH_H_
#define POWNBTN_STD_SWITCH_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnImageStd.h"
#include "pmanageBtnSwitch.h"
//-----------------------------------------------------------
/*
  Eredita da POwnBtnImageStd e quindi si disegna allo stesso modo, solo
  che funziona da switch: un click lo setta (premuto), un secondo click
  lo resetta (sollevato)
*/
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnStdSwitch : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
    friend manageBtnSwitch;
  public:
    POwnBtnStdSwitch(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, uint fPos = wLeft, bool autoDelete = false,
          LPCTSTR text = 0, HINSTANCE hinst = 0) : baseClass(parent, id, x, y, w, h, image, fPos, autoDelete, text, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }

    POwnBtnStdSwitch(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, LPCTSTR text = 0, HINSTANCE hinst = 0) : baseClass(parent, id, r, image,
            fPos, autoDelete, text, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }

    POwnBtnStdSwitch(PWin * parent, uint resid, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, HINSTANCE hinst = 0) : baseClass(parent, resid, image, fPos, autoDelete, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }

    POwnBtnStdSwitch(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, bool autoDelete, uint fPos = wLeft,
          LPCTSTR text = 0, HINSTANCE hinst = 0) : baseClass(parent, id, x, y, w, h, image, autoDelete, fPos, text, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }

    POwnBtnStdSwitch(PWin * parent, uint id, const PRect& r, PBitmap* image,
          bool autoDelete, uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0) : baseClass(parent, id, r, image,
            autoDelete, fPos, text, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }

    POwnBtnStdSwitch(PWin * parent, uint resid, PBitmap* image,
          bool autoDelete, uint fPos = wLeft, HINSTANCE hinst = 0) : baseClass(parent, resid, image, autoDelete, fPos, hinst),
          ManagedSwitch(0) {   enableCapture(true); ManagedSwitch = makeManagedSwitch(); }


    ~POwnBtnStdSwitch() { destroy(); delete ManagedSwitch; }

    manageBtnSwitch::state getState() const;
    virtual void setState(manageBtnSwitch::state stat);
    virtual void switchState();

    bool preProcessMsg(MSG& msg);

    void setManageBtnSwitch(manageBtnSwitch* obj) { ManagedSwitch = obj; }
    manageBtnSwitch* getManageBtnSwitch() { return ManagedSwitch; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);
    virtual void drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual bool setPushed(uint msg, uint key, bool previous);

    manageBtnSwitch* ManagedSwitch;
    virtual manageBtnSwitch* makeManagedSwitch() { return new manageBtnSwitch(this); }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
