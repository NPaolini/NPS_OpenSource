//----------- chooseExport.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef chooseExport_H_
#define chooseExport_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#include "pModDialog.h"
#include "p_util.h"
#include "pListView.h"
//----------------------------------------------------------------------------
class PD_chooseExport : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_chooseExport(PWin* parent, PVect<LPCTSTR>& filelist, uint resId = IDD_DIALOG_CHOOSE_EXPORT, HINSTANCE hinstance = 0);
    virtual ~PD_chooseExport();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
  private:
    PListView* lbAllDisp;
    PListView* lbToExport;
    PVect<LPCTSTR>& filelist;

    void addAll(PListView* target, PListView* source);
    void addLB(PListView* target, PListView* source);
    void perform();
    void fill();
    int beginDrag;
    bool onRun;
    void enableWindow(bool enable);
};
//----------------------------------------------------------------------------
#endif
