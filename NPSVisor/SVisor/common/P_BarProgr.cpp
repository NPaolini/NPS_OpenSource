//----- P_BarProgr.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "P_BarProgr.h"
#include "macro_utils.h"
//--------------------------------------------------------------------------
#define BASE_SHIFT 10
#define BASE_MAX (1 << BASE_SHIFT)
#define CALC_POS(v, m) (((v) << BASE_SHIFT) / (m))
//#define BASE_MAX 1024
//--------------------------------------------------------------------------
BarProgr::BarProgr(PWin* par, DWORD maxStep, bool pumpMsg) :
  baseClass(par, IDD_PROGRESS), MaxStep(maxStep ? maxStep : BASE_MAX), stopped(false), idTimer(0),
  pumpMsg(pumpMsg) {}
//--------------------------------------------------------------------------
static DWORD TickCount;
//--------------------------------------------------------------------------
bool BarProgr::create()
{
  if(!baseClass::create())
    return false;
#if 1
  ::SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, BASE_MAX));
#else
  ::SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_SETRANGE32, 0, (LPARAM)MaxStep);
#endif
  TCHAR buff[64];
  wsprintf(buff, _T(" / %d"), MaxStep);
  SET_TEXT(IDC_STATIC_WAIT_MAX, buff);
  TickCount = GetTickCount();
//  SetTimer(*this, idTimer = 123, 10, 0);
  return true;
}
//--------------------------------------------------------------------------
#define WAIT_BEFORE_SHOW 500
//--------------------------------------------------------------------------
bool BarProgr::setPos(DWORD ix)
{
  if(!getHandle())
    return true;
  if(TickCount) {
    DWORD tick = GetTickCount();
    DWORD diff = tick - TickCount;
    if(tick < TickCount)
      diff = (DWORD)-1 - diff;
    if(diff >= WAIT_BEFORE_SHOW) {
      // ci sta mettendo più di WAIT_BEFORE_SHOW tempo
      TickCount = 0;
      // se non siamo ancora a metà del range visualizza la finestrina,
      // altrimenti mancherà al max WAIT_BEFORE_SHOW tempo e non vale la pena di
      // visualizzarla magari come un flash
//      if(MaxStep > ix * 2 || ix > 50) {
        ShowWindow(*this, SW_SHOWNORMAL);
        InvalidateRect(*this, 0, 1);
        UpdateWindow(*this);
//        }
      }
    }
  ::SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_SETPOS, CALC_POS(ix, MaxStep), 0);
  TCHAR buff[16];
  wsprintf(buff, _T("%d"), ix);
  SET_TEXT(IDC_STATIC_WAIT_COUNT, buff);
  if(pumpMsg)
    getAppl()->pumpMessages();
  return !stopped;
}
//--------------------------------------------------------------------------
LRESULT BarProgr::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
/*
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_TIMER:
      KillTimer(hwnd, idTimer);
      idTimer = 0;
      ShowWindow(*this, SW_SHOWNORMAL);
      break;
*/
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_PROGRESS_ABORT:
          stopped = true;
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
