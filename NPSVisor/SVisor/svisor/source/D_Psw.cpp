//---------- d_psw.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_psw.h"
#include "p_base.h"

#include "1.h"
//----------------------------------------------------------------------------
TD_Psw::TD_Psw(char *buff, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Buff(buff)
{
  E_Psw = new svEdit(this, IDC_EDIT_PSW, MAX_LEN_PSW);
  memset(buff, MAX_LEN_PSW, 0);
}
//----------------------------------------------------------------------------
TD_Psw::~TD_Psw()
{
  destroy();
}
//----------------------------------------------------------------------------
bool TD_Psw::create()
{
  if(!baseClass::create())
    return false;
  setWindowTextByLangGlob(*this, ID_TITLE_INPUT_PSW);
  LPCTSTR p = getString(ID_PSW_CHAR);
  if(p && *p)
//    p = _T("@");
    SendMessage(GetDlgItem(*this, IDC_EDIT_PSW), EM_SETPASSWORDCHAR, (UINT)*p, 0);
//  SendMessage(GetDlgItem(*this, IDC_EDIT_PSW), EM_SETPASSWORDCHAR, (UINT)_T('@'), 0);

  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_Psw::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          return 0;
        case IDOK:
          #if defined(UNICODE) || defined(_UNICODE)
          do {
            TCHAR tmp[30];
            GetWindowText(*E_Psw, tmp, MAX_LEN_PSW);
            for(int i = 0; i < MAX_LEN_PSW; ++i)
              Buff[i] = (char)tmp[i];
            } while(false);
          #else
            GetWindowText(*E_Psw, Buff, MAX_LEN_PSW);
          #endif
          EndDialog(hwnd, IDOK);
          return 0;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

