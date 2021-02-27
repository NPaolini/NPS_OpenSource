//------- dlistchoose.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
//----------------------------------------------------------------------------
#include "dlistChoose.h"
#include "1.h"
#include "language.h"
//----------------------------------------------------------------------------
int TD_ListChoose::choose;
//----------------------------------------------------------------------------
TD_ListChoose::TD_ListChoose(PWin* parent, uint resId)
:
    baseClass(parent, resId)
{
  LB = new PListBox(this, IDC_LISTBOX_CHOOSE_EXP);
}
//--------------------------------------------------------------------------
TD_ListChoose::~TD_ListChoose()
{
  destroy();
}
//--------------------------------------------------------------------------
#ifndef SIZE_A
  #define SIZE_A(a) (sizeof(a) / sizeof(*(a)))
#endif
bool TD_ListChoose::create()
{
  if(!baseClass::create())
    return false;

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_DIALOG_OK);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F2), ID_DIALOG_CANC);

  const int Width = 140;
  const int Height = 70;
  PRect r(0, 0, Width, Height);
  int offsX = (GetSystemMetrics(SM_CXSCREEN) - Width) /2;
  int offsY = (GetSystemMetrics(SM_CYSCREEN) - Height) /3;
  r.Offset(offsX, offsY);
  setWindowPos(0, r, SWP_NOZORDER);
  GetClientRect(*this, r);
//  r.MoveTo(-2,-2);
//  r.Inflate(-7,-7);
  LB->setWindowPos(0, r, SWP_NOZORDER);

#if 1
  smartPointerConstString normal = getStringOrIdByLangGlob(ID_TIT_NORMAL_EXPORT);
  smartPointerConstString binary = getStringOrIdByLangGlob(ID_TIT_BINARY_EXPORT);
  SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)&normal);
  SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)&binary);

#else
  LPCTSTR msg[] = { _T("Normal Export"), _T("Binary Export") };
  for(int i = 0; i < SIZE_A(msg); ++i)
    SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)msg[i]);
#endif
  SendMessage(*LB, LB_SETCURSEL, choose, 0);

  return true;
}
//--------------------------------------------------------------------------
void TD_ListChoose::LBNDblclk()
{
  // INSERT>> Your code here.
  CmOk();
}
//--------------------------------------------------------------------------
void TD_ListChoose::CmOk()
{
  choose =   SendMessage(*LB, LB_GETCURSEL, 0, 0);
  EndDialog(*this, IDOK);
}
//--------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT TD_ListChoose::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_F1:
        EV_BN_CLICKED(IDOK, CmOk)

        case IDC_BUTTON_F2:
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          LBNDblclk();
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);

}
//--------------------------------------------------------------------------

