//----------- test_Addr.h -------------------------------------------------
//----------------------------------------------------------------------------
#ifndef test_Expr_H_
#define test_Expr_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "p_Util.h"
#include "p_file.h"
#include "PBitmap.h"
#include "checkAddr.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class test_Addr : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    test_Addr(PWin * parent, uint id = IDD_CLIENT, HINSTANCE hinst = 0);
    virtual ~test_Addr();

    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hwChild, UINT ctlType);
    void CmOk();
  private:
    HFONT hfEdit;
    HBRUSH Brush1;

    void getPath();
};
//----------------------------------------------------------------------------
#endif

