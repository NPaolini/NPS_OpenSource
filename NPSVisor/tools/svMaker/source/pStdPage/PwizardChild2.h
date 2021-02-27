//----------- pWizardChild2.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD2_H_
#define PWIZARDCHILD2_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWizardChild.h"
//----------------------------------------------------------------------------
class pWizardChild2 : public pWizardChild
{
  private:
    typedef pWizardChild baseClass;
  public:
    pWizardChild2(PWin* parent, uint resId = IDD_WZ_TREND, HINSTANCE hinstance = 0);
    virtual ~pWizardChild2();

    virtual bool create();
    virtual bool save(setOfString& Set);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getTitle(LPTSTR append, size_t lenBuff);

    void checkEnable();

  private:
};
//----------------------------------------------------------------------------
#endif
