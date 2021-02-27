//-------------------- pCheckBox.h ---------------------------
//-----------------------------------------------------------
#ifndef PCHECKBOX_H_
#define PCHECKBOX_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PBUTTON_H_
  #include "pButton.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PCheckBox : public PButton
{
  public:
    PCheckBox(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PCheckBox(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PCheckBox(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PCheckBox() { destroy(); }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PCheckBox::PCheckBox(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PButton(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= BS_AUTOCHECKBOX | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PCheckBox::PCheckBox(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PButton(parent, id, r, text, hinst)
{
  Attr.style |= BS_AUTOCHECKBOX | BS_CENTER | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PCheckBox::PCheckBox(PWin * parent, uint resid, HINSTANCE hinst) :
        PButton(parent, resid, hinst)
{ }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
