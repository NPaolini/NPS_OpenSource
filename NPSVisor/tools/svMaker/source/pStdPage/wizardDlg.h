//----------- wizardDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef WIZARDDLG_H_
#define WIZARDDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif
#include "p_util.h"
#include "common.h"
//----------------------------------------------------------------------------
#define MAX_WIZARD_PAGE 5
//----------------------------------------------------------------------------
class wizardDlg : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    wizardDlg(PWin* parent, setOfString& set, uint resId = IDD_WZ_MAIN, HINSTANCE hinstance = 0);
    virtual ~wizardDlg();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    class pWizardChild* wzChild[MAX_WIZARD_PAGE];
    setOfString& Set;
    uint currPage;
    int currStep;
    void nextStep(bool next = true);
    bool save();
    void showFirst();
    void showNext();

};
//----------------------------------------------------------------------------
#endif
