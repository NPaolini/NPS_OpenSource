//----------- pWizardChild3.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD3_H_
#define PWIZARDCHILD3_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWizardChild.h"
//----------------------------------------------------------------------------
class pWizardChild3 : public pWizardChild
{
  private:
    typedef pWizardChild baseClass;
  public:
    pWizardChild3(PWin* parent, uint resId = IDD_WZ_RECIPE_ROW, HINSTANCE hinstance = 0);
    virtual ~pWizardChild3();

    virtual bool create();
    virtual bool save(setOfString& Set);
    int getRemainSteps(int curr) const { return 2 - curr; }
    virtual void setStep(int ix);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getTitle(LPTSTR append, size_t lenBuff);
  private:
    void findIndicator();
    class wzChild* child[2];
    class recipeRowInfo* RRI;
};
//----------------------------------------------------------------------------
#endif
