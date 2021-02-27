//------ PVarBtnStd.cpp ------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "PVarBtnStd.h"
#include "p_basebody.h"
#include "pedit.h"
#include "p_util.h"
#include "mainClient.h"
#include "id_Btn.h"
#include "1.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void POwnBtnStdFocus::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
void POwnBtnStdFocus::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//-----------------------------------------------------------
manageBtnSwitch::state POwnBtnStdSwitchFocus::getState() const
{
  return ManagedSwitch ? ManagedSwitch->getState() : manageBtnSwitch::OFF;
}
//-----------------------------------------------------------
void POwnBtnStdSwitchFocus::setState(manageBtnSwitch::state stat)
{
  if(ManagedSwitch)
    ManagedSwitch->setState(stat);
}
//-----------------------------------------------------------
void POwnBtnStdSwitchFocus::switchState()
{
  if(ManagedSwitch)
    ManagedSwitch->switchState();
}
//-----------------------------------------------------------
inline bool isBitsEqu(DWORD check, DWORD bits)
{
  return (check & bits) == check;
}
//-----------------------------------------------------------
bool POwnBtnStdSwitchFocus::drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis)
{
  if(ManagedSwitch)
    ManagedSwitch->drawItem(idCtrl, dis);
  bool success = baseClass::drawItem(idCtrl, dis);
/*
  if(success && isThemed() && isBitsEqu(ODS_SELECTED | ODS_FOCUS, dis->itemState)) {
    DrawThemeEdge(          HTHEME hTheme,
        HDC hdc,
        int iPartId,
        int iStateId,
        const RECT *pDestRect,
        UINT uEdge,
        UINT uFlags,
        RECT *pContentRect
        );
//    drawFocusAfter(dis->hDC, dis);
    }
*/
  return success;
}
//-----------------------------------------------------------
LRESULT POwnBtnStdSwitchFocus::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
void POwnBtnStdSwitchFocus::drawDisabled(HDC hdc, LPDRAWITEMSTRUCT lpDIS)
{
  if(ManagedSwitch)
    ManagedSwitch->drawDisabled(hdc, lpDIS);
  baseClass::drawDisabled(hdc, lpDIS);
}
//--------------------------------------------------------------------------
bool POwnBtnStdSwitchFocus::preProcessMsg(MSG& msg)
{
  if(ManagedSwitch && ManagedSwitch->preProcessMsg(msg))
    return true;
  return baseClass::preProcessMsg(msg);
}
//--------------------------------------------------------------------------
bool POwnBtnStdSwitchFocus::setPushed(uint msg, uint key, bool previous)
{
  return ManagedSwitch && ManagedSwitch->setPushed(msg, key, previous);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnBtnStdSelectFocus::setState(manageBtnSwitch::state stat)
{
  if(stat == getState())
    return;

  baseClass::setState(stat);

  if(manageBtnSwitch::ON == stat)
    unselectOther();
}
//-----------------------------------------------------------
void POwnBtnStdSelectFocus::switchState()
{
  baseClass::switchState();

  if(manageBtnSwitch::ON == getState())
    unselectOther();
}
//-----------------------------------------------------------
void POwnBtnStdSelectFocus::addToGroup()
{
  int nElem = Group->Group.getElem();
  Group->Group[nElem] = this;
  if(!nElem)
    Group->lastSel = this;
}
//-----------------------------------------------------------
void POwnBtnStdSelectFocus::removeFromGroup()
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
LRESULT POwnBtnStdSelectFocus::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(isEnabledCapture()) {
        LRESULT result = baseClass::windowProc(hwnd, message, wParam, lParam);
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ClientToScreen(*this, &pt);
        if(!isInWindow(pt)) {
          if(Group->lastSel) {
            ((POwnBtnStdSelectFocus*)Group->lastSel)->setState(manageBtnSwitch::ON);
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
//----------------------------------------------------------------------------
void POwnBtnStdSelectFocus::unselectOther()
{
  PVect<uint> idsBtn;
  int nElem = Group->Group.getElem();
  const PVect<POwnBtn*>& cGrp = Group->Group;
  for(int i = 0; i < nElem; ++i) {
    if(cGrp[i] == this)
      continue;
    manageBtnSwitch::state stat = ((POwnBtnStdSelectFocus*)cGrp[i])->getState();
    if(stat != manageBtnSwitch::OFF) {
      idsBtn[idsBtn.getElem()] = cGrp[i]->Attr.id;
      ((POwnBtnStdSelectFocus*)cGrp[i])->setState(manageBtnSwitch::OFF);
      InvalidateRect(*cGrp[i], 0, 1);
      }
    }
  nElem = idsBtn.getElem();
  if(nElem) {
    P_BaseBody *bb = getParentWin<P_BaseBody>(getParent());
    if(bb) {
      for(int i = 0; i < nElem; ++i)
        bb->simulActionBtn(idsBtn[i]);
      bb->simulActionBtn(Attr.id);
      }
    }
}
//--------------------------------------------------------------------------
#define PREVIOUS_DOWN(a) toBool(((DWORD)(a)) & ( 1 << 30))
//-----------------------------------------------------------
bool POwnBtnStdPress::preProcessMsg(MSG& msg)
{
  if(setPushed(msg.message, msg.wParam, PREVIOUS_DOWN(msg.lParam)))
    return true;
  return baseClass::preProcessMsg(msg);
}
//-----------------------------------------------------------
//#define SEND_MSG SendMessage
#define SEND_MSG PostMessage
//-----------------------------------------------------------
bool POwnBtnStdPress::setPushed(uint msg, uint key, bool previous)
{
  if(WM_KEYDOWN == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      if((!isPressed() || manageBtnSwitch::OFF == getState()) && !previous) {
        setPressed(true);
        verifyFocus();
        setState(manageBtnSwitch::ON);
        SEND_MSG(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, BN_CLICKED), (LPARAM)getHandle());
        }
      return true;
      }
    }
  else if(WM_KEYUP == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      if(isPressed() || manageBtnSwitch::ON == getState()) {
        setPressed(false);
        verifyFocus();
        setState(manageBtnSwitch::OFF);
        SEND_MSG(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, BN_CLICKED), (LPARAM)getHandle());
        }
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
inline
void POwnBtnStdPress::verifyFocus()
{
  if(GetFocus() != getHandle())
    SetFocus(*this);
}
//-----------------------------------------------------------
LRESULT POwnBtnStdPress::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(setPushed(WM_KEYUP, VK_SPACE, 0))
        return 0;
      break;
    case WM_LBUTTONDOWN:
      if(setPushed(WM_KEYDOWN, VK_SPACE, 0))
        return 0;
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
