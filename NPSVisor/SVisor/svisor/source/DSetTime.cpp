//------- dsettime.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "dsettime.h"
#include "1.h"
#include "p_base.h"
//----------------------------------------------------------------------------
static int sHour = -1;
static int sMinute = -1;
//----------------------------------------------------------------------------
TD_SetTime::TD_SetTime(PWin* parent,  int &hour, int& minute, int id_caption, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Hour(hour), Minute(minute), idCaption(id_caption)
{
  hour_ = new svEdit(this, IDC_EDIT_MAINT_HOURS, 9);
  minute_ = new svEdit(this, IDC_EDIT_MAINT_MINUTES, 9);
  txtHour = new PStatic(this, IDC_STATICTEXT_MAINT_HOURS);
  txtMinute = new PStatic(this, IDC_STATICTEXT_MAINT_MINUTES);
}
//----------------------------------------------------------------------------
TD_SetTime::~TD_SetTime()
{
  destroy();
}
//----------------------------------------------------------------------------
bool TD_SetTime::create()
{
  if(!baseClass::create())
    return false;

  if(sMinute < 0)
    sMinute = Minute;
  if(sHour < 0)
    sHour = Hour;
  TCHAR t[20];
  _itot_s(sHour, t, SIZE_A(t), 10);
  SetWindowText(*hour_, t);
  _itot_s(sMinute, t, SIZE_A(t), 10);
  SetWindowText(*minute_, t);

  if(!idCaption)
    idCaption = ID_TITLE_MAINT_SET_TIME;
  setWindowTextByLangGlob(*this, idCaption);
  setWindowTextByLangGlob(*txtHour, ID_MAINT_SET_HOUR);
  setWindowTextByLangGlob(*txtMinute, ID_MAINT_SET_MINUTE);
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_SetTime::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          CmOk();
          EndDialog(hwnd, IDOK);
          break;
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_SetTime::CmOk()
{
  TCHAR t[20];
  GetWindowText(*hour_, t, SIZE_A(t));
  Hour = _ttoi(t);
  GetWindowText(*minute_, t, SIZE_A(t));
  Minute = _ttoi(t);
  sHour = Hour;
  sMinute = Minute;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

