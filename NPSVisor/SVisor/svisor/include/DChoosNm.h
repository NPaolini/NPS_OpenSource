//--------- dchoosnm.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DCHOOSNM_H_
#define DCHOOSNM_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif

#ifndef PSTATIC_H_
  #include "pstatic.h"
#endif

#include "svEdit.h"

#ifndef PBUTTON_H_
  #include "pbutton.h"
#endif

#include "setPack.h"
//----------------------------------------------------------------------------
class TD_ChooseNum : public svDialog
{
  public:
    TD_ChooseNum(PWin* parent, double &init, double& end, bool useInfoMax = true, uint resId = IDD_CHOOSE_NUM, HINSTANCE hinst = 0);
    virtual ~TD_ChooseNum();
    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    double& Init;
    double& End;
    svEdit* a_;
    svEdit* da_;
    PStatic* txtA;
    PStatic* txtDa;
    PStatic* infoMax;
    PStatic* txtMax;
    PButton* OkBtn;
    PButton* CancBtn;
    bool useInfoMax;

    bool BNClicked_F1();
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

