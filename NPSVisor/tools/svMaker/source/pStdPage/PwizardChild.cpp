//----------- pWizardChild.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//#include <Lmwksta.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild.h"
#include "common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pWizardChild::pWizardChild(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),
    main(dynamic_cast<wizardDlg*>(parent))
{

}
//----------------------------------------------------------------------------
pWizardChild::~pWizardChild()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pWizardChild::create()
{
  if(!baseClass::create())
    return false;

  return true;
}
//----------------------------------------------------------------------------
LRESULT pWizardChild::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
/*
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_PREV:
          nextStep(false);
          break;

        case IDC_NEXT:
          nextStep(true);
          break;


//    case WM_ENTERIDLE:
//      checkCode();
//      break;

    }
*/
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pWizardChild::setTitle(HWND par, LPCTSTR baseTitle)
{
  TCHAR append[256];
  getTitle(append, SIZE_A(append));
  if(*append) {
    TCHAR title[512];
    wsprintf(title, _T("%s - %s"), baseTitle, append);
    SetWindowText(par, title);
    }
  else
    SetWindowText(par, baseTitle);
}
//----------------------------------------------------------------------------
class PDlg_Action : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDlg_Action(PWin* parent, uint& action, LPCTSTR txt,  uint id = IDD_DIALOG_ACTION, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), retAction(action), Txt(txt) {  }
    ~PDlg_Action() { destroy(); }

    virtual bool create();
  protected:
    static uint Action;
    uint& retAction;
    LPCTSTR Txt;
    void CmOk();
    void CmCancel() {}
    static POINT pt;
};
//-----------------------------------------------------------
POINT PDlg_Action::pt = { 100, 100 };
uint PDlg_Action::Action = eacApply;
//-----------------------------------------------------------
bool PDlg_Action::create()
{
  if(!baseClass::create())
    return false;
  uint idc[] = { IDC_SKIP_ACTION, IDC_SKIP_ALL_ACTION, IDC_APPLY_ACTION, IDC_APPLY_ALL_ACTION };
  SET_CHECK(idc[Action]);
  TCHAR t[64];
  GET_TEXT(IDC_STATIC_ACTION, t);
  TCHAR t2[512];
  wsprintf(t2, t, Txt);
  SET_TEXT(IDC_STATIC_ACTION, t2);
  SetWindowPos(*this, 0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  return true;
}
//-----------------------------------------------------------
void PDlg_Action::CmOk()
{
  uint idc[] = { IDC_SKIP_ACTION, IDC_SKIP_ALL_ACTION, IDC_APPLY_ACTION, IDC_APPLY_ALL_ACTION };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    if(IS_CHECKED(idc[i])) {
      retAction = Action = i;
      break;
      }
  PRect r;
  GetWindowRect(*this, r);
  pt.x = r.left;
  pt.y = r.top;
  baseClass::CmOk();
}
//-----------------------------------------------------------
actionChoose getActionChoose(PWin* owner, actionChoose curr, LPCTSTR filename)
{
  uint act = curr;
  PDlg_Action(owner, act, filename).modal();
  return actionChoose(act);
}
//----------------------------------------------------------------------------
