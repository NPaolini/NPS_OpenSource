//--------- dchoosnm.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "dchoosnm.h"
#include "1.h"
#include "p_base.h"
//----------------------------------------------------------------------------
#define FIRST_VALUE 1e-200
//----------------------------------------------------------------------------
static double sMax = FIRST_VALUE;
static double sMin = FIRST_VALUE;
//----------------------------------------------------------------------------
TD_ChooseNum::TD_ChooseNum(PWin* parent,  double &init, double& end, bool useInfoMax, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Init(init), End(end), useInfoMax(useInfoMax)
{
  a_ = new svEdit(this, IDC_EDIT_TO, 255);
  da_ = new svEdit(this, IDC_EDIT_FROM, 255);
  txtA = new PStatic(this, IDC_STATIC_TO);
  txtDa = new PStatic(this, IDC_STATIC_FROM);
  infoMax = new PStatic(this, IDC_INFO_MAX);
  txtMax = new PStatic(this, IDC_STATIC_MAX);
  OkBtn = new PButton(this, IDC_BUTTON_F1);
  CancBtn = new PButton(this, IDC_BUTTON_F2);
}
//----------------------------------------------------------------------------
TD_ChooseNum::~TD_ChooseNum()
{
  destroy();
}
//----------------------------------------------------------------------------
/*
void zeroTrim(LPTSTR t)
{
  int len = _tcslen(t);
  for(int i = len - 1; i > 0; --i) {
    if(_T('.') == t[i]) {
      t[i] = 0;
      break;
      }
    else if(_T('0') == t[i])
      t[i] = 0;
    else break;
    }
}
*/
//----------------------------------------------------------------------------
bool TD_ChooseNum::create()
{
  if(!baseClass::create())
    return false;

  if(FIRST_VALUE == sMax)
    sMax = End;
  if(FIRST_VALUE == sMin)
    sMin = Init;
  TCHAR t[64];
  _stprintf_s(t, SIZE_A(t), _T("%0.5f"), sMin);
  zeroTrim(t);
//  _itot(sMin, t, 10);
  SetWindowText(*da_, t);
  _stprintf_s(t, SIZE_A(t), _T("%0.5f"), sMax);
  zeroTrim(t);
//  _itot(sMax, t, 10);
  SetWindowText(*a_, t);
  if(useInfoMax) {
    _stprintf_s(t, SIZE_A(t), _T("%0.5f"), End);
//  _itot(End, t, 10);
    SetWindowText(*infoMax, t);
#if 1
    setWindowTextByLangGlob(*txtMax, ID_TXT_MAX_NUM);
    }
  setWindowTextByLangGlob(*this, ID_TITLE_CHOOSE_NUM);
  setWindowTextByLangGlob(*txtDa, ID_FROM_NUM);
  setWindowTextByLangGlob(*txtA, ID_TO_NUM);
  setWindowTextByLangGlob(*OkBtn, ID_DIALOG_OK);
  setWindowTextByLangGlob(*CancBtn, ID_DIALOG_CANC);
#else

    SetWindowText(*txtMax, getStringOrId(ID_TXT_MAX_NUM));
    }
  SetWindowText(*this, getStringOrId(ID_TITLE_CHOOSE_NUM));
  SetWindowText(*txtDa, getStringOrId(ID_FROM_NUM));
  SetWindowText(*txtA, getStringOrId(ID_TO_NUM));
  SetWindowText(*OkBtn, getStringOrId(ID_DIALOG_OK));
  SetWindowText(*CancBtn, getStringOrId(ID_DIALOG_CANC));
#endif
  return true;
}
//----------------------------------------------------------------------------
bool TD_ChooseNum::BNClicked_F1()
{
  TCHAR t[20];
  GetWindowText(*da_, t, SIZE_A(t));
  Init = _tstof(t);
  GetWindowText(*a_, t, SIZE_A(t));
  End = _tstof(t);
  if(End < Init)
    return false;
  sMin = Init;
  sMax = End;
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT TD_ChooseNum::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDC_BUTTON_F1:
          if(BNClicked_F1())
            EndDialog(hwnd, IDOK);
          break;
        case IDCANCEL:
        case IDC_BUTTON_F2:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

