//----------- pWizardChild1.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD1_H_
#define PWIZARDCHILD1_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWizardChild.h"
//----------------------------------------------------------------------------
class pWizardChild1 : public pWizardChild
{
  private:
    typedef pWizardChild baseClass;
  public:
    pWizardChild1(PWin* parent, uint resId = IDD_WZ_FIRST_PAGE, HINSTANCE hinstance = 0);
    virtual ~pWizardChild1();

    virtual bool create();
    virtual bool save(setOfString& set);
    int getChoice();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
};
//----------------------------------------------------------------------------
#endif
