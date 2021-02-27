//----------- pWizardChild1.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//#include <Lmwksta.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild1.h"
#include "common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pWizardChild1::pWizardChild1(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{

}
//----------------------------------------------------------------------------
pWizardChild1::~pWizardChild1()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pWizardChild1::create()
{
  if(!baseClass::create())
    return false;
  SET_CHECK(IDC_RADIO_WZ_FIRST_TREND);
  return true;
}
//----------------------------------------------------------------------------
LRESULT pWizardChild1::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_WZ_FIRST_TREND:
        case IDC_RADIO_WZ_FIRST_RECIPE_ROW:
        case IDC_RADIO_WZ_FIRST_RECIPE:
        case IDC_RADIO_WZ_FIRST_RECIPE_LB:
        case IDC_RADIO_WZ_LINKED_PAGE:
          PostMessage(*getParent(), WM_CUSTOM_WZ, MAKEWPARAM(CM_NEXT_IS_STEP, 0), 0);
          break;

        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool pWizardChild1::save(setOfString& Set)
{
  return false;
}
//----------------------------------------------------------------------------
int pWizardChild1::getChoice()
{
  if(IS_CHECKED(IDC_RADIO_WZ_FIRST_TREND))
    return pTrend;
  else if(IS_CHECKED(IDC_RADIO_WZ_FIRST_RECIPE_ROW))
    return pRecipeRow;
  else if(IS_CHECKED(IDC_RADIO_WZ_FIRST_RECIPE))
    return pRecipe;
  else if(IS_CHECKED(IDC_RADIO_WZ_FIRST_RECIPE_LB))
    return pRecipeLB;
  else if(IS_CHECKED(IDC_RADIO_WZ_LINKED_PAGE))
    return pLinkedPage;
  return notUsed;
}
