//----------- pWizardChild.h -------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD_H_
#define PWIZARDCHILD_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"
#include "macro_utils.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
#include "pListbox.h"
#include "p_util.h"
#include "p_txt.h"
#include "svmProperty.h"
//----------------------------------------------------------------------------
#define WM_CUSTOM_WZ (WM_FW_FIRST_FREE + 100)
  #define CM_NEXT_IS_SAVE 1
  #define CM_NEXT_IS_STEP 2
//----------------------------------------------------------------------------
struct id_value {
  uint id;
  LPCTSTR value;
  };
//----------------------------------------------------------------------------
#define DEF_WIZ_PAGEPATH _T("tmp")
//----------------------------------------------------------------------------
#define DEF_WIZ_PAGENAME  DEF_WIZ_PAGEPATH _T("\\wz_stdPage")PAGE_EXT
//----------------------------------------------------------------------------
enum iWizPage { notUsed, pMaintRemoved, pTrend, pRecipeRow, pRecipe, pRecipeLB, pLinkedPage };
//----------------------------------------------------------------------------
class pWizardChild : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    pWizardChild(PWin* parent, uint resId, HINSTANCE hinstance = 0);
    virtual ~pWizardChild();

    virtual bool create();
    virtual bool save(setOfString& set) = 0;
    virtual void setItem(int ix) { Item = ix; }
    int getItem() { return Item; }

    virtual void setStep(int ix) { Step = ix; }
    int getStep() { return Step; }
    virtual int getRemainSteps(int curr) const { return 1 - curr; }
    virtual void setTitle(HWND par, LPCTSTR baseTitle);
  protected:
    wizardDlg* main;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getTitle(LPTSTR append, size_t lenBuff) { append[0] = 0; }
  private:
    int Item;
    int Step;
};
//----------------------------------------------------------------------------
inline
bool isNotEmpty(HWND hw)
{
  return GetWindowTextLength(hw) > 0;
}
//----------------------------------------------------------------------------
enum actionChoose { eacSkip, eacSkipAll, eacApply, eacApplyAll, eacNull, eacMax };
extern actionChoose getActionChoose(PWin* owner, actionChoose curr, LPCTSTR filename);
//----------------------------------------------------------------------------
#endif
