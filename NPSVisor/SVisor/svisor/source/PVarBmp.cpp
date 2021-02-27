//------ PVarBmp.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarBmp.h"
#include "p_basebody.h"
#include "newNormal.h"
#include "mainclient.h"
#include "p_util.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PVarBmp::PVarBmp(P_BaseBody* owner, uint id) : baseVar(owner, id), Style(fixed), currState(sOff), 
          aBmp(0), Bmp(0), needRefresh(false), timeCycle(0), lastTime(0)

{}
//----------------------------------------------------------------------------
PVarBmp::~PVarBmp()
{
  delete Bmp;
  delete []aBmp;
  flushPV(localBmp);
}
//----------------------------------------------------------------------------
extern DWORD getGlobalTimeCycle();
//----------------------------------------------------------------------------
bool PVarBmp::allocObj(LPVOID)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int scale = 0;
  int flag = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &scale, &flag);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  POINT pt = { R__X(x), R__Y(y) };

  p = findNextParam(p, 4);
  if(!p)
    return false;

  int mul = PVarBmp::maxMul;

  Style = flag;
  while(flag > PVarBmp::lastCode) {
    flag -= mul;
    mul /= 10;
    }

  int numBmp;
  switch(flag) {
    case PVarBmp::fixed_lamp:
    case PVarBmp::fixed:
      numBmp = 1;
      break;
    case PVarBmp::on_off:
    case PVarBmp::dual_lamp:
      numBmp = 2;
      break;
    default:
      numBmp = -1;
      break;
    }

  PVect<uint> idBmp;
  p = insertIdBmp(p, idBmp, numBmp);
  if(p) {
    timeCycle = -_ttoi(p);
    if((int)timeCycle < 0)
      timeCycle = 0;
    }
  if(!timeCycle)
    timeCycle = getGlobalTimeCycle();
  if(scale < 0) {
    const PVect<PBitmap*>& source = getOwner()->getBmp4Var();
    SIZE sz = source[idBmp[0]]->getSize();
    double t = R__X(-scale);
    t /= sz.cx;
    scale = (int)(t * 1000 + 0.5);
    }
  else
    scale = R__X(scale);

  int nElem = idBmp.getElem();

  aBmp = new PBitmap*[nElem];

  const PVect<PBitmap*>& source = getOwner()->getBmp4Var();

  for(int i = 0; i < nElem; ++i) {
    uint ix = idBmp[i];
    aBmp[i] = source[ix];
    if(aBmp[i]->hasGifControl()) {
      uint nElem = localBmp.getElem();
      localBmp[nElem] = new PBitmap(*aBmp[i]);
      aBmp[i] = localBmp[nElem];
      }
    aBmp[i]->addGifControl(getOwner());
    }

  Bmp = new PSeqTraspBitmap(getOwner(), aBmp, nElem, pt);

  Bmp->setScale(scale / 1000.0);
//  Bmp->setCanSaveBkg(false);

  if(Style >= enableMove) {
    Style -= enableMove;
    Effect.move = true;
    }

  if(Style >= hideAnim) {
    Style -= hideAnim;
    Effect.hide = true;
    }

  if(Style >= noTransp) {
    Style -= noTransp;
    Effect.noTrsp = true;
    }

  if(Style >= negative) {
    Style -= negative;
    Effect.neg = true;
    }

  if(Effect.noTrsp)
    Bmp->setTransp(false);

//  if(sequence == Style || anim == Style || on_off == Style)
    currState = sShow;
  // per compatibilità lo lasciamo
  if(Effect.move) {
    uint id2 = getThirdExtendId();
    p = getOwner()->getPageString(id2);
    delete MoveInfo;
    MoveInfo = new pInfoMoveObj(this);
    if(!MoveInfo->init(p)) {
      delete MoveInfo;
      MoveInfo = 0;
      }
    }
  PRect r(Bmp->getRect());
  setRect(r);
  return makeStdVars();
}
//----------------------------------------------------------------------------
/*
void PVarBmp::allocMovingData(LPCTSTR p)
{
  if(!p) {
    Effect.move = 0;
    return;
    }
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  int code[MAX_MOVING_DATA];
  int dir = 0;
  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    code[i] = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
          &x1, &y1, &x2, &y2,
          &code[0], &code[1], &code[2], &code[3], &code[4], &code[5], &dir);

  for(int i = 0; i < MAX_MOVING_DATA; ++i) {
    if(!code[i]) {
      Effect.move = 0;
      return;
      }
    }

  movingData = new moving_data;

  PRect r = Bmp->getRect();
  SIZE sz = { r.Width(), r.Height() };

  sz.cx = (LONG)(sz.cx * Bmp->getScaleX());
  sz.cy = (LONG)(sz.cy * Bmp->getScaleY());

  x1 = R__X(x1);
  y1 = R__Y(y1);
  x2 = R__X(x2);
  y2 = R__Y(y2);

  x2 -= sz.cx;
  y2 -= sz.cy;
  if(x2 < x1)
    x2 = x1;
  if(y2 < y1)
    y2 = y1;
  r = PRect(x1, y1, x2, y2);
  movingData->rect = r;
  if(r != movingData->rect)
    dir ^= 1;
  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    fillData(movingData->Data[i], code[i]);

  movingData->dir = dir;

}
//----------------------------------------------------------------------------
void PVarBmp::fillData(infoData& Data, int code)
{
  Data.prph = 0;
  manageObjId moi(code);
  uint id = moi.getFirstExtendId();
  LPCTSTR p = getOwner()->getPageString(id);
  if(!p)
    return;

  int idprph = 0;
  DWDATA addr = 0;
  uint type = 0;
  DWDATA norm = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d"), &idprph, &addr, &type, &norm);

  // non ha senso il tipo stringa o il tipo bit
  if(prfData::tStrData == type || prfData::tBitData == type)
    return;

  Data.data.typeVar = (prfData::tData)type;

  Data.data.lAddr = addr;
  Data.data.pAddr = 0;
  Data.norm = norm;
  Data.prph = idprph;
}
*/
//----------------------------------------------------------------------------
bool PVarBmp::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible()) {
/*
    if(upd && Effect.move) {
      PRect r = get_Rect();
      r |= movingData->rect;
      InvalidateRect(*getOwner(), r, 0);
      }
*/
    return upd;
    }
/*
  if(anim == getStyle()) {
    invalidate();
    return;
    }
*/
  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  bool select;

  DWORD mask = (DWORD)BaseVar.getMaskA();
  // se non c'è la maschera la si crea dal pacchetto di bit
  if(!mask) {
    if(!BaseVar.getnBit())
      mask = (DWDATA) -1;
    else
      mask = (1 << BaseVar.getnBit()) - 1;
    }
  DWDATA val = 0;

  // se campo di bit, basta che sia attivo almeno un bit della maschera
  if(prfData::tBitData == BaseVar.getType()) {
    val = data.U.dw & mask;
    select = toBool(val);
    // se sequenza si trasforma la posizione del bit nel numero
    if(sequence & getStyle()) {
      val = 0;
      DWDATA j = data.U.dw;
      for(uint i = 0; i < BaseVar.getnBit(); ++i, j >>= 1) {
        if(1 & j) {
          val = i + 1;
          break;
          }
        }
      }
    }
  // altrimenti devono essere attivi tutti i bit della maschera
  else {
    val = (data.U.dw >> BaseVar.getOffs()) & mask;
    select = val == mask;
    }
  select = select ^ BaseVar.getNegA();

  force |= upd;

  if(force || prfData::notModified != result) {
    if(select)
      Set();
    else
      Reset();
    needRefresh = true;
    }

  performAltern(val);
//  updateMoving();
  if(needRefresh) {
    invalidate();
    upd = true;
    }
  needRefresh = false;
  return upd;
}
//----------------------------------------------------------------------------
/*
double getVal(const PVarBmp::infoData& Data)
{
  const prfData& data = Data.data;
  double norm = getNorm(Data.norm);
  switch(data.typeVar) {
    case prfData::tBData:
      return data.U.b * norm;

    case prfData::tBsData:
      return data.U.sb * norm;

    case prfData::tWData:
      return data.U.w * norm;

    case prfData::tWsData:
      return data.U.sw * norm;

    case prfData::tDWData:
      return data.U.dw * norm;

    case prfData::tDWsData:
      return data.U.sdw * norm;

    case prfData::tFRData:
      return data.U.fw * norm;

    case prfData::ti64Data:
      return (double)data.U.li.QuadPart * norm;

    case prfData::tRData:
      return data.U.rw * norm;

    default:
      return 0;
    }
}
//----------------------------------------------------------------------------
static
void getData(PWin* w, PVarBmp::infoData& data)
{
  mainClient* Par = getMain();
  if(!Par)
    return;

  genericPerif* prph = Par->getGenPerif(data.prph);
  if(prph)
    prph->get(data.data);
}
*/
//----------------------------------------------------------------------------
const PRect& PVarBmp::get_Rect() const
{
//  if(!Effect.move)
    return baseVar::get_Rect();
//  static PRect rect;
//  rect = Bmp->getRect();
//  return rect;
}
//----------------------------------------------------------------------------
/*
void PVarBmp::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  if(!Effect.move)
    return;
  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    addReqVar2(allBits, movingData->Data[i].prph, movingData->Data[i].data.lAddr);
}
//----------------------------------------------------------------------------
void PVarBmp::updateMoving()
{
  if(!Effect.move)
    return;
  if(!Bmp->isEnabled())
//  if(!Bmp->isVisible())
    return;

  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    getData(getOwner(), movingData->Data[i]);

  enum { ID_MIN_X, ID_MIN_Y, ID_MAX_X, ID_MAX_Y, ID_CURR_X, ID_CURR_Y };

  double rangeX = getVal(movingData->Data[ID_MAX_X]);
  double minX = getVal(movingData->Data[ID_MIN_X]);
  rangeX -= minX;
  if(rangeX <= 0.0)
    rangeX = 1.0;

  double rangeY = getVal(movingData->Data[ID_MAX_Y]);
  double minY = getVal(movingData->Data[ID_MIN_Y]);
  rangeY -= minY;
  if(rangeY <= 0.0)
    rangeY = 1.0;

  double scaleX = movingData->rect.Width() / rangeX;
  double scaleY = movingData->rect.Height() / rangeY;
  POINT pt;
  pt.x = (LONG)((getVal(movingData->Data[ID_CURR_X]) - minX) * scaleX);
  pt.y = (LONG)((getVal(movingData->Data[ID_CURR_Y]) - minY) * scaleY);

  if(pt.x > movingData->rect.Width())
    pt.x = movingData->rect.Width();
  else if(pt.x < 0)
    pt.x = 0;

  if(pt.y > movingData->rect.Height())
    pt.y = movingData->rect.Height();
  else if(pt.y < 0)
    pt.y = 0;

  switch(movingData->dir) {
    case 0:
      break;
    case 3:
      pt.y = movingData->rect.Height() - pt.y;
      // fall through
    case 1:
      pt.x = movingData->rect.Width() - pt.x;
      break;
    case 2:
      pt.y = movingData->rect.Height() - pt.y;
      break;
    }

  if(needRefresh || movingData->last.x != pt.x || movingData->last.y != pt.y) {
    movingData->last = pt;
    PRect r = Bmp->getRect();
    pt.x += movingData->rect.left;
    pt.y += movingData->rect.top;
    Bmp->moveToSimple(pt);
    r |= Bmp->getRect();
    InvalidateRect(*getOwner(), r, 1);
    needRefresh = false;
    }
}
*/
//----------------------------------------------------------------------------
void PVarBmp::Set()
{
  uint style = Style % 1000;
  switch(style) {
    case fixed:
//      Bmp->setEnable(true);
      currState = sShow;
      break;

    case on_off:
      Bmp->setCurr(1, false);
      break;

    case dual_lamp:
    case fixed_lamp:
//      Bmp->setEnable(true);
      Bmp->setCurr(0, false);
      currState = sShow;
      break;

    case animVar:
      currState = sShow;
      if(Effect.hide) {
        Bmp->setCurr(0, false);
//        Bmp->setEnable(true);
        }
      else
        Bmp->setCurr(1, false);
      break;
    }
}
//----------------------------------------------------------------------------
void PVarBmp::Reset()
{
  uint style = Style % 1000;
  switch(style) {
    case fixed:
//      Bmp->setEnable(false);
      currState = sOff;
      break;

    case on_off:
      Bmp->setCurr(0, false);
      break;

    case dual_lamp:
    case fixed_lamp:
//      Bmp->setEnable(false);
      currState = sOff;
      break;

    case animVar:
      Bmp->setCurr(0, false);
      if(Effect.hide)
        currState = sOff;
      break;
    }
}
//----------------------------------------------------------------------------
void PVarBmp::performDraw(HDC hdc)
{
  if(sShow != currState)
    return;
  PRect r(get_Rect());
  POINT pt = { r.left, r.top };
  Bmp->moveToSimple(pt);
  Bmp->Draw(hdc);
}
//----------------------------------------------------------------------------
void PVarBmp::toggleState()
{
  if(sShow == currState)
    currState = sHide;
  else
    currState = sShow;
}
//----------------------------------------------------------------------------
void PVarBmp::performAltern(DWDATA val)
{
  if(sOff == currState)
    return;

  uint style = Style % 1000;
  switch(style) {
    case fixed:
    case on_off:
      return;
    }
  if(needRefresh)
    lastTime = 0;
  DWORD curr = GetTickCount();
  if(curr - lastTime < timeCycle)
    return;
  lastTime = curr;
  switch(style) {
    case fixed_lamp:
      toggleState();
      break;

    case dual_lamp:
      Bmp->setCurr((Bmp->getCurr() + 1) & 1, false);
      break;

    case sequence:
      if(val == Bmp->getCurr() && !needRefresh)
        return;
      Bmp->setCurr(val, false);
      break;

    case animVar:
    case anim:
      do {
        if(animVar == style && !Effect.hide && !val)
          break;
        int curr = Bmp->getCurr() + 1;
        int nElem = Bmp->getNBmp();
        curr %= nElem;
        if(!curr && animVar == style && !Effect.hide)
          ++curr;
        Bmp->setCurr(curr, false);
        } while(0);
      break;
    }
  needRefresh = true;
}
//----------------------------------------------------------------------------
