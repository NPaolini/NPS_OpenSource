//-------------------- pEdit.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PEDIT_H_
#include "pEdit.h"
#endif
//-----------------------------------------------------------
//----------------------------------------------------------------------------
void PEdit::setFilter(PFilter* filter)
{
  if(filter != Filter) {
    delete Filter;
    Filter = filter;
    }

  if(Filter && getHandle())
    Filter->initCtrl(*this);
}
//-----------------------------------------------------------
bool PEdit::create()
{
  if(!PControl::create())
    return false;

  SendMessage(getHandle(), EM_SETLIMITTEXT, TextLen, 0);
  if(Filter)
    Filter->initCtrl(*this);

  PWin* par = getParent();
  do {
    if(par->isFlagSet(pfIsDialog/*pfModal*/))
      customPasteAndUndo = false;
    par = par->getParent();
    } while(par && customPasteAndUndo);

  return true;
}
//-----------------------------------------------------------
bool PEdit::evKeyUp(WPARAM& key)
{
  if(VK_DOWN == key || VK_RETURN == key) {
    up_click(VK_TAB);
    return true;
    }
  if(VK_UP == key) {
    up_click(VK_TAB);
    up_click(VK_SHIFT);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool PEdit::evKeyDown(WPARAM& key)
{
  if(VK_DOWN == key || VK_RETURN == key) {
    up_click(key);
    down_click(VK_TAB);
    return true;
    }

  if(VK_UP == key) {
    up_click(VK_UP);
    down_click(VK_SHIFT);
    down_click(VK_TAB);
    return true;
    }
  if(GetKeyState(VK_CONTROL)& 0x8000) {
    switch(key) {
      case _T('c'):
      case _T('C'):
        SendMessage(*this, WM_COPY, 0, 0);
//        return true;
        break;
      case _T('v'):
      case _T('V'):
        if(customPasteAndUndo)
          SendMessage(*this, WM_PASTE, 0, 0);
//        return true;
        break;
      case _T('x'):
      case _T('X'):
        SendMessage(*this, WM_CUT, 0, 0);
//        return true;
        break;
      case _T('z'):
      case _T('Z'):
        if(customPasteAndUndo)
          SendMessage(*this, WM_UNDO, 0, 0);
//        return true;
        break;
      }
    }

  return false;
}
//-----------------------------------------------------------
bool PEdit::evChar(WPARAM& key)
{
  if(Filter && !Filter->accept(key, *this))
    return true;

  if(VK_RETURN == key) {
    click(VK_TAB);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
LRESULT PEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KILLFOCUS:
      if(Filter)
        Filter->onKillFocus(*this);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
int PEdit::verifyKey()
{
  if(GetKeyState(VK_RETURN)& 0x8000)
    return VK_RETURN;
  if(GetKeyState(VK_DOWN)& 0x8000)
    return VK_DOWN;
  if(GetKeyState(VK_UP)& 0x8000)
    return VK_UP;
//  if(GetKeyState(VK_CONTROL)& 0x8000)
//    return VK_CONTROL;

  return baseClass::verifyKey();
}
//-----------------------------------------------------------
