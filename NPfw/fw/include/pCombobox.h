//-------------------- pCombobox.h ---------------------------
//-----------------------------------------------------------
#ifndef PCOMBOBOX_H_
#define PCOMBOBOX_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PComboBox : public PControl
{
  public:
    PComboBox(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PComboBox(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PComboBox(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PComboBox() { destroy(); }
  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PComboBox::PComboBox(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PComboBox::PComboBox(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.style |= CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PComboBox::PComboBox(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PComboBox::getClassName() const { return _T("combobox"); }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
