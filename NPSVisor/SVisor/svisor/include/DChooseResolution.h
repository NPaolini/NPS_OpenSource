//------- DChooseResolution.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DChooseResolution_H_
#define DChooseResolution_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svdialog.h"
#include "svEdit.h"
#include "pStatic.h"
#include "resource.h"
#include "config.h"
#include "set_custom_cfg.h"
//----------------------------------------------------------------------------
class DChooseResolution : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    DChooseResolution(PWin* parent, bool& changed, uint resId = IDD_DIALOG_CHOOSE_RESOLUTION, HINSTANCE hinstance = 0);
    virtual ~DChooseResolution();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void BNClicked_F1();
    void BNClicked_F2();
    void CmOk() {}
    void CmCancel() {}
    bool& Changed;

  private:
    void checkValidWidth();
    void checkValidHeight();
};
//----------------------------------------------------------------------------
#endif

