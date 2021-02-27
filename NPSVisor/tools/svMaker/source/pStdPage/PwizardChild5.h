//----------- pWizardChild5.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD5_H_
#define PWIZARDCHILD5_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWizardChild.h"
//----------------------------------------------------------------------------
class pWizardChild5 : public pWizardChild
{
  private:
    typedef pWizardChild baseClass;
  public:
    pWizardChild5(PWin* parent, uint resId = IDD_WZ_LINKED_PAGE, HINSTANCE hinstance = 0);
    virtual ~pWizardChild5();

    virtual bool create();
    virtual bool save(setOfString& Set);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getTitle(LPTSTR append, size_t lenBuff);

    void checkEnable();
    void chooseFile(uint idcEdit);

  private:
};
//----------------------------------------------------------------------------
#endif
