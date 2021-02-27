//----------- show_edit.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "show_edit.h"
#include "svmDataProject.h"
#include "assocDlg.h"
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#define bkg_Color1 0xC1E1D8
#define bkg_Color2 0xD8E1C1
//----------------------------------------------------------------------------
genShow::genShow(PWin* parent, LPTSTR buff, uint idd, COLORREF color) :
      baseClass(parent, idd), Buff(buff), Color(color), Brush(CreateSolidBrush(color))
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  HINSTANCE hi = getHInstance();
  POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], hi);
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp,  POwnBtnImageStd::wLeft, true);
    btn->setColorRect(cr);
    }
  enableCapture(true);
}
//----------------------------------------------------------------------------
genShow::~genShow()
{
  destroy();
  DeleteObject(Brush);
}
//----------------------------------------------------------------------------
LRESULT genShow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    case WM_NCHITTEST:
      SetWindowLongPtr(*this, DWLP_MSGRESULT, HTCAPTION);
      return TRUE;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH genShow::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  SetBkColor(hdc, Color);
  return (Brush);
}
//----------------------------------------------------------------------------
bool ShowType::create()
{
  if(!baseClass::create())
    return false;
  uint v = _ttoi(Buff);
  SET_CHECK(IDC_CHECK_TYPE1 + v);
  return true;
}
//----------------------------------------------------------------------------
void ShowType::CmOk()
{
  for(uint i = 0; i <= tStrData; ++i) {
    if(IS_CHECKED(IDC_CHECK_TYPE1 + i)) {
      wsprintf(Buff, _T("%d"), i);
      break;
      }
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
bool ShowAssocType::create()
{
  if(!baseClass::create())
    return false;
  switch(*Buff) {
    case 0:
    case _T('-'):
    default:
      SET_CHECK(IDC_CHECK_TYPE1);
      break;
    case _T('V'):
    case _T('v'):
      SET_CHECK(IDC_CHECK_TYPE2);
      break;
    case _T('N'):
    case _T('n'):
      SET_CHECK(IDC_CHECK_TYPE3);
      break;
    case _T('B'):
    case _T('b'):
      SET_CHECK(IDC_CHECK_TYPE4);
      break;
    }
  return true;
}
//----------------------------------------------------------------------------
void ShowAssocType::CmOk()
{
  Buff[0] = _T('-');
  Buff[1] = 0;

  if(IS_CHECKED(IDC_CHECK_TYPE2))
    *Buff = _T('V');
  else if(IS_CHECKED(IDC_CHECK_TYPE3))
    *Buff = _T('N');
  else if(IS_CHECKED(IDC_CHECK_TYPE4))
    *Buff = _T('B');

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
