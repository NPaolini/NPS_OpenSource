//------ P_ModalRecipe.cpp -------------------------------------------------------
#define _WIN32_WINNT 0x0501
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "P_ModalRecipe.h"
#include "pAllObj.h"
#include "id_btn.h"
#include "password.h"
#include "p_util.h"
//#include "def_dir.h"
#include "lnk_body.h"
//#include "newnormal.h"
#include "sizer.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_ModalRecipe::P_ModalRecipe(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    TD_Recipe(0, parent, resId, hInstance), P_ModalBody(0, parent, resId, hInstance)
{}
//----------------------------------------------------------------------------
P_ModalRecipe::~P_ModalRecipe()
{
  exiting = true;
  TD_Recipe::destroy();
}
//----------------------------------------------------------------------------
LRESULT P_ModalRecipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDC_BUTTON_F2:
        case IDC_BUTTON_F3:
        case IDC_BUTTON_F4:
        case IDC_BUTTON_F5:
        case IDC_BUTTON_F6:
        case IDC_BUTTON_F7:
        case IDC_BUTTON_F8:
        case IDC_BUTTON_F9:
        case IDC_BUTTON_F10:
        case IDC_BUTTON_F11:
        case IDC_BUTTON_F12:
          pushedBtn(LOWORD(wParam) - IDC_BUTTON_F1 + ID_F1);
          return 1;
        }
      break;
    case WM_NCHITTEST:
      if(noCaption)
        return HTCAPTION;
      break;
    case WM_NCMOUSEMOVE:
      if(noCaption)
        SetCursor(LoadCursor(TD_Recipe::getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    case WM_NCLBUTTONDOWN:
      if(noCaption)
        SetCursor(LoadCursor(TD_Recipe::getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    case WM_NCLBUTTONUP:
      if(noCaption)
        SetCursor(LoadCursor(TD_Recipe::getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    }
  return TD_Recipe::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModalRecipe::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      return P_ModalBody::pushedBtn(ID_F1);
    default:
      break;
    }
  return TD_Recipe::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
bool P_ModalRecipe::create()
{
  if(!P_ModalBody::create())
    return false;
  TD_Recipe::Attr.id = IDD_RECIPE;
  return true;
}
//----------------------------------------------------------------------------
void P_ModalRecipe::refreshBody()
{
  if(exiting)
    return;
  TD_Recipe::refreshBody();
}
//----------------------------------------------------------------------------
void P_ModalRecipe::setReady(bool first)
{
  P_ModalBody::setReady(first);
  TD_Recipe::convertRecipe();
  TD_Recipe::prepareBitToFunct();
  P_ModalBody::refresh();
}
//----------------------------------------------------------------------------
P_ModelessRecipe::P_ModelessRecipe(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
  P_ModalRecipe(0, parent, resId, hInstance), TD_Recipe(0, parent, resId, hInstance), P_ModalBody(0, parent, resId, hInstance)
{
}
//----------------------------------------------------------------------------
P_ModelessRecipe::~P_ModelessRecipe()
{
  TD_Recipe::destroy();
}
//----------------------------------------------------------------------------
LRESULT P_ModelessRecipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ACTIVATE:
    case WM_POST_ACTIVATE:
      // bypassa il controllo del modal
      return P_DefBody::windowProc(hwnd, message, wParam, lParam);
    case WM_DESTROY:
      savePos(hwnd);
      break;
    }
  return P_ModalRecipe::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModelessRecipe::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      return 0;

    default:
      break;
    }
  return P_ModalRecipe::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
extern void getRememberPos(LPCTSTR page, int& x, int& y);
extern void setRememberPos(HWND hwnd, LPCTSTR page);
//----------------------------------------------------------------------------
void P_ModelessRecipe::setReady(bool first)
{
  LPCTSTR p = TD_Recipe::getPageString(ID_MODELESS_MEMO_POS);
  if(!(p && _ttoi(p))) {
    LPCTSTR page = P_ModalBody::getPageName();
    getRememberPos(page, TD_Recipe::Attr.x, TD_Recipe::Attr.y);
    }
  P_ModalRecipe::setReady(true);
  TD_Recipe::Attr.id = IDD_RECIPE;
  P_ModalBody::refresh();
}
//----------------------------------------------------------------------------
void P_ModelessRecipe::savePos(HWND hwnd)
{
  LPCTSTR p = TD_Recipe::getPageString(ID_MODELESS_MEMO_POS);
  if(p && _ttoi(p))
    return;

  LPCTSTR page = TD_Recipe::getPageName();
  setRememberPos(hwnd, page);
}
