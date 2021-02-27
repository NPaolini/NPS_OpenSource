//------- d_svdfl.cpp --------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_svdfl.h"
#include "p_base.h"
#include "1.h"
//----------------------------------------------------------------------------
#define MAX_DELAY_RESPONCE 5000
//----------------------------------------------------------------------------
void showFileResult(bool success, PWin *parent)
{
  do {
    TD_SavedFile wait(success, parent);
    wait.create();
    DWORD tick = GetTickCount();
    do {
      if(wait.isFinished())
        break;
      getAppl()->pumpMessages();
      if(!parent->getHandle()) // potrebbe essere terminato il programma
        break;
      if(GetTickCount() - tick > MAX_DELAY_RESPONCE)
        break;
      } while(true);
    } while(0);
  SetFocus(*parent);
}
//----------------------------------------------------------------------------
bool showMsgFileExist(PWin *parent)
{
  return IDYES == msgBoxByLangGlob(parent, ID_EXIST, ID_ALERT, MB_YESNO | MB_ICONINFORMATION);
}
//----------------------------------------------------------------------------
TD_SavedFile::TD_SavedFile(bool success, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Success(success),
      mustClose(false)
{
  TxtSaved = new PStatic(this, IDC_STATICTEXT_SAVED_FILE);
  BmpSaved = new PStatic(this, IDC_STATICBITMAP_SAVE);
  BmpNotSaved = new PStatic(this, IDC_STATICBITMAP_NOT_SAVE);
}
//----------------------------------------------------------------------------
TD_SavedFile::~TD_SavedFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool TD_SavedFile::create()
{
  if(!baseClass::create())
    return false;
  setWindowTextByLangGlob(*TxtSaved, Success ? ID_FILE_SAVED : ID_FILE_NOT_SAVED);
//  SetWindowText(*TxtSaved, getStringOrId(Success ? ID_FILE_SAVED : ID_FILE_NOT_SAVED));
  if(Success) {
    ShowWindow(*BmpSaved, SW_SHOW);
    ShowWindow(*BmpNotSaved, SW_HIDE);
    }
  else {
    ShowWindow(*BmpSaved, SW_HIDE);
    ShowWindow(*BmpNotSaved, SW_SHOW);
    }
  return true;
}
//----------------------------------------------------------------------------
bool TD_SavedFile::preProcessMsg(MSG& msg)
{
  if(WM_CHAR == msg.message || WM_LBUTTONDOWN == msg.message)
    mustClose = true;
  return baseClass::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
LRESULT TD_SavedFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
          mustClose = true;
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------

