//------- PDiagOffset.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PDiagOffset_H_
#define PDiagOffset_H_
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
class PDiagOffset : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDiagOffset(PWin* parent, infoOffsetReplace& ior, uint resId = IDD_ADDR_OFFSET, HINSTANCE hinstance = 0);
    virtual ~PDiagOffset();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void enableAddr();
    void enablePrph();
    void enableSimpleText();
    void enableVar();

  private:
    infoOffsetReplace& i_o_r;
};
//----------------------------------------------------------------------------
#endif
