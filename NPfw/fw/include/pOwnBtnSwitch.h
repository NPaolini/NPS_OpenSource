//-------------------- POwnBtnSwitch.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNBTNSWITCH_H_
#define POWNBTNSWITCH_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnImage.h"
#include "pmanageBtnSwitch.h"
//-----------------------------------------------------------
/*
  Eredita da POwnBtn(Full)Image e quindi si disegna allo stesso modo, solo
  che funziona da switch: un click lo setta (premuto), un secondo click
  lo resetta (sollevato)
*/
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnSwitch : public POwnBtnImage
{
  private:
    typedef POwnBtnImage baseClass;
    friend manageBtnSwitch;
  public:
    POwnBtnSwitch(PWin * parent, uint id, int x, int y, int w, int h,
          const PVect<PBitmap*>& image, bool sizeByImage = true, LPCTSTR text = 0,
          HINSTANCE hinst = 0);
    POwnBtnSwitch(PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0);

    POwnBtnSwitch(PWin * parent, uint resid,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, HINSTANCE hinst = 0);

    ~POwnBtnSwitch() { destroy(); delete ManagedSwitch; }

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
inline
POwnBtnSwitch::POwnBtnSwitch(PWin * parent, uint id, int x, int y, int w, int h,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, image, sizeByImage, text, hinst),
      ManagedSwitch(0)
{
  enableCapture(true);
  ManagedSwitch = makeManagedSwitch();
}
//-----------------------------------------------------------
inline
POwnBtnSwitch::POwnBtnSwitch(PWin * parent, uint id, const PRect& r,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, image, sizeByImage, text, hinst),
      ManagedSwitch(0)
{
  enableCapture(true);
  ManagedSwitch = makeManagedSwitch();
}
//-----------------------------------------------------------
inline POwnBtnSwitch::POwnBtnSwitch(PWin * parent, uint resid,
            const PVect<PBitmap*>& image, bool sizeByImage, HINSTANCE hinst) :
        baseClass(parent, resid, image, sizeByImage, hinst),
      ManagedSwitch(0)
{
  enableCapture(true);
  ManagedSwitch = makeManagedSwitch();
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
