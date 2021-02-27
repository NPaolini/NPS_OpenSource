//-------- p_param.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_PARAM_H
#define P_PARAM_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
struct value_name
{
  int value;
  LPTSTR name;
};
//----------------------------------------------------------------------------
class P_Param : public svDialog
{
  public:
    P_Param(struct paramCom& par, PWin* parent, uint resId = IDD_PARAM, HINSTANCE hinstance = 0);
    virtual ~P_Param();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
  private:
    void BNClickedOk();
    paramCom &Par;
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
