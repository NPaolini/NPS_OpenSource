//--------- DSimpleEditor.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DSimpleEditor_H_
#define DSimpleEditor_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svdialog.h"
#include "svEdit.h"
#include "pOwnBtnImageStd.h"

#include "setPack.h"
//----------------------------------------------------------------------------
class DSimpleEditor : public svDialog
{
  public:
    DSimpleEditor(PWin* parent, LPCTSTR filename, uint resId = IDD_MODAL_BODY, HINSTANCE hinst = 0);
    virtual ~DSimpleEditor();
    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    LPCTSTR Filename;
    svEdit* Text;
    POwnBtnImageStd* OkBtn;
    POwnBtnImageStd* CancBtn;
    bool BNClicked_F1();
    bool BNClicked_F2();
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

