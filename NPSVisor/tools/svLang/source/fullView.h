//-------- fullView.h -----------------------------------------------
//-------------------------------------------------------------------
#ifndef fullView_H
#define fullView_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "mainwin.h"
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
//-------------------------------------------------------------------
#endif
