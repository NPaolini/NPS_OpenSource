//-------------------- pmanageBtnSwitch.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pmanageBtnSwitch.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
bool manageBtnSwitch::drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis)
{
  if(!(dis->itemState & ODS_SELECTED)) {
    if(ON == Status)
      dis->itemState |= ODS_SELECTED;
    }
  else if(!pressed) {
    if(OFF == Status)
      dis->itemState &= ~ODS_SELECTED;
    }
  else
    return false;
  return true;
}
//-----------------------------------------------------------
void manageBtnSwitch::setState(manageBtnSwitch::state stat)
{
  if(stat == Status)
    return;
  Status = stat;
  Owner->invalidate();
}
//-----------------------------------------------------------
void manageBtnSwitch::switchState()
{
  switch(Status) {
    case ON:
      setState(OFF);
      break;
    case OFF:
      setState(ON);
      break;
    }
}
//-----------------------------------------------------------
bool manageBtnSwitch::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& res)
{
  switch(message) {
    case WM_LBUTTONDOWN:
      pressed = true;
      switchState();
      break;
    // verifica se si rilascia il pulsante fuori dell'area, allora non è valido il click
    case WM_LBUTTONUP:
      pressed = false;
      do {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ClientToScreen(*Owner, &pt);
        if(!Owner->isInWindow(pt)) {
          switchState();
          Owner->invalidate();
          }
        } while(false);
      break;
    }
  return false;
}
//-----------------------------------------------------------
void manageBtnSwitch::drawDisabled(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(ON == getState())
    lpDIS->itemState |= ODS_SELECTED;
}
//--------------------------------------------------------------------------
#define PREVIOUS_DOWN(a) toBool(((DWORD)(a)) & ( 1 << 30))
//-----------------------------------------------------------
bool manageBtnSwitch::preProcessMsg(MSG& msg)
{
  return setPushed(msg.message, (uint)msg.wParam, PREVIOUS_DOWN(msg.lParam));
}
//-----------------------------------------------------------
//#define SEND_MSG SendMessage
#define SEND_MSG PostMessage
//-----------------------------------------------------------
bool manageBtnSwitch::setPushed(uint msg, uint key, bool previous)
{
  if(WM_KEYDOWN == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      if(!previous) {
        PostMessage(*Owner, WM_LBUTTONDOWN, 0, 0);
        }
      return true;
      }
    }
  else if(WM_KEYUP == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      pressed = false;
      SEND_MSG(*Owner->getParent(), WM_COMMAND, MAKEWPARAM(Owner->Attr.id, BN_CLICKED), (LPARAM)(HWND)*Owner);
      Owner->invalidate();
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
