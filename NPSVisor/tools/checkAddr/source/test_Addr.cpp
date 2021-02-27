//----------- test_Addr.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdio.h>
#include "test_Addr.h"
#include "PEdit.h"
#include "pCommonFilter.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"

//#include "mainDlg.h"
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor1 YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xdf,0xff,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
static TCHAR currPath[_MAX_PATH];
//----------------------------------------------------------------------------
test_Addr::test_Addr(PWin * parent, uint id, HINSTANCE hinst) :
      baseClass(parent, id, hinst), hfEdit(0),
      Brush1(CreateSolidBrush(bkgColor1))

{
  GetCurrentDirectory(SIZE_A(currPath), currPath);
  int idBmp[] = { IDB_RUN, IDB_FOLDER };
  int idBtn[] = { IDC_BUTTON_RUN, IDC_BUTTON_SEARCH_FILE };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true );
    }
  hfEdit = D_FONT(18, 0, 0, _T("arial"));
  PEdit* ed = new PEdit(this, IDC_EDIT_PATH);
  ed->setFont(hfEdit);
}
//----------------------------------------------------------------------------
test_Addr::~test_Addr()
{
  destroy();
  DeleteObject(HGDIOBJ(Brush1));
  if(hfEdit)
    DeleteObject(HGDIOBJ(hfEdit));
}
//----------------------------------------------------------------------------
extern void setPath(LPCTSTR path);
extern void getPath(LPTSTR path);
//-----------------------------------------------------------
bool test_Addr::create()
{
  if(!baseClass::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
  TCHAR path[_MAX_PATH];
  ::getPath(path);
  SET_TEXT(IDC_EDIT_PATH, path);
  return true;
}
//----------------------------------------------------------------------------
LRESULT test_Addr::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RUN:
          CmOk();
          break;
        case IDC_BUTTON_SEARCH_FILE:
          getPath();
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
//-----------------------------------------------------------
HBRUSH test_Addr::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_EDIT_PATH:
      SetTextColor(hdc, RGB(0, 0, 128));
      SetBkColor(hdc, bkgColor1);
      return (Brush1);
    }
  return 0;
}
//----------------------------------------------------------------------------
void test_Addr::getPath()
{
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PATH, path);
  if(PChooseFolder(*this, path)) {
    SET_TEXT(IDC_EDIT_PATH, path);
    ::setPath(path);
    }
}
//----------------------------------------------------------------------------
void test_Addr::CmOk()
{
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PATH, path);
  ::setPath(path);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
