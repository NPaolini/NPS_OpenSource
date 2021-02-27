//------------------ svmCalibrate.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmCalibrate.h"
#include "svmMainWorkArea.h"
#include "common.h"
//-----------------------------------------------------------
#define MIN_V 0.3
#define MAX_V 3.0
#define DIFF_V (MAX_V - MIN_V)
//-----------------------------------------------------------
#define MIN_P 0
#define MAX_P 100
#define DIFF_P (MAX_P - MIN_P)
//-----------------------------------------------------------
static void setText(HWND ed, double v)
{
  TCHAR t[64];
  _stprintf_s(t, _T("%0.2f"), v);
  SetWindowText(ed, t);
}
//-----------------------------------------------------------
bool PCalibrate::create()
{
  oldAdjuct = svmManZoom::getAdjuct();
  if(!baseClass::create())
    return false;
  SendMessage(GetDlgItem(*this, IDC_SLIDER1), TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(MIN_P, MAX_P));
  SendMessage(GetDlgItem(*this, IDC_SLIDER1), TBM_SETPAGESIZE, 0, (LPARAM)(DIFF_P / 20));
  setText(GetDlgItem(*this, IDC_EDIT1), oldAdjuct.H);

  SendMessage(GetDlgItem(*this, IDC_SLIDER2), TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(MIN_P, MAX_P));
  SendMessage(GetDlgItem(*this, IDC_SLIDER2), TBM_SETPAGESIZE, 0, (LPARAM)(DIFF_P / 20));
  setText(GetDlgItem(*this, IDC_EDIT2), oldAdjuct.V);
  return true;
}
//-----------------------------------------------------------
infoAdjusct loadAdjuct(infoAdjusct def)
{
  DWORD target;
  bool success = getKeyParam(_T("Adjuct_screen"), MAIN_PATH, target);
  if(success)
    def.H = (double)target / 1000.0;

  success = getKeyParam(_T("Adjuct_screen_V"), MAIN_PATH, target);
  if(success)
    def.V = (double)target / 1000.0;
  else
    def.V = def.H;
  return def;
}
//-----------------------------------------------------------
void saveAdjuct()
{
  infoAdjusct adjuct = svmManZoom::getAdjuct();
  DWORD adjuctH = ROUND_POS_REAL(adjuct.H * 1000);
  DWORD adjuctV = ROUND_POS_REAL(adjuct.V * 1000);

  setKeyParam(_T("Adjuct_screen"), MAIN_PATH, adjuctH);
  setKeyParam(_T("Adjuct_screen_V"), MAIN_PATH, adjuctV);
}
//-----------------------------------------------------------
void PCalibrate::CmOk()
{
  saveAdjuct();
  baseClass::CmCancel();
}
//-----------------------------------------------------------
void PCalibrate::CmCancel()
{
  setAdjuct(oldAdjuct);
  baseClass::CmCancel();
}
//-----------------------------------------------------------
void PCalibrate::setAdjuct(infoAdjusct v)
{
  svmManZoom::setAdjuct(v);
  childScrollWorkArea* mc = getParentWin<childScrollWorkArea>(this);
  if(mc) {
    mc->resetScroll();
    mc->setZoom(svmManZoom::zOne, true);
    InvalidateRect(*mc, 0, 0);
    }
}
//-----------------------------------------------------------
LRESULT PCalibrate::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_EDIT1:
          switch(HIWORD(wParam)) {
            case EN_CHANGE:
              editPos(IDC_EDIT1);
              break;
            }
          break;
        case IDC_EDIT2:
          switch(HIWORD(wParam)) {
            case EN_CHANGE:
              editPos(IDC_EDIT2);
              break;
            }
          break;
        }
      break;
    case WM_HSCROLL:
      evHScrollBar((HWND)lParam, short(LOWORD(wParam)), short(HIWORD(wParam)));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
void PCalibrate::evHScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, TBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    default:
      if(pos)
        break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    }
  chgPos(pos, GetWindowLong(child, GWL_ID));
}
//-----------------------------------------------------------
void PCalibrate::editPos(uint idc)
{
  if(onEdit)
    return;
  onEdit = true;
  TCHAR t[64];
  GET_TEXT(idc, t);
  double v = _tstof(t);
  if(v < MIN_V) {
    v = MIN_V;
    setText(GetDlgItem(*this, idc), v);
    }
  else if(v > MAX_V) {
    v = MAX_V;
    setText(GetDlgItem(*this, idc), v);
    }
  long pos = long((v - MIN_V) / DIFF_V * DIFF_P);
  infoAdjusct adj = svmManZoom::getAdjuct();
  if(IDC_EDIT1 == idc) {
    adj.H = v;
    idc = IDC_SLIDER1;
    }
  else {
    adj.V = v;
    idc = IDC_SLIDER2;
    }
  chgPos(pos, idc);
  setAdjuct(adj);
  onEdit = false;
}
//-----------------------------------------------------------
void PCalibrate::chgPos(DWORD pos, uint idc)
{
  if(onTrack)
    return;
  onTrack = true;
  if(!onEdit) {
    double v = double(pos) / DIFF_P * DIFF_V + MIN_V;
    if(IDC_SLIDER1 == idc)
      idc = IDC_EDIT1;
    else
      idc = IDC_EDIT2;
    setText(GetDlgItem(*this, idc), v);
    }
  else
    SendMessage(GetDlgItem(*this, idc), TBM_SETPOS, TRUE, (LPARAM)pos);
  onTrack = false;
}
//-----------------------------------------------------------
