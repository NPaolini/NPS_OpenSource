//--------- dinputdt.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "dinputdt.h"
#include "p_base.h"
#include "p_date.h"
#include "1.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
#define DIM_INP_DATE 6
//----------------------------------------------------------------------------
TD_InputDate::TD_InputDate(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), fT1(ft1), fT2(ft2)
{
  From = new svEdit(this, IDC_EDIT_INP_DATE_FROM, DIM_INP_DATE);
  timeFrom = new svEdit(this, IDC_EDIT_INP_TIME_FROM);
  To = new svEdit(this, IDC_EDIT_INP_DATE_TO, DIM_INP_DATE);
  timeTo = new svEdit(this, IDC_EDIT_INP_TIME_TO);
  YearFrom = new svEdit(this, IDC_EDIT_YEAR_FROM);
  YearTo = new svEdit(this, IDC_EDIT_YEAR_TO);
  TxtFrom = new PStatic(this, IDC_STATICTEXT_INP_FROM);
  TxtTo = new PStatic(this, IDC_STATICTEXT_INP_TO);
  timeFrom->setFilter(new PTimeFilter);
  timeTo->setFilter(new PTimeFilter);
//  From->SetValidator(new TPXPictureValidator("[##/##]",true));
//  To->SetValidator(new TPXPictureValidator("[##/##]",true));
}
//----------------------------------------------------------------------------
TD_InputDate::~TD_InputDate()
{
  destroy();
}
//----------------------------------------------------------------------------
static void restoreMonthDay(FILETIME &ft, PEdit *ctrl, PEdit *ctrlTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  int year = st.wYear;

  TCHAR t[40];
  GetWindowText(*ctrl, t, SIZE_A(t));

  _tcscat_s(t, SIZE_A(t), _T("|"));
  int len = _tcslen(t);

  GetWindowText(*ctrlTime, t + len, SIZE_A(t) - len);

  unformat_data(t, st, whichData());

  st.wYear = year;
  SystemTimeToFileTime(&st, &ft);
}
//----------------------------------------------------------------------------
static void formatMonthDay(FILETIME &ft, PEdit *ctrl, PEdit *ctrlTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR t[30];
  int d1;
  int d2;
  switch(whichData()) {
    case European:
      d1 = st.wDay;
      d2 = st.wMonth;
      break;
    default:
      d2 = st.wDay;
      d1 = st.wMonth;
      break;
    }
  wsprintf(t, _T("%02d/%02d"), d1, d2);
  SetWindowText(*ctrl, t);

  wsprintf(t, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
  SetWindowText(*ctrlTime, t);
}
//----------------------------------------------------------------------------
static void formatYear(FILETIME &ft, PEdit *ctrl)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR t[10];
  wsprintf(t, _T("/%02d"), st.wYear);
  SetWindowText(*ctrl, t);
}
//----------------------------------------------------------------------------
bool TD_InputDate::create()
{
  if(!baseClass::create())
    return false;

  formatMonthDay(fT1, From, timeFrom);
  formatYear(fT1, YearFrom);

  formatMonthDay(fT2, To, timeTo);
  formatYear(fT2, YearTo);
  setWindowTextByLangGlob(*TxtFrom, ID_INP_DATE_FROM);
  setWindowTextByLangGlob(*TxtTo, ID_INP_DATE_TO);

  setWindowTextByLangGlob(*this, ID_DATA_HEADER);

  return true;
}
//----------------------------------------------------------------------------
HBRUSH TD_InputDate::evCtlColor(HDC dc, HWND hWndChild, uint /*ctlType*/)
{
  HBRUSH result = 0;

  int id = ::GetDlgCtrlID(hWndChild);
  if(IDC_EDIT_YEAR_FROM == id || IDC_EDIT_YEAR_TO == id) {
    result = (HBRUSH)GetStockObject(WHITE_BRUSH);
    ::SetTextColor(dc, 0);
    }
  return result;
}
//----------------------------------------------------------------------------
LRESULT TD_InputDate::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDC_BUTTON_F1:
          restoreMonthDay(fT1, From, timeFrom);
          restoreMonthDay(fT2, To, timeTo);
          EndDialog(hwnd, IDOK);
          break;
        case IDCANCEL:
        case IDC_BUTTON_F2:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

//----------------------------------------------------------------------------
TD_InputDateOnlyDay::TD_InputDateOnlyDay(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId, HINSTANCE hinst) :
  baseClass(ft1, ft2, parent, resId, hinst)
{
}
//----------------------------------------------------------------------------
TD_InputDateOnlyDay::~TD_InputDateOnlyDay()
{
  destroy();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void formatMonthYear(FILETIME &ft, PEdit *ctrl)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR t[10];
  wsprintf(t, _T("/%02d/%04d"), st.wMonth, st.wYear);
  SetWindowText(*ctrl, t);
}
//----------------------------------------------------------------------------
static void formatDay(FILETIME &ft, PEdit *ctrl, PEdit *ctrlTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR t[20];
  wsprintf(t, _T("%02d"), st.wDay);
  SetWindowText(*ctrl, t);
  wsprintf(t, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
  SetWindowText(*ctrlTime, t);
}
//----------------------------------------------------------------------------
bool TD_InputDateOnlyDay::create()
{
  if(!TD_InputDate::create())
    return false;

  formatDay(fT1, From, timeFrom);
  formatMonthYear(fT1, YearFrom);

  formatDay(fT2, To, timeTo);
  formatMonthYear(fT2, YearTo);

/*
#if 1
  setWindowTextByLangGlob(*TxtFrom, ID_INP_DATE_FROM);
  setWindowTextByLangGlob(*TxtTo, ID_INP_DATE_TO);
#else
  SetWindowText(*TxtFrom, getStringOrId(ID_INP_DATE_FROM));
  SetWindowText(*TxtTo, getStringOrId(ID_INP_DATE_TO));
#endif
*/
  return true;
}
//----------------------------------------------------------------------------
static bool restoreDay(FILETIME &ft, PEdit *ctrl, PEdit *ctrlTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  int year = st.wYear;
  int month = st.wMonth;
  TCHAR t[40];
  GetWindowText(*ctrl, t, SIZE_A(t));

  int day = _ttoi(t);

  GetWindowText(*ctrlTime, t, SIZE_A(t));
  unformat_time(t, st);

  st.wYear = year;
  st.wMonth = month;
  st.wDay = WORD(day);
  return toBool(SystemTimeToFileTime(&st, &ft));
}
//----------------------------------------------------------------------------
void TD_InputDateOnlyDay::BNClickedF1()
{
  FILETIME ft1 = fT1;
  FILETIME ft2 = fT2;
  if(restoreDay(ft1, From, timeFrom))
    if(restoreDay(ft2, To, timeTo)) {
      fT1 = ft1;
      fT2 = ft2;
      }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT TD_InputDateOnlyDay::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDC_BUTTON_F1:
          BNClickedF1();
          EndDialog(hwnd, IDOK);
          return 1;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static void restoreTime(FILETIME &ft, PEdit *ctrlTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  int year = st.wYear;
  int month = st.wMonth;
  int day = st.wDay;

  TCHAR t[40];
  GetWindowText(*ctrlTime, t, SIZE_A(t));
  unformat_time(t, st);

  st.wYear = year;
  st.wMonth = month;
  st.wDay = day;
  SystemTimeToFileTime(&st, &ft);
}
//----------------------------------------------------------------------------
static void formatTime(FILETIME &ft, PEdit *ctrlTime)
{
  TCHAR t[40];
  set_format_data(t, SIZE_A(t), ft, whichData(), _T("|"));
//  set_format_time(t, SIZE_A(t), ft);
  SetWindowText(*ctrlTime, t + 11);
}
//----------------------------------------------------------------------------
TD_InputTime::TD_InputTime(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), fT1(ft1), fT2(ft2)
{
  timeFrom = new svEdit(this, IDC_EDIT_INP_TIME_FROM);
  timeTo = new svEdit(this, IDC_EDIT_INP_TIME_TO);
  dateInfo = new PStatic(this, IDC_STATICTEXT_DATEINFO);
  TxtFrom = new PStatic(this, IDC_STATICTEXT_INP_FROM);
  TxtTo = new PStatic(this, IDC_STATICTEXT_INP_TO);
  timeFrom->setFilter(new PTimeFilter);
  timeTo->setFilter(new PTimeFilter);
}
//----------------------------------------------------------------------------
TD_InputTime::~TD_InputTime()
{
  destroy();
}
//----------------------------------------------------------------------------
bool TD_InputTime::create()
{
  if(!baseClass::create())
    return false;

  formatTime(fT1, timeFrom);
  formatTime(fT2, timeTo);

  setWindowTextByLangGlob(*TxtFrom, ID_INP_TIME_FROM);
  setWindowTextByLangGlob(*TxtTo, ID_INP_TIME_TO);

  TCHAR date[40];
  set_format_data(date, SIZE_A(date), fT1, whichData(), _T("|"));
  date[10] = 0;
  SetWindowText(*dateInfo, date);

  setWindowTextByLangGlob(*this, ID_TIME_HEADER);

  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_InputTime::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDC_BUTTON_F1:
          restoreTime(fT1, timeFrom);
          restoreTime(fT2, timeTo);
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

