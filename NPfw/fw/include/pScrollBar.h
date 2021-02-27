//-------------------- pScrollBar.h -------------------------
//-----------------------------------------------------------
#ifndef PSCROLLBAR_H_
#define PSCROLLBAR_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PScrollBar : public PControl
{
  private:
    typedef PControl baseClass;
  public:
    PScrollBar(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PScrollBar(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PScrollBar(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PScrollBar() { destroy(); }
  protected:
    virtual LPCTSTR getClassName() const { return _T("scrollbar"); }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PScrollBar::PScrollBar(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, text, hinst)
{
  Attr.style |= SBS_VERT | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PScrollBar::PScrollBar(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, text, hinst)
{
  Attr.style |= SBS_VERT | WS_TABSTOP;
}
//-----------------------------------------------------------
inline PScrollBar::PScrollBar(PWin * parent, uint resid, HINSTANCE hinst) :
        baseClass(parent, resid, hinst)
{ }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
