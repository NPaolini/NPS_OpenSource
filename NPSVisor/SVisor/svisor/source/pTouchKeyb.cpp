//------ pTouchKeyb.cpp -------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "pTouchKeyb.h"
#include "pAllObj.h"
#include "id_btn.h"
#include "p_util.h"
#include "pvarEdit.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
bool onTouchKeyb(PWin* child)
{
  pTouchKeyb* par = dynamic_cast<pTouchKeyb*>(child->getParent());
  return toBool(par);
}
//----------------------------------------------------------------------------
pTouchKeyb::pTouchKeyb(int /*idParent*/, PWin* parent, uint resId, HWND hwTarget, bool& setup, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance), hwTarget(hwTarget), Setup(setup)
{
  init();
}
//----------------------------------------------------------------------------
pTouchKeyb::pTouchKeyb(PWin* parent, LPCTSTR pageDescr, HWND hwTarget, bool& setup) :
    baseClass(parent, pageDescr), hwTarget(hwTarget), Setup(setup)
{
  init();
}
//----------------------------------------------------------------------------
pTouchKeyb::~pTouchKeyb()
{
  destroy();
}
//----------------------------------------------------------------------------
void pTouchKeyb::init()
{
  StatusModifier[0].key = VK_CONTROL;
  StatusModifier[1].key = VK_MENU;
  StatusModifier[2].key = VK_SHIFT;

  StatusModifier[0].stat = false;
  StatusModifier[1].stat = false;
  StatusModifier[2].stat = false;
  oldSel = 0;
}
//----------------------------------------------------------------------------
bool pTouchKeyb::create()
{
  smartPointerConstString tit = getTitle();
  if(!baseClass::create())
    return false;
  if(!Edi.getElem()) {
    PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F2, 0), 0);
    return false;
    }
  UINT pswChar = SendMessage(hwTarget, EM_GETPASSWORDCHAR, 0, 0);
  if(pswChar) {
    Edi[0]->makeSingleLine();
    SendMessage(*Edi[0], EM_SETPASSWORDCHAR, (WPARAM)pswChar, 0);
//    SendMessage(*Edi[0], EM_SETPASSWORDCHAR, (WPARAM)(UINT)_T('@'), 0);
    }
  Edi[0]->copyFilter(hwTarget);
  UINT len = SendMessage(hwTarget, WM_GETTEXTLENGTH, 0, 0);
  if(len) {
    LPTSTR buff = new TCHAR[len + 1];
    GetWindowText(hwTarget, buff, len + 1);
    SetWindowText(*Edi[0], buff);
    delete []buff;
    }
  SetWindowText(*this, tit);
  InvalidateRect(*this, 0, 1);
  SetWindowPos(*this, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  oldSel = MAKEWPARAM(0, 65535);
  SetFocus(*Edi[0]);

  return true;
}
//----------------------------------------------------------------------------
bool pTouchKeyb::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYUP:
      switch(msg.wParam) {
        case VK_RETURN:
          PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F1, 0), 0);
          break;
        case VK_ESCAPE:
          PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F2, 0), 0);
          break;
        }
      break;
    }
  return baseClass::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
LRESULT pTouchKeyb::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
      switch(HIWORD(wParam)) {
        case BN_CLICKED:
        case BN_DBLCLK:
          action_Btn(LOWORD(wParam));
          return 1;

        case EN_KILLFOCUS:
          oldSel = SendMessage(*Edi[0], EM_GETSEL, 0, 0);
          break;
        case EN_SETFOCUS:
          SendMessage(*Edi[0], EM_SETSEL, (WPARAM)LOWORD(oldSel), (LPARAM)HIWORD(oldSel));
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
#ifndef WM_LAST_EXT_ONMEM
  #define WM_LAST_EXT_ONMEM 20
#endif
//-----------------------------------------------------------
#define ID_PRPH_4_KEYB 100
//----------------------------------------------------------------------------
void pTouchKeyb::action_Btn(uint idBtn)
{  // se non è nel range dei pulsanti
  if(idBtn < ID_INIT_VAR_BTN || (ID_INIT_VAR_BTN + ADD_INIT_VAR) <= idBtn)
    return;

  // se non viene trovato nei pulsanti allocati esce
  uint nElem = Btn.getElem();
  PVarBtn* p_Btn = 0;
  for(uint i = 0; i < nElem; ++i) {
    PVarBtn* pBtn = Btn[i];
    if(pBtn->getId() == idBtn) {
      p_Btn = pBtn;
      break;
      }
    }
  if(!p_Btn)
    return;

  int idprf = p_Btn->getIdPrph();

  if(!idprf) {
    int addr = p_Btn->getAddr();
    if(ID_F1 <= addr && addr <= ID_F12)
      PostMessage(*Par, WM_MYHOTKEY, addr, 0);
    return;
    }


  if(WM_LAST_EXT_ONMEM != idprf && ID_PRPH_4_KEYB != idprf)
    return;

#define MAX_CHAR_ 0xffff
//#define MAX_CHAR_ 0xff

  uint addr = p_Btn->getAddr();
  if(!addr || addr > MAX_CHAR_)
    return;

  sendChar(p_Btn, addr);
}
//----------------------------------------------------------------------------
#define ID_VK_SETUP 1
//----------------------------------------------------------------------------
void pTouchKeyb::sendChar(PVarBtn* pBtn, uint addr)
{
  switch(addr) {
    case ID_VK_SETUP:
      postSetup();
      break;
    case VK_CONTROL:
    case VK_MENU:
    case VK_SHIFT:
      toggleStatus(pBtn, addr);
      break;
    default:
      performSendChar(addr);
      break;
    }
}
//----------------------------------------------------------------------------
void pTouchKeyb::postSetup()
{
//  mainClient* mc = getMain(this);

//  PostMessage(*mc, WM_MY_HOTKEY, ID_FZ_SETUP, 0);
  Setup = true;
  pushedBtn(ID_F2);
}
//----------------------------------------------------------------------------
void pTouchKeyb::toggleStatus(PVarBtn* pBtn, uint addr)
{
  HWND hBtn = pBtn->getHwnd();
  PWin* w = PWin::getWindowPtr(hBtn);
  POwnBtnSwitch* pBS = dynamic_cast<POwnBtnSwitch*>(w);
  if(w) {
    for(uint i = 0; i < SIZE_A(StatusModifier); ++i) {
      if(addr == StatusModifier[i].key) {
        StatusModifier[i].stat = !StatusModifier[i].stat;
        break;
        }
      }
    }
}
//----------------------------------------------------------------------------
void pTouchKeyb::performSendChar(uint addr)
{
  PVarEdit* pEdi = Edi[0];
  SetFocus(*pEdi);
  for(uint i = 0; i < SIZE_A(StatusModifier); ++i)
    if(StatusModifier[i].stat)
      down_click(StatusModifier[i].key);
  click(addr);
  for(int i = SIZE_A(StatusModifier) - 1; i >= 0; --i)
    if(StatusModifier[i].stat)
      up_click(StatusModifier[i].key);

//  if(pswChar)
//    SendMessage(*Edi[0], EM_SETPASSWORDCHAR, (UINT)_T('@'), 0);
}
//----------------------------------------------------------------------------
void pTouchKeyb::saveData()
{
  UINT len = SendMessage(*Edi[0], WM_GETTEXTLENGTH, 0, 0);
  LPTSTR buff = new TCHAR[len + 1];
  GetWindowText(*Edi[0], buff, len + 1);
  SetWindowText(hwTarget, buff);
  delete []buff;

  SendMessage(hwTarget, EM_SETMODIFY, 1, 0);
  PWin* w = PWin::getWindowPtr(hwTarget);
  PVarEdit* ed = dynamic_cast<PVarEdit*>(w);
  if(ed)
    ed->setFocus(pAround::aTop);
}
//----------------------------------------------------------------------------
P_Body* pTouchKeyb::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      saveData();
      // fall through
    case ID_F2:
      if(isFlagSet(pfModal))
        EndDialog(getHandle(), IDOK);
      else
        DestroyWindow(getHandle());
      return 0;

    default:
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
LPCTSTR pTouchKeyb::findChildPage(int idBtn, int& idChild, int& pswLevel, bool& predefinite)
{
  return 0;
}
//----------------------------------------------------------------------------
void pTouchKeyb::refresh()
{
  return;
}
