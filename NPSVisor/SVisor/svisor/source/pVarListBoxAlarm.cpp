//------ pVarListBoxAlarm.cpp -----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarListBoxAlarm.h"
#include "p_basebody.h"
#include "mainClient.h"
#include "p_util.h"
#include "currAlrm.h"
#include "p_name.h"
#include "1.h"
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
#define DEF_STYLE LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_CHILD | \
          WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP
//----------------------------------------------------------------------------
pVarListBoxAlarm::pVarListBoxAlarm(P_BaseBody* owner, uint id, pAround& Around) :
    baseClass(owner, id, PRect(0, 0, 10, 10), 0, 0),
    base_Active(owner, id, Around), totElem(0)
{
  Attr.style |= LBS_OWNERDRAWVARIABLE | DEF_STYLE;
}
//----------------------------------------------------------------------------
pVarListBoxAlarm::~pVarListBoxAlarm()
{
  destroy();
}
//----------------------------------------------------------------------------
const pBaseListAlarm::infoColorLB* pVarListBoxAlarm::getColor(colorRef ix)
{
  switch(ix) {
    case crBase:
      return &cBase;
    case crEvent:
      return &cEvent;
    case crActive:
      return &cActive;
    case crAcknoledged:
      return &cAcknoledged;
    case crAcknoledgedEvent:
      return &cAcknoledgedEvent;
    }
  return 0;
}
//----------------------------------------------------------------------------
void pVarListBoxAlarm::calcWidth()
{
  for(int i = 0; i < totElem; ++i)
    if(tWidth[i] < 2)
      ++tWidth[i];
  SetTabStop(totElem, tWidth);
  for(int i = 0; i < totElem; ++i)
    if(tWidth[i] < 12)
      setAlign(i, aCenter);
  recalcWidth();
}
//----------------------------------------------------------------------------
void pVarListBoxAlarm::formatHeaderAlarm(uint idHeadTitle)
{
  PVect<LPCTSTR> txt;
  base_Active::formatHeaderAlarm(txt, tWidth, idHeadTitle);
  totElem = txt.getElem();
#if 0
  ZeroMemory(tWidth, sizeof(tWidth));
#else
  int cx = getSizeFont().cx;
  for(int i = 0; i < totElem; ++i) {
    tWidth[i] *= SIMUL_DEC_FONT;
    tWidth[i] /= cx;
    }
/*
  SetTabStop(nElem, tWidth);
  recalcWidth();
*/
#endif
  flushPAV(txt);
}
//----------------------------------------------------------------------------
#define SEP TAB
//----------------------------------------------------------------------------
#define D_ID_PERIF      3
#define D_ID_STAT       3 // stato
#define D_ID_AL_GRP     3 // gruppo
#define D_ID_AL_FLT     3 // filtro
//#define D_ID_COD_ALARM  8 // allarme, composto da id + (addr, bit)
#define D_ID_COD_ALARM 16 // allarme, composto da id + (addr, bit)
#define D_DESCR_AL_GRP 20 // descrizione gruppo
#define D_DATE         11
#define D_TIME          9
#define D_DESCR_ALARM 200
#define D_DESCR_ALARM_LV 50
//----------------------------------------------------------------------------
#define FILL__(a, b, l) fillStr(a, b, l); a[l-1] = 0
#define SPACE__(a, l) FILL__(a, _T(' '), l)
#define ALL_DIM (D_ID_AL_FLT + D_ID_PERIF + D_ID_AL_GRP + D_ID_COD_ALARM + D_DESCR_AL_GRP + D_DATE + D_TIME + D_DESCR_ALARM + D_ID_STAT + 10)
//----------------------------------------------------------------------------
void pVarListBoxAlarm::formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat)
{
  PVect<LPCTSTR> items;
  base_Active::formatAlarm(items, fia, st, stat);

  int nElem = items.getElem();
  if(!nElem)
    return;
  TCHAR buff[ALL_DIM * 2] = _T("");
  SPACE__(buff, SIZE_A(buff) - 1);
  LPTSTR p = buff;
  int len = 0;
  for(int i = 0; i < nElem; ++i) {
    int l = _tcslen(items[i]);
    if(tWidth[i] < l)
      tWidth[i] = l;
    len += l + 1;
    if(len >= SIZE_A(buff))
      break;
    copyStr(p, items[i], l);
    p += l;
    *p++ = SEP;
    }
  *p = 0;
  int curr = SendMessage(*this, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(*this, LB_SETITEMDATA, curr, (LPARAM)(fia.makeFullIdAlarm()));
  flushPAV(items);
}
//----------------------------------------------------------------------------
extern bool getUse64Filter();
#ifndef STEP_GRP_ALARM_BITS
  #define STEP_GRP_ALARM_BITS 10
#endif
//----------------------------------------------------------------------------
bool pVarListBoxAlarm::create()
{
  const pBaseListAlarm::infoColorLB* ic = getColor(crBase);
  SetColor(ic->fgN, ic->bgN);
  SetColorSel(ic->fgS, ic->bgS);
  if(!baseClass::create())
    return false;
  setIntegralHeight();
  return true;
}
//----------------------------------------------------------------------------
int pVarListBoxAlarm::getCount()
{
  return SendMessage(*this, LB_GETCOUNT, 0, 0);
}
//----------------------------------------------------------------------------
int pVarListBoxAlarm::getCurrSel()
{
  return SendMessage(*this, LB_GETCURSEL, 0, 0);
}
//----------------------------------------------------------------------------
void pVarListBoxAlarm::setCurrSel(int sel)
{
  SendMessage(*this, LB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
void pVarListBoxAlarm::removeAllItem()
{
  SendMessage(*this, LB_RESETCONTENT, 0, 0);
}
//----------------------------------------------------------------------------
LRESULT pVarListBoxAlarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONDBLCLK:
      handleLBDblClick();
      break;
    }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pVarListBoxAlarm::handleLBDblClick()
{
  int sel = getCurrSel();
  if(sel >= 0) {
    int id = (int)SendMessage(*this, LB_GETITEMDATA, sel, 0);
    currAlarm *Al = getGestAlarm();
    Al->remove(id);
    }
}
//----------------------------------------------------------------------------
int pVarListBoxAlarm::verifyKey()
{
  if(GetKeyState(VK_RETURN)& 0x8000)
    handleLBDblClick();
  return 0;
}
//----------------------------------------------------------------------------
#define CHAR_INIT_ALARM _T('1')
#define CHAR_END_ALARM  _T('7')
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool pVarListBoxAlarm::evKeyDown(WPARAM& key)
{
  return evSetAroundFocus(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
HPEN pVarListBoxAlarm::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  long id = (long)drawInfo.itemData;
  currAlarm *Al = getGestAlarm();
  fullInfoAlarm idAlarm;
  FILETIME ft;
  currAlarm::alarmStat cStat;
  if(!Al->find(id, &idAlarm, &ft, cStat))
    return 0;

  TCHAR cGrp = idAlarm.idGrp + _T('0');
  const pBaseListAlarm::infoColorLB* ic = 0;

  switch(cStat) {
  // se allarme attivo
    case currAlarm::asActive:
      // se non è un evento, usa le impostazioni di base
      if(cGrp >= (CHAR_INIT_ALARM + getWarningLevelFilterAlarm()) && cGrp <= CHAR_END_ALARM)
        ic = getColor(crActive);
      else
        ic = getColor(crEvent);
      break;

    default:
    case currAlarm::asInactive:
      ic = getColor(crBase);
      break;

    case currAlarm::asActiveRecognized:
      if(cGrp >= (CHAR_INIT_ALARM + getWarningLevelFilterAlarm()) && cGrp <= CHAR_END_ALARM)
        ic = getColor(crAcknoledged);
      else
        ic = getColor(crAcknoledgedEvent);
      break;
    }

  COLORREF cText;
  COLORREF cBkg;
  if((ODS_SELECTED  | ODS_FOCUS) & drawInfo.itemState) {
    cText = ic->fgS;
    cBkg = ic->bgS;
    }
  else {
    cText = ic->fgN;
    cBkg = ic->bgN;
    }

  PRect Rect(drawInfo.rcItem);

  SetBkColor(hDC, cBkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, cText);
  HPEN pen = CreatePen(PS_SOLID, 1, cText);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//----------------------------------------------------------------------------
bool pVarListBoxAlarm::evChar(WPARAM& key)
{
  if(VK_RETURN == key)
    PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, LBN_DBLCLK), (LPARAM)getHandle());
  return baseClass::evChar(key);
}
//----------------------------------------------------------------------------
