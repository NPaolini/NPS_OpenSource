//-------------------- POwnBtnStdSwitch.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnStdSwitch.h"
//-----------------------------------------------------------
manageBtnSwitch::state POwnBtnStdSwitch::getState() const
{
  return ManagedSwitch ? ManagedSwitch->getState() : manageBtnSwitch::OFF;
}
//-----------------------------------------------------------
void POwnBtnStdSwitch::setState(manageBtnSwitch::state stat)
{
  if(ManagedSwitch)
    ManagedSwitch->setState(stat);
}
//-----------------------------------------------------------
void POwnBtnStdSwitch::switchState()
{
  if(ManagedSwitch)
    ManagedSwitch->switchState();
}
//-----------------------------------------------------------
bool POwnBtnStdSwitch::drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis)
{
  if(ManagedSwitch)
    ManagedSwitch->drawItem(idCtrl, dis);
  return baseClass::drawItem(idCtrl, dis);
}
//-----------------------------------------------------------
LRESULT POwnBtnStdSwitch::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT res = 0;
  if(ManagedSwitch) {
    if(!ManagedSwitch->windowProc(hwnd, message, wParam, lParam, res))
      res = baseClass::windowProc(hwnd, message, wParam, lParam);
    }
  else
    res = baseClass::windowProc(hwnd, message, wParam, lParam);
  return res;
}
//-----------------------------------------------------------
void POwnBtnStdSwitch::drawDisabled(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(ManagedSwitch)
    ManagedSwitch->drawDisabled(hdc, lpDIS);
  baseClass::drawDisabled(hdc, lpDIS);
}
//--------------------------------------------------------------------------
bool POwnBtnStdSwitch::preProcessMsg(MSG& msg)
{
  if(ManagedSwitch && ManagedSwitch->preProcessMsg(msg))
    return true;
  return baseClass::preProcessMsg(msg);
}
//--------------------------------------------------------------------------
bool POwnBtnStdSwitch::setPushed(uint msg, uint key, bool previous)
{
  return ManagedSwitch && ManagedSwitch->setPushed(msg, key, previous);
}
//-----------------------------------------------------------
