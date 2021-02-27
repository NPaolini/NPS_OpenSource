//------------- pModDialog.cpp -------------------------------------------
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include "pModDialog.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
LRESULT PModDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          CmOk();
          break;
        case IDCANCEL:
          CmCancel();
          break;
        }
      break;
    }

  return PDialog::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------
void PModDialog::CmOk()
{
  Result = IDOK;
  if(isFlagSet(pfModal) && !isFlagSet(pfDialogToWin))
    EndDialog(getHandle(), IDOK);
  else
    DestroyWindow(getHandle());
}
//------------------------------------------------------------------------
void PModDialog::CmCancel()
{
  Result = IDCANCEL;
  if(isFlagSet(pfModal) && !isFlagSet(pfDialogToWin))
    EndDialog(getHandle(), IDCANCEL);
  else
    DestroyWindow(getHandle());
}
//------------------------------------------------------------------------
