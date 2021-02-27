//------ PVarListBox.cpp -----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarListBox.h"
#include "p_basebody.h"
#include "p_util.h"
//----------------------------------------------------------------------------
#define DEF_STYLE LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_CHILD | \
          WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP
//----------------------------------------------------------------------------
PVarListBox::PVarListBox(P_BaseBody* owner, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst) :
    PListBox(owner, id, rect, textlen, hinst), baseActive(owner, id),
    FzCallBack(0), FzCallBackWinProc(0), FzCallBacPreProcess(0)
{
  Attr.style = style;
  Attr.style |= DEF_STYLE;
  setRect(rect);
}
//----------------------------------------------------------------------------
PVarListBox::~PVarListBox()
{
  destroy();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
HWND PVarListBox::setListBoxCallBack(infoCallBackSend* fzInfo, uint forWinProc)
{
  switch(forWinProc) {
    case 1:
      FzCallBackWinProc = fzInfo->fz;
      break;
    case 2:
      FzCallBacPreProcess = fzInfo->fz;
      break;
    default:
      FzCallBack = fzInfo->fz;
      break;
    }
  callBackOwner = fzInfo->objOwner;
  return *this;
}
//----------------------------------------------------------------------------
void PVarListBox::setVisibility(bool set)
{
  baseActive::setVisibility(set);
//  if(!set)
    invalidateWithBorder();
}
//----------------------------------------------------------------------------
LRESULT PVarListBox::windowProcStd(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KEYDOWN:
      if(evKeyDown(wParam))
        return 0;
    }
  return PListBox::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
inline DWORD isSet(DWORD v, uint bit)
{
  return v & (1 << bit);
}
//----------------------------------------------------------------------------
inline DWORD setBit(DWORD v, uint bit)
{
  return v | (1 << bit);
}
//----------------------------------------------------------------------------
inline DWORD resetBit(DWORD v, uint bit)
{
  return v & ~(1 << bit);
}
//----------------------------------------------------------------------------
bool PVarListBox::preProcessMsgStd(MSG& msg)
{
  return PListBox::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
bool PVarListBox::preProcessMsg(MSG& msg)
{
  if(!FzCallBacPreProcess)
    return preProcessMsgStd(msg);
  CallBackListBox_PreProcessMsg stru = { msg };
  infoCallBackListBox full = {
    icbPreProcessMsg,
    1 << icbfBefore,
    0,
    *this,
    callBackOwner,
    &stru
    };
  bool result = FzCallBacPreProcess(&full);
  if(!result)
    return preProcessMsgStd(msg);

  // se richiede che sia eseguito il metodo base
  if(isSet(full.flag, icbfCanExec)) {
    bool res = preProcessMsgStd(msg);
    // se richiede di essere richiamata dopo il metodo base
    if(isSet(full.flag, icbfNeedAfter)) {
      full.result = res;
      full.flag = 0;
      setBit(full.flag, icbfValid);
      result = FzCallBackWinProc(&full);
      if(!result || !isSet(full.flag, icbfValid))
        return res;
      return toBool(full.result);
      }
    return res;
    }

  if(isSet(full.flag, icbfValid))
    return toBool(full.result);
  // ogni altra cosa dovrebbe essere un errore ...
  return preProcessMsgStd(msg);
}
//----------------------------------------------------------------------------
LRESULT PVarListBox::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(!FzCallBackWinProc)
    return windowProcStd(hwnd, message, wParam, lParam);

  CallBackListBox_WindowProc stru = { hwnd, message, wParam, lParam };
  infoCallBackListBox full = {
    icbcWindowProc,
    1 << icbfBefore,
    0,
    *this,
    callBackOwner,
    &stru
    };
  bool result = FzCallBackWinProc(&full);
  if(!result)
    return windowProcStd(hwnd, message, wParam, lParam);

  // se richiede che sia eseguito il metodo base
  if(isSet(full.flag, icbfCanExec)) {
    LRESULT res = windowProcStd(hwnd, message, wParam, lParam);
    // se richiede di essere richiamata dopo il metodo base
    if(isSet(full.flag, icbfNeedAfter)) {
      full.result = res;
      full.flag = 0;
      setBit(full.flag, icbfValid);
      result = FzCallBackWinProc(&full);
      if(!result || !isSet(full.flag, icbfValid))
        return res;
      return full.result;
      }
    return res;
    }

  if(isSet(full.flag, icbfValid))
    return full.result;
  // ogni altra cosa dovrebbe essere un errore ...
  return windowProcStd(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static
bool isFirstRow(HWND hwnd)
{
  int currSel = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
  if(currSel < 0)
    currSel = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
  return currSel <= 0;
}
//----------------------------------------------------------------------------
static
bool isLastRow(HWND hwnd)
{
  int currSel = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
  int count =  SendMessage(hwnd, LB_GETCOUNT, 0, 0);
  if(currSel < 0) {
    if(count)
      return false;
    return true;
    }

  return currSel == count - 1;
}
//----------------------------------------------------------------------------
bool PVarListBox::evSetAroundFocus(UINT key)
{
  switch(key) {
    case VK_UP:
      if(isFirstRow(*this)) {
        Around.setFocus(pAround::aTop);
        return true;
        }
      break;
    case VK_DOWN:
      if(isLastRow(*this)) {
        Around.setFocus(pAround::aBottom);
        return true;
        }
      break;

    case VK_LEFT:
      Around.setFocus(pAround::aLeft);
      return true;

    case VK_RIGHT:
      Around.setFocus(pAround::aRight);
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool PVarListBox::setFocus(pAround::around where)
{
  uint key[] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };
  return evSetAroundFocus(key[where]);
}
//----------------------------------------------------------------------------
bool PVarListBox::evKeyDown(WPARAM& key)
{
  return evSetAroundFocus(key);
}
//----------------------------------------------------------------------------
HPEN PVarListBox::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!FzCallBack)
    return PListBox::setColor(hDC, drawInfo);
  CallBackListBox_setColor stru = { hDC, &drawInfo, Buff };
  infoCallBackListBox full = {
    icbcSetColor,
    1 << icbfBefore,
    0,
    *this,
    callBackOwner,
    &stru
    };
  bool result = FzCallBack(&full);
  if(!result)
    return PListBox::setColor(hDC, drawInfo);

  // se richiede che sia eseguito il metodo base
  if(isSet(full.flag, icbfCanExec)) {
    HPEN hpen = PListBox::setColor(hDC, drawInfo);
    // se richiede di essere richiamata dopo il metodo base
    if(isSet(full.flag, icbfNeedAfter)) {
      full.result = (LRESULT)hpen;
      full.flag = 0;
      setBit(full.flag, icbfValid);
      result = FzCallBack(&full);
      if(!result || !isSet(full.flag, icbfValid))
        return hpen;
      return (HPEN)full.result;
      }
    return hpen;
    }

  if(isSet(full.flag, icbfValid))
    return (HPEN)full.result;
  // ogni altra cosa dovrebbe essere un errore ...
  return PListBox::setColor(hDC, drawInfo);
}
//----------------------------------------------------------------------------
void PVarListBox::paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!FzCallBack) {
    PListBox::paintTxt(hDC, x, y, buff, ix, drawInfo);
    return;
    }
  CallBackListBox_paintTxt stru = { hDC, x, y, buff, ix, &drawInfo };
  infoCallBackListBox full = {
    icbcPaintTxt,
    1 << icbfBefore,
    0,
    *this,
    callBackOwner,
    &stru
    };
  bool result = FzCallBack(&full);
  if(!result) {
    PListBox::paintTxt(hDC, x, y, buff, ix, drawInfo);
    return;
    }

  // se richiede che sia eseguito il metodo base
  if(isSet(full.flag, icbfCanExec)) {
    PListBox::paintTxt(hDC, x, y, buff, ix, drawInfo);
    // se richiede di essere richiamata dopo il metodo base
    if(isSet(full.flag, icbfNeedAfter)) {
      full.flag = 0;
      FzCallBack(&full);
      }
    return;
    }
  // ogni altra cosa dovrebbe essere un errore ...
  PListBox::paintTxt(hDC, x, y, buff, ix, drawInfo);
}
//----------------------------------------------------------------------------
void PVarListBox::paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!FzCallBack) {
    PListBox::paintCustom(hDC, r, ix, drawInfo);
    return;
    }
  RECT rect = *r;
  CallBackListBox_paintCustom stru = { hDC, &rect, Buff, ix, &drawInfo };
  infoCallBackListBox full = {
    icbcPaintCustom,
    1 << icbfBefore,
    0,
    *this,
    callBackOwner,
    &stru
    };
  bool result = FzCallBack(&full);
  if(!result) {
    PListBox::paintCustom(hDC, r, ix, drawInfo);
    return;
    }

  // se richiede che sia eseguito il metodo base
  if(isSet(full.flag, icbfCanExec)) {
    PListBox::paintCustom(hDC, r, ix, drawInfo);
    // se richiede di essere richiamata dopo il metodo base
    if(isSet(full.flag, icbfNeedAfter)) {
      full.flag = 0;
      FzCallBack(&full);
      }
    return;
    }
  // ogni altra cosa dovrebbe essere un errore ...
  PListBox::paintCustom(hDC, r, ix, drawInfo);
}
//----------------------------------------------------------------------------
