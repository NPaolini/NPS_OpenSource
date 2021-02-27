//-------------------- POwnBtnSelect.cpp --------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnSelect.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnBtnSelect::setState(manageBtnSwitch::state stat)
{
  if(stat == getState())
    return;

  baseClass::setState(stat);

  if(manageBtnSwitch::ON == stat)
    unselectOther();
}
//-----------------------------------------------------------
void POwnBtnSelect::switchState()
{
  baseClass::switchState();

  if(manageBtnSwitch::ON == getState())
    unselectOther();
}
//-----------------------------------------------------------
void POwnBtnSelect::unselectOther()
{
  int nElem = Group->Group.getElem();
  const PVect<POwnBtn*>& cGrp = Group->Group;
  for(int i = 0; i < nElem; ++i) {
    if(cGrp[i] == this)
      continue;
    manageBtnSwitch::state stat = ((POwnBtnSelect*)cGrp[i])->getState();
    if(stat != manageBtnSwitch::OFF) {
      ((POwnBtnSelect*)cGrp[i])->setState(manageBtnSwitch::OFF);
      InvalidateRect(*cGrp[i], 0, 1);
      }
    }
}
//-----------------------------------------------------------
void POwnBtnSelect::addToGroup()
{
  int nElem = Group->Group.getElem();
  Group->Group[nElem] = this;
  if(!nElem)
    Group->lastSel = this;
}
//-----------------------------------------------------------
void POwnBtnSelect::removeFromGroup()
{
  uint nElem = Group->Group.getElem();
  const PVect<POwnBtn*>& cGrp = Group->Group;
  for(uint i = 0; i < nElem; ++i) {
    if(cGrp[i] == this) {
      Group->Group.remove(i);
      break;
      }
    }
  if(!Group->Group.getElem())
    delete Group;
}
//-----------------------------------------------------------
LRESULT POwnBtnSelect::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(isEnabledCapture()) {
        LRESULT result = baseClass::windowProc(hwnd, message, wParam, lParam);
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ClientToScreen(*this, &pt);
        if(!isInWindow(pt)) {
          if(Group->lastSel) {
            ((POwnBtnSelect*)Group->lastSel)->setState(manageBtnSwitch::ON);
            InvalidateRect(*Group->lastSel, 0, 0);
            }
          }
        else
          Group->lastSel = this;
        return result;
        }
      break;

    case BM_SETSTATE:
      if(wParam) {
        if(manageBtnSwitch::ON != getState())
          return 0;
        else
          unselectOther();
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
