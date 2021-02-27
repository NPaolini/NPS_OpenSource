//------------------ svmMainClient.h ------------------------
//-----------------------------------------------------------
#ifndef SVMMAINCLIENT_H_
#define SVMMAINCLIENT_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "headerMsg.h"
#include <stdlib.h>
#include <commctrl.h>
//-----------------------------------------------------------
#include "PscrollContainer.h"
#define ID_CLIENT_SCROLL 110
//-----------------------------------------------------------
#define FIRST_CUSTOM_ID 1000000
//-----------------------------------------------------------
struct menuPopup
{
  uint flag;
  uint id;
  LPCTSTR text;
};
//-----------------------------------------------------------
PRect lgToDp(PRect r);
int popupMenu(HWND hwnd, menuPopup* item, int num, POINT* pt = 0);
//-----------------------------------------------------------
#define WM_SEND_DIM   (WM_FW_FIRST_FREE + 4)
#define WM_SEND_PAN   (WM_SEND_DIM + 1)
#define WM_HAS_SCROLL (WM_SEND_PAN + 1)
#define WM_SEND_SCROLL_POS (WM_HAS_SCROLL + 1)
//-----------------------------------------------------------
class childScrollContainer : public PscrollContainer
{
  private:
    typedef PscrollContainer baseClass;
  public:
    childScrollContainer(PWin * parent, uint id, HINSTANCE hInstance);
    virtual ~childScrollContainer();

    virtual bool create();

    void setScrollPos(POINT* pt);

  protected:
    virtual LPCTSTR getClassName() const { return _T("svmContainer_ClassName"); }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual PclientScrollWin* makeClient();

    void resize_test();
    void manageDim();
    void managePan(POINT* delta);
    void evMouseWheel(short delta, short x, short y);
  private:
};
//-----------------------------------------------------------
#endif
