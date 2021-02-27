//-------------------- pButton.h ---------------------------
//-----------------------------------------------------------
#ifndef PBUTTON_H_
#define PBUTTON_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PButton : public PControl
{
  public:
    PButton(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PButton(PWin * parent, uint id, const PRect& r, LPCTSTR text = 0, HINSTANCE hinst = 0);

    PButton(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PButton() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PButton::PButton(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= BS_PUSHBUTTON | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PButton::PButton(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.style |= BS_PUSHBUTTON | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PButton::PButton(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PButton::getClassName() const { return _T("button"); }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
