//---------- password.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PASSWORD_H_
#define PASSWORD_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pmoddialog.h"
#include "resource.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//--------------------------------------------------------------
// livello usato quando necessita solo di una conferma
#define ONLY_CONFIRM 1000
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_ChgPassword : public PModDialog
{
  public:
    P_ChgPassword(PWin* parent, uint resId = IDD_CHG_PSW, HINSTANCE hinst = 0)  :
      PModDialog(parent, resId, hinst) {  }
    virtual bool create();
    virtual void CmOk();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void showMsg(int lev);
    void setEnableEdit(int ix);
};
//----------------------------------------------------------------------------

#include "restorePack.h"

#endif


