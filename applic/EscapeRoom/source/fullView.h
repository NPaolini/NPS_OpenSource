//-------- fullView.h -----------------------------------------------
//-------------------------------------------------------------------
#ifndef fullView_H
#define fullView_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "mainwin.h"
#include "p_file.h"
#include "PSplitWin.h"
#include "pMDC_custom.h"
#include "pBitmap.h"
#include "p_vect.h"
#include "p_util.h"
//----------------------------------------------------------------------------
struct menuInfo
{
  LPCTSTR item;
  bool grayed;
};
extern int getResultMenu(PWin* owner, const menuInfo* mInfo, uint nElem);
//-------------------------------------------------------------------
class fullView : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    fullView(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst) { init(); }

    fullView(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst) { init(); }

    fullView(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst) { init(); }

    virtual ~fullView() { destroy(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void resize();
    virtual HWND getHWClient() { return 0; }

    void init();
};
//-----------------------------------------------------------
class myPSplitWin : public PSplitWin
{
  private:
    typedef PSplitWin baseClass;
  public:
    myPSplitWin(PWin* parent, uint id, uint perc = 500, uint width = 3, uint style = PSplitter::PSW_HORZ, HINSTANCE hInst = 0) :
      baseClass(parent, id, perc, width, style, hInst) { }

    virtual ~myPSplitWin();

};
//-------------------------------------------------------------------
void makeTrueFullPath(LPTSTR path, uint dim);
//-------------------------------------------------------------------
#endif
