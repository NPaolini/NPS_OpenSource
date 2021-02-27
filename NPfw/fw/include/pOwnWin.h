//-------------------- pOwnWin.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNWIN_H_
#define POWNWIN_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT POwnWin : public PControl
{
  public:
    POwnWin(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnWin(PWin * parent, uint id, const PRect& r, LPCTSTR text = 0, HINSTANCE hinst = 0);

    // Se il controllo è scritto in un file di risorse, e non è ownerDraw,
    // ne memorizza le coordinate, lo distrugge e lo ricrea al volo
    // è necessario passare lo stile di ownerDraw perché non è lo stesso
    // nei vari controlli
    POwnWin(PWin * parent, uint resid, uint style, HINSTANCE hinst = 0);

    ~POwnWin() { destroy(); }

    virtual bool create();

    void invalidate();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual bool EvPaint(HDC hdc, LPPAINTSTRUCT ps);
    virtual bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);
    virtual bool measureItem(uint idCtrl, LPMEASUREITEMSTRUCT mis);
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual bool isOwnedDraw() = 0;

//    virtual void makeStruct(HDC hdc, LPDRAWITEMSTRUCT dis, LPPAINTSTRUCT ps);

    // utilizzata quando il controllo proviene da un file di risorse
    virtual void setCoord(PRect& /*rect*/) { }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
POwnWin::POwnWin(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
//  enableCapture(true);
}
//-----------------------------------------------------------
inline
POwnWin::POwnWin(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
//  enableCapture(true);
}
//-----------------------------------------------------------
inline
POwnWin::POwnWin(PWin * parent, uint resid, uint style, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{
  Attr.style = WS_VISIBLE | WS_TABSTOP | WS_CHILD | style;
//  enableCapture(true);
}
//-----------------------------------------------------------
inline
bool POwnWin::drawItem(uint /*idCtrl*/, LPDRAWITEMSTRUCT /*dis*/)
{
  return false;
}
//-----------------------------------------------------------
inline
bool POwnWin::measureItem(UINT /*idCtrl*/, LPMEASUREITEMSTRUCT /*mis*/)
{
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
