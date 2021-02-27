//-------------------- pToolBar.h ---------------------------
//-----------------------------------------------------------
#ifndef PTOOLBAR_H_
#define PTOOLBAR_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
struct PToolBarInfo
{
  UINT idToolBar;
  TBBUTTON *pButtons;
  UINT nButtons;
  UINT idBmp;
  UINT nRow;
  SIZE szBtn;
  SIZE szBmp;
  int startSelection;
  // se NULL occorre derivare e ridefinire la getTips()
  LPCTSTR* tips;
  // se idBmp == 0 questo deve contenere un handle a bitmap
  // altrimenti è ignorato
  HBITMAP hBitmap;
};
//-----------------------------------------------------------
class PToolBar : public PWin
{
  public:
    PToolBar(PWin * parent, PToolBarInfo* info, HINSTANCE hInstance) :
        PWin(parent, 0, hInstance), Info(info) {}
    virtual ~PToolBar() { destroy(); }
    LPCTSTR getTips(int idCommand);
    void enableBtn(int id, bool enable);
    void selectBtn(int id, bool sel = true);
    void setHotItem(int id);
  protected:
    virtual LPCTSTR getClassName() const;
    virtual HWND performCreate();
    PToolBarInfo* Info;
};
//-----------------------------------------------------------
inline LPCTSTR PToolBar::getClassName() const { return TOOLBARCLASSNAME; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
