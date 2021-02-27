//----------- wizardDlg.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
//#include <lm.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild1.h"
#include "pWizardChild2.h"
#include "pWizardChild3.h"
#include "pWizardChild4.h"
#include "pWizardChild5.h"
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
#define X_IMAGE 10
#define Y_IMAGE 15
//----------------------------------------------------------------------------
#define W_IMAGE 120
#define H_IMAGE 165
//----------------------------------------------------------------------------
bool getValidIx(int ix, int& result)
{
  if(pRecipeLB == ix)
    --ix;
  --ix;
  if(ix >= MAX_WIZARD_PAGE)
    ix = MAX_WIZARD_PAGE - 1;
  else if(ix < 0)
    return false;

  result = ix;
  return true;
}
//----------------------------------------------------------------------------
wizardDlg::wizardDlg(PWin* parent, setOfString& set, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), currPage(0), Set(set), currStep(0)
{

  int idChild[] = { IDD_WZ_FIRST_PAGE, IDD_WZ_TREND, IDD_WZ_RECIPE_ROW, IDD_WZ_RECIPE, IDD_WZ_LINKED_PAGE };
  wzChild[0] = new pWizardChild1(this, idChild[0], getHInstance());
  wzChild[1] = new pWizardChild2(this, idChild[1], getHInstance());
  wzChild[2] = new pWizardChild3(this, idChild[2], getHInstance());
  wzChild[3] = new pWizardChild4(this, idChild[3], getHInstance());
  wzChild[4] = new pWizardChild5(this, idChild[4], getHInstance());
  setCustomEraseBkg(true);

}
//----------------------------------------------------------------------------
wizardDlg::~wizardDlg()
{
  destroy();
}
//----------------------------------------------------------------------------
extern void DisplayErrorString(DWORD dwErr);
bool wizardDlg::create()
{
  if(!baseClass::create())
    return false;
  showFirst();
  PRect r(-2, 0, 0, 0);
  for(int i = 0; i < MAX_WIZARD_PAGE; ++i)
    if(!wzChild[i]->setWindowPos(0, r, SWP_NOZORDER | SWP_NOSIZE))
      DisplayErrorString(GetLastError());

  return true;
}
//----------------------------------------------------------------------------
LRESULT wizardDlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static bool toggle;
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_PREV:
          nextStep(false);
          break;

        case IDC_BUTTON_NEXT:
          nextStep(true);
          break;
        }
      break;
    case WM_CUSTOM_WZ:
      switch(LOWORD(wParam)) {
        case CM_NEXT_IS_SAVE:
          SET_TEXT(IDC_BUTTON_NEXT, _T("Salva"));
          break;
        case CM_NEXT_IS_STEP:
          SET_TEXT(IDC_BUTTON_NEXT, _T("Avanti >>"));
          break;
        }
      break;
    case WM_ERASEBKGND:
      do {
        HDC hdc = (HDC)wParam;
        if(!hdc)
          hdc = GetDC(*this);
        PBitmap bmp(IDB_BKG, getHInstance());
        HBRUSH bkgBrush = CreatePatternBrush(bmp);

        HGDIOBJ old = SelectObject(hdc, bkgBrush);
        PRect r;
        GetClientRect(*this, r);
        PatBlt(hdc, 0, 0, r.Width(), r. Height(), PATCOPY);

        DeleteObject(SelectObject(hdc, old));
        if(!wParam)
          ReleaseDC(*this, hdc);
//        return TRUE;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void wizardDlg::nextStep(bool next)
{
  if(!next) {
    --currStep;
    if(currStep <= 0) {
      ShowWindow(GetDlgItem(*this, IDC_BUTTON_PREV), SW_HIDE);
      currPage = 0;
      currStep = 0;
      showFirst();
      }
    else {
      int ix;
      if(!getValidIx(currPage, ix))
        return;
      wzChild[ix]->setStep(currStep);
      wzChild[ix]->setTitle(*this, _T("Wizard Pagine Predefinite"));
      }
    SET_TEXT(IDC_BUTTON_NEXT, _T("Avanti >>"));
    return;
    }

  if(currPage) {
    int ix;
    if(!getValidIx(currPage, ix))
      return;
    int remainSteps = wzChild[ix]->getRemainSteps(currStep);

    if(!remainSteps) {
      if(save())
        CmOk();
      }
    else {
      ShowWindow(GetDlgItem(*this, IDC_BUTTON_PREV), SW_SHOWNORMAL);
      if(1 == remainSteps)
        SET_TEXT(IDC_BUTTON_NEXT, _T("Salva"));
      wzChild[ix]->setStep(++currStep);
      wzChild[ix]->setTitle(*this, _T("Wizard Pagine Predefinite"));
      }
    }


  else {  // !currPage
    currPage = ((pWizardChild1*)wzChild[0])->getChoice();
/*
    if(1 == currPage) {
      if(save())
        CmOk();
      return;
      }
*/
    int ix;
    if(!getValidIx(currPage, ix))
      return;
    int remainSteps = wzChild[ix]->getRemainSteps(currStep++);
    if(1 >= remainSteps)
        SET_TEXT(IDC_BUTTON_NEXT, _T("Salva"));
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_PREV), SW_SHOWNORMAL);
    showNext();
    }
}
//----------------------------------------------------------------------------
void wizardDlg::showFirst()
{
  ShowWindow(*wzChild[0], SW_SHOWNORMAL);
  for(int i = 1; i < MAX_WIZARD_PAGE; ++i) {
    wzChild[i]->setStep(currStep);
    ShowWindow(*wzChild[i], SW_HIDE);
    }
  wzChild[0]->setTitle(*this, _T("Wizard Pagine Predefinite"));
}
//----------------------------------------------------------------------------
void wizardDlg::showNext()
{
  int ix;
  if(!getValidIx(currPage, ix))
    return;
  ShowWindow(*wzChild[ix], SW_SHOWNORMAL);
  ShowWindow(*wzChild[0], SW_HIDE);
  wzChild[ix]->setItem(currPage);
  wzChild[ix]->setStep(currStep);
  wzChild[ix]->setTitle(*this, _T("Wizard Pagine Predefinite"));
}
//----------------------------------------------------------------------------
bool wizardDlg::save()
{
  int ix;
  if(!getValidIx(currPage, ix))
    return false;
  return wzChild[ix]->save(Set);
}
//----------------------------------------------------------------------------
