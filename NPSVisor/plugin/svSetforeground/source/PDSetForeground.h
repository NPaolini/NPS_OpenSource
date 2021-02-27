//----------- PDSetForeground.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PDSetForeground_H_
#define PDSetForeground_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "POwnBtnImageStd.h"
#include "winList.h"
//----------------------------------------------------------------------------
class PDSetForeground : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDSetForeground(PWin* parent, uint resId, HINSTANCE hinstance = 0);
    virtual ~PDSetForeground();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void fillAppl();
    bool setForeground();
    void CmOk() {}
    POwnBtnImageStd* Btn[2];

    winList wl;
};
//----------------------------------------------------------------------------
#endif
