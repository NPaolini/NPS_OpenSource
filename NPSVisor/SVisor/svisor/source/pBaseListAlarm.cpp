//------ pBaseListAlarm.cpp -----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pBaseListAlarm.h"
#include "p_basebody.h"
#include "mainClient.h"
#include "p_util.h"
#include "currAlrm.h"
#include "p_name.h"
#include "1.h"
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
#define SEP TAB
//----------------------------------------------------------------------------
#define D_ID_PERIF      3
#define D_ID_STAT       3 // stato
#define D_ID_AL_GRP     3 // gruppo
#define D_ID_AL_FLT     3 // filtro
#define D_ID_COD_ALARM  8 // allarme, composto da id + (addr, bit)
//#define D_ID_COD_ALARM 16 // allarme, composto da id + (addr, bit)
#define D_DESCR_AL_GRP 20 // descrizione gruppo
#define D_DATE         11
#define D_TIME          9
#define D_DESCR_ALARM 200
#define D_DESCR_ALARM_LV 50
//----------------------------------------------------------------------------
#define FILL__(a, b) fillStr(a, b, SIZE_A(a)-1); a[SIZE_A(a)-1] = 0
#define SPACE__(a) FILL__(a, _T(' '))
#define ALL_DIM (D_ID_AL_FLT + D_ID_PERIF + D_ID_AL_GRP + D_ID_COD_ALARM + D_DESCR_AL_GRP + D_DATE + D_TIME + D_DESCR_ALARM + D_ID_STAT + 9)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DEF_START_SHOW (esIdAlarm | esTime | esPrph | esDescr)
//----------------------------------------------------------------------------
static int tabShowAl[] = {  D_ID_COD_ALARM, D_ID_STAT, D_DATE, D_TIME, D_ID_AL_FLT, D_ID_AL_GRP, D_DESCR_AL_GRP, D_ID_PERIF, D_DESCR_ALARM_LV };
//----------------------------------------------------------------------------
static int centerAlign[] = { 0, 1, 2, 3, 4, 5, 7 };
//----------------------------------------------------------------------------
void pBaseListAlarm::reverseTrueOrder(int target[], int source[])
{
  for(uint i = 0; i < MAX_SHOW_AL; ++i) {
    for(uint j = 0; j < MAX_SHOW_AL; ++j) {
      if(i == source[j]) {
        target[i] = j;
        break;
        }
      }
    }
}
//----------------------------------------------------------------------------
void pBaseListAlarm::verifyTrueOrder(int set[])
{
  DWORD t = 0;
  for(uint i = 0; i < MAX_SHOW_AL; ++i) {
    if(set[i] < 0 || set[i] >= MAX_SHOW_AL)
      set[i] = i;
    if(t & (1 << set[i])) {
      for(uint j = 0; j < MAX_SHOW_AL; ++j) {
        uint k = (j + i + 1) % MAX_SHOW_AL;
        if(!(t & (1 << k))) {
          t |= 1 << k;
          set[i] = k;
          break;
          }
        }
      }
    else
      t |= 1 << set[i];
    }
}
//----------------------------------------------------------------------------
void pBaseListAlarm::makeTrueOrder(int target[], int source[], int set[])
{
  for(uint i = 0; i < MAX_SHOW_AL; ++i)
    target[set[i]] = source[i];
}
//----------------------------------------------------------------------------
void pBaseListAlarm::makeSet(DWORD var, int set[])
{
  for(uint i = 0; i < MAX_SHOW_AL; ++i, var >>= 1)
    set[i] = var & 1;
}
//----------------------------------------------------------------------------
pBaseListAlarm::pBaseListAlarm(P_BaseBody* owner, uint id, pAround& Around) :
    Owner(owner), currShow(DEF_START_SHOW), Around(Around)
{
  ZeroMemory(percWidth, sizeof(percWidth));
  for(uint i = 0; i < SIZE_A(columnPos); ++i)
    columnPos[i] = i;
  ZeroMemory(FilterText, sizeof(FilterText));
}
//----------------------------------------------------------------------------
pBaseListAlarm::~pBaseListAlarm()
{
}
//----------------------------------------------------------------------------
static
LPCTSTR load_Color(COLORREF& c, LPCTSTR p)
{
  uint col[3];
  memset(col, 0, sizeof(col));
  for(int i = 0; i < SIZE_A(col); ++i) {
    if(!p)
      break;
    col[i] = _ttoi(p);
    p = findNextParamTrim(p);
    }
  c = RGB(col[0], col[1], col[2]);
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR pBaseListAlarm::loadColor(infoColorLB& ic, LPCTSTR p)
{
  p = load_Color(ic.fgN, p);
  p = load_Color(ic.bgN, p);
  p = load_Color(ic.fgS, p);
  p = load_Color(ic.bgS, p);
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR pBaseListAlarm::loadColumnWidth(LPCTSTR p)
{
  for(uint i = 0; i < MAX_SHOW_AL; ++i) {
    if(!p)
      break;
    columnPos[i] = _ttoi(p) - 1;
    p = findNextParamTrim(p);
    }
  verifyTrueOrder(columnPos);
  return p;
}
//----------------------------------------------------------------------------
void pBaseListAlarm::loadAllColor(LPCTSTR p)
{
  p = loadColor(cActive, p);
  p = loadColor(cAcknoledged, p);
  p = loadColor(cEvent, p);
  p = loadColor(cAcknoledgedEvent, p);
  p = loadColor(cBase, p);
}
//----------------------------------------------------------------------------
void pBaseListAlarm::calcPercWidth(LPCTSTR p)
{
  int cx = getSizeFont().cx;

  for(uint i = 0; i < MAX_SHOW_AL; ++i) {
    if(!p)
      break;
    percWidth[i] = _ttoi(p);
    if(percWidth[i])
      percWidth[i] = percWidth[i] * tabShowAl[i] / 100;
    else
      percWidth[i] = tabShowAl[i];
    percWidth[i] *= cx;
    percWidth[i] /= SIMUL_DEC_FONT;
    p = findNextParamTrim(p);
    }
}
//----------------------------------------------------------------------------
void pBaseListAlarm::formatHeaderAlarm(PVect<LPCTSTR>& txt, int width[], uint idHeadTitle)
{
  int show_alarm[MAX_SHOW_AL] = { 0 };
  makeSet(currShow, show_alarm);

  int tTab[MAX_SHOW_AL] = { 0 };
  reverseTrueOrder(tTab, columnPos);

  int tShow[MAX_SHOW_AL] = { 0 };
  makeTrueOrder(tShow, show_alarm, columnPos);

  int tWidth[MAX_SHOW_AL] = { 0 };
  makeTrueOrder(tWidth, percWidth, columnPos);

  if(idHeadTitle) {
    for(uint i = 0, j = 0; i < SIZE_A(tabShowAl); ++i) {
      if(tShow[i]) {
        smartPointerConstString cod(getStringOrIdByLangGlob(idHeadTitle + tTab[i]));
        txt[j] = str_newdup(cod);
        width[j] = tWidth[i];
        ++j;
        }
      }
    }
  else {
    for(uint i = 0, j = 0; i < SIZE_A(tabShowAl); ++i) {
      if(tShow[i]) {
        TCHAR t[64];
        wsprintf(t, _T("id mancante %d"), ID_INIT_TEXT_HEADER_LB_ALARM + tTab[i]);
        txt[j] = str_newdup(t);
        width[j] = tWidth[i];
        ++j;
        }
      }
    }
}
//----------------------------------------------------------------------------
static bool acceptRowAlarm(const fullInfoAlarm& fia, LPCTSTR FilterText)
{
  if(*FilterText) {
    TCHAR descr[D_DESCR_ALARM];
    makeAlarmDescr(descr, D_DESCR_ALARM, fia);
    _tcslwr_s(descr);
    return toBool(_tcsstr(descr, FilterText));
    }
  return true;
}
//----------------------------------------------------------------------------
void pBaseListAlarm::formatAlarm(PVect<LPCTSTR>& items, const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat)
{
  if(!acceptRowAlarm(fia, getFilterText()))
    return;

  int nBitAlarm = 32;
  mainClient* par = getMain();
  genericPerif *prf = par->getGenPerif(fia.getTruePrph());
  if(prf)
    nBitAlarm = prf->getNBitAlarm();

  int addr = (fia.alarm - 1) / nBitAlarm;
  if(prf)
    addr += prf->getAddrAlarm();

  int tTab[MAX_SHOW_AL] = { 0 };
  reverseTrueOrder(tTab, columnPos);
  int len;

  int show_alarm[MAX_SHOW_AL] = { 0 };
  makeSet(currShow, show_alarm);

  int tShow[MAX_SHOW_AL] = { 0 };
  makeTrueOrder(tShow, show_alarm, columnPos);

  TCHAR buff[D_DESCR_ALARM + 2];
  for(uint i = 0; i < MAX_SHOW_AL; ++i) {
    if(!tShow[i])
      continue;
    switch(tTab[i]) {
      case 0:
        wsprintf(buff, _T("%d (%d, %d)"), fia.alarm, addr, (fia.alarm - 1) % nBitAlarm);
        break;
      case 1:
        buff[1] = 0;
        switch(stat) {
          case currAlarm::asActive:
            buff[0] = _T('1');
            break;
          case currAlarm::asInactive:
            buff[0] = _T('0');
            break;
          default:
            buff[0] = _T('2');
            break;
          }
        break;
      case 2:
        set_format_data(buff, SIZE_A(buff), st, whichData(), _T("|"));
        buff[10] = 0;
        break;
      case 3:
        wsprintf(buff, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
        break;
      case 4:
        wsprintf(buff, _T("%d"), fia.idFlt);
        break;
      case 5:
        buff[1] = 0;
        buff[0] = fia.idGrp + _T('0');
        break;
      case 6:
        {
        smartPointerConstString cod(getStringOrIdByLangGlob(fia.idGrp + INIT_COD_ALARM));
        len = _tcslen(cod);
        if(len > D_DESCR_AL_GRP)
          len = D_DESCR_AL_GRP;
        copyStr(buff, (LPCTSTR)cod, len);
        buff[len] = 0;
        }
        break;
      case 7:
        buff[1] = 0;
        buff[0] = fia.getTruePrph() + _T('0');
        break;
      case 8:
        makeAlarmDescr(buff, D_DESCR_ALARM, fia);
        break;
      }
    items[items.getElem()] = str_newdup(buff);
    }
}
//----------------------------------------------------------------------------
bool pBaseListAlarm::isFirstRow()
{
  int currSel = getCurrSel();
  if(currSel < 0)
    currSel = getCount();
  return currSel <= 0;
}
//----------------------------------------------------------------------------
bool pBaseListAlarm::isLastRow()
{
  int currSel = getCurrSel();
  int count =  getCount();
  if(currSel < 0) {
    if(count)
      return false;
    return true;
    }
  return currSel == count - 1;
}
//----------------------------------------------------------------------------
bool pBaseListAlarm::evSetAroundFocus(UINT key)
{
  switch(key) {
    case VK_UP:
      if(isFirstRow()) {
        Around.setFocus(pAround::aTop);
        return true;
        }
      break;
    case VK_DOWN:
      if(isLastRow()) {
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
bool pBaseListAlarm::setFocus(pAround::around where)
{
  uint key[] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };
  return evSetAroundFocus(key[where]);
}
//----------------------------------------------------------------------------
