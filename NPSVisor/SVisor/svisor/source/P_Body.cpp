//------ P_Body.cpp ----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "id_btn.h"
#include "p_body.h"
#include "hdrmsg.h"
#include "password.h"
#include "prfData.h"
#include "p_util.h"
#include "p_date.h"
#include "p_heap.h"
#include "PVarEdit.h"
#include "PVarBtn.h"
#include "pTraspBitmap.h"
#include "pTextPanel.h"
//----------------------------------------------------------------------------
DWORD getGlobalTimeCycle()
{
  static DWORD timeCycle = (DWORD)-1;
  if((DWORD)-1 == timeCycle) {
    timeCycle = 0;
    LPCTSTR p = getString(ID_TIMER_APP);
    if(p) {
      p = findNextParamTrim(p);
      if(p)
        timeCycle = _ttoi(p);
      }
    }
  return timeCycle;
}
//----------------------------------------------------------------------------
// tempo in secondi
#define INIT_TIMER_SAVE_SEC  60
//----------------------------------------------------------------------------
// tempo in tick del timer
#define INIT_TIMER_SAVE_TICK ((INIT_TIMER_SAVE_SEC * 1000) / TIME_SLICE)
//----------------------------------------------------------------------------
P_Body::P_Body(int idParent, PWin* parent, uint resId, HINSTANCE hInstance)
:
    P_BaseBody(parent, resId, hInstance), idInitMenu(ID_INIT_MENU), ParentID(idParent),
    readOnly(false), Modal(0)
{
}
//----------------------------------------------------------------------------
P_Body::~P_Body()
{
  uint nElem = modeless.getElem();
  for(uint i = 0; i < nElem; ++i) {
    modeless[i]->sendListValue(false);
    modeless[i]->refresh();
    }
  destroy();
}
//----------------------------------------------------------------------------
void P_Body::destroyModal()
{
  if(Modal && Modal->getHandle())
    DestroyWindow(*Modal);
  Modal = 0;
}
//----------------------------------------------------------------------------
void P_Body::setReady(bool first)
{
  baseClass::setReady(first);
  ShowWindow(*this, SW_SHOWNORMAL);
}
//----------------------------------------------------------------------------
void P_Body::enableBtn(int idBtn, bool enable)
{
  Par->enableBtn(idBtn, enable);
}
//----------------------------------------------------------------------------
bool P_Body::isEnabledBtn(uint idBtn)
{
  LPCTSTR p = getPageString(idBtn);
  return toBool(p);
}
//----------------------------------------------------------------------------
smartPointerConstString P_Body::getTitle()
{
  bool needDelete;
  LPCTSTR p = getTitle(needDelete);

  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
LPCTSTR P_Body::getTitle(bool& needDelete)
{
  if(!sStr.getNumElem()) {
    TCHAR path[_MAX_PATH];
    getFileStr(path);
    fillPageStr(path);
    }

  return getStringByLang(ID_TITLE, needDelete);
}
//----------------------------------------------------------------------------
setOfString& P_Body::getInfo(long &id)
{
  if(!sStr.getNumElem()) {
    TCHAR path[_MAX_PATH];
    getFileStr(path);
    fillPageStr(path);
    }

  id = idInitMenu;
  return sStr;
}
//----------------------------------------------------------------------------
void P_Body::postOpenModal(LPCTSTR name)
{
  PostMessage(*this, WM_POST_NAME_MODAL_BODY, 0, (LPARAM) name);
}
//----------------------------------------------------------------------------
void P_Body::postOpenKeyboard(PEdit* pCtrl)
{
  PVarEdit* ed = dynamic_cast<PVarEdit*>(pCtrl);
  LPCTSTR name = 0;
  if(ed) {
    uint type = ed->getType();
    if(prfData::tStrData == type || prfData::tNoData == type) {
      LPCTSTR p = getString(ID_USE_TOUCH_KEYB);
      name = findNextParam(p);
      }
    else {
      name = getString(ID_NUMB_TOUCH_KEYB);
      if(!name) {
        LPCTSTR p = getString(ID_USE_TOUCH_KEYB);
        name = findNextParam(p);
        }
      }
    }
  else {
    LPCTSTR p = getString(ID_USE_TOUCH_KEYB);
    name = findNextParam(p);
    }
  if(name && *name)
    PostMessage(*this, WM_POST_OPEN_MODAL_KEYB, (WPARAM)pCtrl->getHandle(), (LPARAM) name);
}
//----------------------------------------------------------------------------
void P_Body::postOpenModeless(LPCTSTR name, bool open)
{
  PostMessage(*this, WM_POST_NAME_MODELESS_BODY, open, (LPARAM) name);
}
//----------------------------------------------------------------------------
void P_Body::setStatus(LPCTSTR msgStatus)
{
  Par->setStatus(msgStatus);
}
//----------------------------------------------------------------------------
void P_Body::refresh()
{
  if(Modal)
    Modal->refresh();
  int nElem = modeless.getElem();
  for(int i = 0; i < nElem; ++i)
    modeless[i]->refresh();

  P_BaseBody::refresh();

//  P_Body* bd = dynamic_cast<P_Body*>(getParent());
//  if(!bd)
    refreshBody();
  needRefresh = 0;
}
//----------------------------------------------------------------------------
void P_Body::request()
{
  if(Modal)
    Modal->makeListReqRead();
  int nElem = modeless.getElem();
  for(int i = 0; i < nElem; ++i)
    modeless[i]->makeListReqRead();

  P_BaseBody::request();
}
//----------------------------------------------------------------------------
void P_Body::refreshBody()
{
}
//----------------------------------------------------------------------------
void P_Body::releasePress()
{
  uint nElem = Btn.getElem();
  for(uint i = 0; i < nElem; ++i) {
      if(Btn[i]->needReleasePress())
        actionBtn(Btn[i]->getId(), false);
    }
}
//----------------------------------------------------------------------------
void P_Body::evPaint(HDC hdc, const PRect& rect)
{
  baseClass::evPaint(hdc, rect);
  if(this == getCurrBody()) {
//  if(Par == getParent()) {
    PTextAutoPanel* time = Par->getTimeBox();
    if(time) {
      PRect r = time->getRect();
      if(r.Intersec(rect))
        time->draw(hdc);
      }
    }
}
//----------------------------------------------------------------------------
P_Body* P_Body::runWithPsw(pfzMember fz, int level)
{
  PassWord psw;
  P_Body* result = 0;
  if(level == psw.getPsW(level, this)) {
    result = (this->*fz)();
    psw.restartTime(level);
    }
  return result;
}
//----------------------------------------------------------------------------
P_Body* getCurrBody()
{
  PWin* w = getAppl()->getMainWindow();
    // mainClient
  w = w->getFirstChild();
    // la prima delle tre sezioni
  PWin* first = w->getFirstChild();
  w = first;
  do {
    P_Body *body = dynamic_cast<P_Body*>(w);
    if(body)
      return body;
    } while((w = w->Next()) != first);
  return 0;
}
//----------------------------------------------------------------------------
bool P_Body::usePredefBtn(uint id)
{
  LPCTSTR p = getString(ID_USE_PREDEF_BTN);
  if(!p)
    return true;
  uint flag = _ttoi(p);
  switch(id) {
    case ID_F1:
      return toBool(flag & 1);
    case ID_F12:
      return toBool(flag & 2);
    }
  return false;
}
//----------------------------------------------------------------------------
void P_Body::sendListValue(bool onLoad)
{
  if(!getHandle())
    return;
  uint id = onLoad ? ID_SEND_VALUE_ON_OPEN : ID_SEND_VALUE_ON_CLOSE;
  LPCTSTR p = getPageString(id);
  while(p)
    p = sendValue(p);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void alignWithMonitor(PWin* owner, int& x, int& y, bool act)
{
  LPCTSTR p = getString(ID_RESOLUTION_BASE);
  p = findNextParamTrim(p);
  if(p) {
    int ixMon = _ttoi(p);
    switch(ixMon) {
      case 0:  // è il primo, nessuna azione
      case 10:
      default:
        return;
      case 20:
      case 30:
      case 40:
      case 130:
      case 140:
        break;
      }
    PWin* mw = getAppl()->getMainWindow();
    if(mw) {
      PRect r;
      GetWindowRect(*mw, r);
      x += r.left;
      y += r.top;
      if(act)
        SetWindowPos(*owner, 0, x, y, 0, 0, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
      }
    }
}
//----------------------------------------------------------------------------

