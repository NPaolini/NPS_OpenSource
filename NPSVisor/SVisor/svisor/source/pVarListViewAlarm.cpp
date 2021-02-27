//------ pVarListViewAlarm.cpp -----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarListViewAlarm.h"
#include "p_basebody.h"
#include "mainClient.h"
#include "p_util.h"
#include "currAlrm.h"
#include "p_name.h"
#include "1.h"
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
pVarListViewAlarm::pVarListViewAlarm(P_BaseBody* owner, uint id, pAround& Around) :
    baseClass(owner, id, PRect(0, 0, 10, 10), 0, 0),
    base_Active(owner, id, Around)
{
}
//----------------------------------------------------------------------------
pVarListViewAlarm::~pVarListViewAlarm()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pVarListViewAlarm::getColorBkg(COLORREF& c)
{
  c = cBase.bgN;
  return true;
}
//----------------------------------------------------------------------------
bool pVarListViewAlarm::getColorTextBkg(COLORREF& c)
{
  c = cBase.bgN;
  return true;
}
//----------------------------------------------------------------------------
bool pVarListViewAlarm::getColorText(COLORREF& c)
{
  c = cBase.fgN;
  return true;
}
//----------------------------------------------------------------------------
bool pVarListViewAlarm::getColorTextBkgSel(COLORREF& c)
{
  c = cBase.bgS;
  return true;
}
//----------------------------------------------------------------------------
bool pVarListViewAlarm::getColorTextSel(COLORREF& c)
{
  c = cBase.fgS;
  return true;
}
//----------------------------------------------------------------------------
void pVarListViewAlarm::formatHeaderAlarm(uint idHeadTitle)
{
  ListView_DeleteAllItems(*this);
  while(ListView_DeleteColumn(*this, 0)) {
    ;
    }
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
  lvc.fmt = LVCFMT_LEFT;

  int tWidth[MAX_SHOW_AL] = { 0 };

  PVect<LPCTSTR> txt;
  base_Active::formatHeaderAlarm(txt, tWidth, idHeadTitle);

  uint nElem = txt.getElem();
  for(uint i = 0; i < nElem; ++i) {
    lvc.pszText = (LPTSTR)txt[i];
    lvc.cx = tWidth[i];
    if(ListView_InsertColumn (*this, i, &lvc) < 0)
      break;
    }
  flushPAV(txt);
}
//----------------------------------------------------------------------------
void pVarListViewAlarm::formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat)
{
  LVITEM lvI;
  lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
  lvI.iItem = 65535;
  lvI.iSubItem = 0;
  lvI.lParam = (LPARAM)(fia.makeFullIdAlarm());
  lvI.state = 0;
  lvI.stateMask = -1;

  PVect<LPCTSTR> items;
  base_Active::formatAlarm(items, fia, st, stat);

  int nElem = items.getElem();
  if(!nElem)
    return;
  lvI.pszText = (LPTSTR)items[0];
  lvI.iItem = SendMessage(*this, LVM_INSERTITEM, 0, (LPARAM) &lvI);
  lvI.mask = LVIF_TEXT;
  ++lvI.iSubItem;

  for(int i = 1; i < nElem; ++i) {
    lvI.pszText =  (LPTSTR)items[i];
    SendMessage(*this, LVM_SETITEM, 0, (LPARAM) &lvI);
    ++lvI.iSubItem;
    }
  flushPAV(items);
}
//----------------------------------------------------------------------------
extern bool getUse64Filter();
#ifndef STEP_GRP_ALARM_BITS
  #define STEP_GRP_ALARM_BITS 10
#endif
//----------------------------------------------------------------------------
bool pVarListViewAlarm::create()
{
  Attr.style &= ~(LVS_SHOWSELALWAYS);
  Attr.style |= LVS_SINGLESEL;
  if(!baseClass::create())
    return false;
  SendMessage (*this, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  return true;
}
//----------------------------------------------------------------------------
int pVarListViewAlarm::getCount()
{
  return ListView_GetItemCount(*this);
}
//----------------------------------------------------------------------------
int pVarListViewAlarm::getCurrSel()
{
  return ListView_GetNextItem(*this, -1, LVNI_SELECTED);
}
//----------------------------------------------------------------------------
static void setFocusItem(HWND hwlist, int sel)
{
  LVITEM lvI;
  lvI.mask = LVIF_STATE;
  lvI.iItem = sel;
  lvI.state = LVIS_FOCUSED | LVIS_SELECTED;
  lvI.stateMask = -1;
  SendMessage(hwlist, LVM_SETITEM, 0, (LPARAM) &lvI);
  ListView_EnsureVisible(hwlist, sel, false);
}
//----------------------------------------------------------------------------
void pVarListViewAlarm::setCurrSel(int sel)
{
  setFocusItem(*this, sel);
}
//----------------------------------------------------------------------------
void pVarListViewAlarm::removeAllItem()
{
  ListView_DeleteAllItems(*this);
}
//----------------------------------------------------------------------------
LRESULT pVarListViewAlarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONDBLCLK:
      handleLBDblClick();
      break;
    }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pVarListViewAlarm::handleLBDblClick()
{
  int sel = ListView_GetNextItem(*this, -1, LVNI_SELECTED);

  if(sel >= 0) {
    LVITEM lvI;
    lvI.mask = LVIF_PARAM;
    lvI.iItem = sel;
    if(!ListView_GetItem(*this, &lvI))
      return;
    int id = (int)lvI.lParam;
    currAlarm *Al = getGestAlarm();
    Al->remove(id);
    }
}
//----------------------------------------------------------------------------
int pVarListViewAlarm::verifyKey()
{
  if(GetKeyState(VK_RETURN)& 0x8000)
    handleLBDblClick();
  return 0;
}
//----------------------------------------------------------------------------
#define CHAR_INIT_ALARM _T('1')
#define CHAR_END_ALARM  _T('7')
//----------------------------------------------------------------------------
bool pVarListViewAlarm::ProcessNotify(LPARAM lParam, LRESULT& res)
{
  LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) lParam;
  if(!lplvcd)
    return false;
  res = CDRF_DODEFAULT;
  switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_ITEM | CDDS_ITEMPREPAINT:
      break;
    case CDDS_PREPAINT:
      res = CDRF_NOTIFYITEMDRAW;
      // fall through
//    default:
      return true;
    default:
      return false;
    }

  LVITEM lv;
  lv.iItem = lplvcd->nmcd.dwItemSpec;
  lv.mask = LVIF_STATE | LVIF_PARAM;
  if(ListView_GetItem(*this, &lv)) {
    long id = (long)lv.lParam;
    currAlarm *Al = getGestAlarm();
    fullInfoAlarm idAlarm;
    FILETIME ft;
    currAlarm::alarmStat cStat;
    if(!Al->find(id, &idAlarm, &ft, cStat))
      return false;

    TCHAR cGrp = idAlarm.idGrp + _T('0');
    infoColorLB* ic = 0;

    switch(cStat) {
    // se allarme attivo
      case currAlarm::asActive:
        // se non è un evento, usa le impostazioni di base
        if(cGrp >= (CHAR_INIT_ALARM + getWarningLevelFilterAlarm()) && cGrp <= CHAR_END_ALARM)
          ic = &cActive;
        else
          ic = &cEvent;
        break;

      default:
      case currAlarm::asInactive:
        return false;

      case currAlarm::asActiveRecognized:
        if(cGrp >= (CHAR_INIT_ALARM + getWarningLevelFilterAlarm()) && cGrp <= CHAR_END_ALARM)
          ic = &cAcknoledged;
        else
          ic = &cAcknoledgedEvent;
        break;
      }
//    if((LVIS_SELECTED | LVIS_FOCUSED) & lv.state) {
//    if(CDIS_FOCUS & lplvcd->nmcd.uItemState) {
    if((CDIS_SELECTED  | CDIS_FOCUS) & lplvcd->nmcd.uItemState) {
      lplvcd->clrText = ic->fgS;
      lplvcd->clrTextBk = ic->bgS;
      }
    else {
      lplvcd->clrText = ic->fgN;
      lplvcd->clrTextBk = ic->bgN;
      }
    res = CDRF_NEWFONT;
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool pVarListViewAlarm::evKeyDown(UINT& key)
{
  return evSetAroundFocus(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
