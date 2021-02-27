//------ pVarListAlarm.cpp -----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarListAlarm.h"
#include "p_basebody.h"
#include "mainClient.h"
#include "p_util.h"
#include "currAlrm.h"
#include "p_name.h"
#include "1.h"
#include "pBaseListAlarm.h"
#include "pVarListViewAlarm.h"
#include "pVarListBoxAlarm.h"
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
inline DWORD clearUnusedBit(DWORD v)
{
  v &= (esMAX_VAL - 1) | (1 << ID_ALARM_BIT_INIT);
  return v;
}
//----------------------------------------------------------------------------
#define LOCK_BIT  10
#define ORDER_BIT 11
#define RESET_BIT 12
//----------------------------------------------------------------------------
//uint pVarListAlarm::idHeadTitle = (uint)-1;
uint pVarListAlarm::idHeadTitle = ID_INIT_TEXT_HEADER_LB_ALARM;
//----------------------------------------------------------------------------
pVarListAlarm::pVarListAlarm(P_BaseBody* owner, uint id) :
    baseClass(owner, id), listObj(0),
    idObj(0), filterFix(0),
    currFilter(0), currFilterGroup(0)
{
}
//----------------------------------------------------------------------------
pVarListAlarm::~pVarListAlarm()
{
  delete listObj;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool pVarListAlarm::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;

  int idFont = 0;
  int idFilter  = 0;
  int whichshow = 0;
  int idFilterTxt = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%lld,%d,%d"),
                    &x, &y, &w, &h, &idFont,
                    &idObj, &filterFix,
                    &idFilter, &whichshow
                    );
  bool useListBox = toBool(whichshow & esUseListBox);

  if(useListBox)
    listObj = new pVarListBoxAlarm(getOwner(), getId(), Around);
  else
    listObj = new pVarListViewAlarm(getOwner(), getId(), Around);
  PControl* ctrl = dynamic_cast<PControl*>(listObj);
  currFilter = filterFix;
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  ctrl->Attr.x = rect.left;
  ctrl->Attr.y = rect.top;
  ctrl->Attr.w = rect.Width();
  ctrl->Attr.h = rect.Height();
  setRect(rect);

  if(idObj - 1 > 31)
    return false;
  if(!makeStdVars())
    return false;

  LPCTSTR pW = findNextParamTrim(p, 9 + MAX_SHOW_AL);
  pW = listObj->loadColumnWidth(pW);

  int idFilterGroup  = 0;
  if(pW) {
    idFilterGroup = _ttoi(pW);
    pW = findNextParamTrim(pW);
    if(pW)
      idFilterTxt = _ttoi(pW);
    }

  pW = findNextParamTrim(p, 9);

  p = getOwner()->getPageString(getThirdExtendId());
  listObj->loadAllColor(p);

  makeOtherVar(vFilter, idFilter);
  if(idFilterGroup)
    makeOtherVar(vFilterGroup, idFilterGroup);

  if(idFilterTxt) {
     makeOtherVar(vFilterTxt, idFilterTxt);
//     vFilterTxt.setnBit(vFilterTxt.getNorm());
//     vFilterTxt.setNorm(0);
     }
  idFont -= ID_INIT_FONT;
  HFONT font = getOwner()->getFont(idFont);
  ctrl->setFont(font, false);

  // elimina i bit idesiderati
  whichshow = clearUnusedBit(whichshow);
  if(whichshow)
    listObj->setShow(whichshow);

  if(!listObj->create())
    return false;

  listObj->calcPercWidth(pW);
  return true;
}
//----------------------------------------------------------------------------
HWND pVarListAlarm::getHwnd() { return listObj->getHwnd(); }
//----------------------------------------------------------------------------
void pVarListAlarm::setVisibility(bool set)
{
  baseActive::setVisibility(set);
  invalidateWithBorder();
}
//----------------------------------------------------------------------------
void pVarListAlarm::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vFilter);
  addReqVar2(allBits, vFilterGroup);
  addReqVar2(allBits, vFilterTxt);
}
//----------------------------------------------------------------------------
bool pVarListAlarm::setFocus(pAround::around where)
{
  return listObj->setFocus(where);
}
//----------------------------------------------------------------------------
static bool isLocked(DWORD v)
{
  return toBool(v & (1 << LOCK_BIT));
}
//----------------------------------------------------------------------------
static bool revOrder(DWORD v)
{
  return toBool(v & (1 << ORDER_BIT));
}
//----------------------------------------------------------------------------
static bool needReset(DWORD& v)
{
  bool result = toBool(v & (1 << RESET_BIT));
  v &= ~(1 << RESET_BIT);
  return result;
}
//----------------------------------------------------------------------------
bool pVarListAlarm::setBaseData(const prfData& data)
{
  mainClient* mc = getMain();
  genericPerif* prph = mc->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return false;
  prph->set(data, true);
  return true;
}
//----------------------------------------------------------------------------
extern bool getUse64Filter();
#ifndef STEP_GRP_ALARM_BITS
  #define STEP_GRP_ALARM_BITS 10
#endif
//----------------------------------------------------------------------------
bool pVarListAlarm::update(bool force)
{
  bool upd = baseActive::update(force);
  if(!isVisible())
    return upd;

  force |= upd;

  bool fullReload = force;

  bool order = true;
  prfData data;
  if(BaseVar.getPrph() && BaseVar.getAddr()) {
    force |= prfData::isChanged == BaseVar.getData(getOwner(), data, getOffs());
    if(needReset(data.U.dw)) {
      if(!setBaseData(data))
        return false;
      mainClient* par = getMain();
      genericPerif *prf = par->getGenPerif(1);
      if(prf)
        prf->sendResetAlarm();
      perifsSet* Perif = par->getPerifs();
      Perif->resetAlarm();
      currAlarm *Al = getGestAlarm();
       Al->remove(-1);
       fullReload = true;
      }
    if(!fullReload && isLocked(data.U.dw))
      return upd;
    order = revOrder(data.U.dw);
    DWORD whichshow = clearUnusedBit(data.U.dw);
    if(whichshow != listObj->getShow()) {
      if(!(whichshow & (1 << ID_ALARM_BIT_INIT))) { // probabile prima entrata, il bit deve essere attivo
        data.U.dw |= listObj->getShow() | (1 << ID_ALARM_BIT_INIT);
        if(!setBaseData(data))
          return false;
        }
      else
        listObj->setShow(whichshow);
      fullReload = true;
      }
    }

  if(vFilter.getPrph() && vFilter.getAddr()) {
    LARGE_INTEGER li;
    li.QuadPart = 0;
    vFilter.getData(getOwner(), data, getOffs());
    li.LowPart = data.U.dw;
    if(getUse64Filter()) {
      DWORD addr = vFilter.getAddr();
      vFilter.setAddr(addr + 1);
      vFilter.getData(getOwner(), data, getOffs());
      li.HighPart = data.U.dw;
      vFilter.setAddr(addr);
      }
    li.QuadPart |= filterFix;
    if(li.QuadPart != currFilter) {
      currFilter = li.QuadPart;
      force = true;
      }
    }

  if(vFilterGroup.getAddr() && (prfData::isChanged == vFilterGroup.getData(getOwner(), data, getOffs()) || force)) {
    DWORD flt = data.U.dw;
    if(flt != currFilterGroup) {
      currFilterGroup = flt;
      force = true;
      }
    }
  if(vFilterTxt.getPrph()) {
    prfData fdata(vFilterTxt.getDec());
    if(prfData::isChanged == vFilterTxt.getData(getOwner(), fdata, getOffs()))
      force = true;
    copyStrZ(listObj->FilterText, fdata.U.str.buff, fdata.U.str.len + 1);
    trim(lTrim(listObj->FilterText));
    _tcslwr_s(listObj->FilterText);
    }

  currAlarm *Al = getGestAlarm();
  if(Al->isDirtyByObj(idObj) || fullReload || force) {
    Al->commitByObj(idObj);
    bool oldOrder = Al->isReverseOrder();
    Al->setOrder(order);
    int sel = listObj->getCurrSel();
    listObj->removeAllItem();

    if(fullReload)
      listObj->formatHeaderAlarm(idHeadTitle);

    FilterAlarm* fAl = getFilterAlarm();
    FilterAlarm* oldAl = fAl->makeclone();
    LARGE_INTEGER li;
    li.QuadPart = currFilter;
    fAl->setDWordAt(0, li.LowPart);
    fAl->setDWordAt(1, li.HighPart);

    if(!Al->setFirst()) {
      Al->setOrder(oldOrder);
      fAl->copyAll(oldAl, true);
      return upd;
      }

    SYSTEMTIME st;
    currAlarm::alarmStat stat;
    do {
      fullInfoAlarm fia;
      Al->get(&fia, &st, stat);
      if(!currFilterGroup || (currFilterGroup & (1 << fia.idGrp)))
        listObj->formatAlarm(fia, st, stat);
      } while(Al->setNext());
    listObj->calcWidth();
    Al->setOrder(oldOrder);
    fAl->copyAll(oldAl, true);

    uint count = listObj->getCount();
    if(sel < 0 && count)
      sel = 0;
    else if((uint)sel >= count)
      sel = count - 1;
    listObj->setCurrSel(sel);
    }
  return upd;
}
//----------------------------------------------------------------------------
