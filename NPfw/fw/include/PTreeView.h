//-------------------- PTreeView.h --------------------------
//-----------------------------------------------------------
#ifndef PTREEVIEW_H_
#define PTREEVIEW_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PTreeView : public PControl
{
  public:
    PTreeView(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PTreeView(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PTreeView(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PTreeView() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PTreeView::PTreeView(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PTreeView::PTreeView(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.style |= TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PTreeView::PTreeView(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PTreeView::getClassName() const { return WC_TREEVIEW; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif