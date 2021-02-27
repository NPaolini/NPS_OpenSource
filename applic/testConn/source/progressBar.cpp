//-------- progressBar.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include "progressBar.h"
#include "p_util.h"
#include "pOwnBtnImageStd.h"
//----------------------------------------------------------------------------
progressBar::progressBar(PWin* parent, LPCTSTR message, DWORD max_count, COLORREF bar, uint resId,  HINSTANCE hInstance)
  : baseClass(parent, resId, hInstance), barColor(bar),
    Aborted(false), Message(str_newdup(message)), maxCount(max_count), currPos(0), oldFocus(0)
{
  new POwnBtnImageStd(this, IDC_BUTTON_ABORT_COMM,
      new PBitmap(IDB_BITMAP_CANC, getHInstance()), POwnBtnImageStd::wLeft, true);
}
//----------------------------------------------------------------------------
progressBar::~progressBar()
{
  destroy();
  Aborted = true;
  delete []Message;
  if(oldFocus && IsWindow(oldFocus))
    SetFocus(oldFocus);
}
//----------------------------------------------------------------------------
#define PBS_SMOOTH              0x01
#define PBS_VERTICAL            0x04
#define PBM_SETBARCOLOR         (WM_USER+9)             // lParam = bar color
#define PBM_SETRANGE32          (WM_USER+6)  // lParam = high, wParam = low
//----------------------------------------------------------------------------
bool progressBar::create()
{
  oldFocus = GetFocus();
  if(!baseClass::create())
    return false;
  HWND child = GetDlgItem(*this, IDC_PROGRESSBAR_COMM);

  SendMessage(child, PBM_SETRANGE32, 0, (LPARAM) maxCount);
  SendMessage(child, PBM_SETBARCOLOR, 0, (LPARAM) barColor);
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT progressBar::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      Aborted = true;
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ABORT_COMM:
          Aborted = true;
          //DestroyWindow(hwnd);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool progressBar::setCount(DWORD count)
{
  if(Aborted)
    return false;
  currPos = count;
  SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR_COMM), PBM_SETPOS, count, 0);

  TCHAR buff[500];
  wsprintf(buff, Message, count, maxCount);
  SetDlgItemText(*this, IDC_STATICTEXT_PROGRESS, buff);
  return true;
}
