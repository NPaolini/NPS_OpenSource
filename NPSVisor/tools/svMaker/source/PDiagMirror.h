//------- PDiagMirror.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PDiagMirror_H_
#define PDiagMirror_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include "pModDialog.h"
#include "resource.h"
#include "macro_utils.h"
#include "svmObject.h"
#include "1.h"
//----------------------------------------------------------------------------
class PDiagMirror : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDiagMirror(PWin* parent, bool& all, uint& choose, bool hasSelected, uint resId = IDD_MIRROR, HINSTANCE hinstance = 0);
    virtual ~PDiagMirror();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();

  private:
    bool& All;
    uint& Choose;
    bool hasSelected;
};
//----------------------------------------------------------------------------
#endif
