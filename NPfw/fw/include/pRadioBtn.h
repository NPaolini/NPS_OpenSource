//-------------------- PRadioBtn.h ---------------------------
//-----------------------------------------------------------
#ifndef PRADIOBTN_H_
#define PRADIOBTN_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PBUTTON_H_
  #include "pButton.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PRadioButton : public PButton
{
  public:
    PRadioButton(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PRadioButton(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PRadioButton(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PRadioButton() { destroy(); }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PRadioButton::PRadioButton(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PButton(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= BS_AUTORADIOBUTTON | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PRadioButton::PRadioButton(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PButton(parent, id, r, text, hinst)
{
  Attr.style |= BS_AUTORADIOBUTTON | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PRadioButton::PRadioButton(PWin * parent, uint resid, HINSTANCE hinst) :
        PButton(parent, resid, hinst)
{ }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
