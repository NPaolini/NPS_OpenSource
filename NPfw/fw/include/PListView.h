//-------------------- PListView.h --------------------------
//-----------------------------------------------------------
#ifndef PLISTVIEW_H_
#define PLISTVIEW_H_
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
class IMPORT_EXPORT PListView : public PControl
{
  public:
    PListView(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PListView(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PListView(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PListView() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PListView::PListView(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= LVS_NOSORTHEADER | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;
}
//-----------------------------------------------------------
inline PListView::PListView(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.style |= LVS_NOSORTHEADER | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;
}
//-----------------------------------------------------------
inline PListView::PListView(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PListView::getClassName() const { return WC_LISTVIEW; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
