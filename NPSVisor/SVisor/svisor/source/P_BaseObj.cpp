//------ P_BaseObj.cpp -------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "mainclient.h"
#include "p_util.h"
#include "pEdit.h"
#include "p_basebody.h"
#include "newNormal.h"
#include "config.h"
//----------------------------------------------------------------------------
void pConst::makeConst(LPCTSTR p)
{
  // il valore costante viene sempre salvato come (cast) dword, ed è il type che indica se int o real
  DWDATA v = 0;
  int type = 0;
  _stscanf_s(p, _T("%u,%d"), &v, &type);
  Owner->setOffs(v);
  Owner->setType(type);
}
//----------------------------------------------------------------------------
prfData::tResultData pConst::getData(prfData& data)
{
  if(prfData::tFRData == Owner->getType())
    data.typeVar = prfData::tFRData;
  else
    data.typeVar = prfData::tDWData;
  data.U.dw = Owner->getOffs();
  return prfData::okData;
}
//----------------------------------------------------------------------------
void pVariable::makeConst(LPCTSTR p)
{
  vConst = new pConst(this);
  vConst->makeConst(p);
}
//----------------------------------------------------------------------------
prfData::tResultData pVariable::getData(P_BaseBody* owner, prfData& data, uint addrOffs)
{
  if(vConst)
    return vConst->getData(data);

  data.lAddr = Addr + addrOffs;

  if(prfData::tStrData == Type)
    data.setStr(Dec);
  else {
    data.typeVar = Type;
    if(prfData::tBitData == Type)
      data.U.dw =  MAKELONG(nBit, Offs);
    }

  if(!Prph)
    return prfData::invalidPerif;

  genericPerif* prph = owner->getGenPerif(Prph);
  if(!prph)
    return prfData::invalidPerif;

  return prph->get(data);
}
//----------------------------------------------------------------------------
double pVariable::getResult(prfData& data)
{
  switch(Type) {
    case prfData::tBData:
      return data.U.b;
      break;
    case prfData::tBsData:
      return data.U.sb;
      break;

    case prfData::tWData:
      return data.U.w;
      break;
    case prfData::tWsData:
      return data.U.sw;
      break;

    case prfData::tBitData: // il campo di bit viene trattato come dword
    case prfData::tDWData:
      return data.U.dw;
      break;
    case prfData::tDWsData:
      return data.U.sdw;
      break;

    case prfData::tFRData:
      return data.U.fw;

    case prfData::ti64Data: // 64 bit
      return (double)data.U.li.QuadPart;

    case prfData::tRData:   // 64 bit float
      return data.U.rw;

    default:
      return 0.0;
    }
}
//----------------------------------------------------------------------------
double pVariable::getNormalizedResult(prfData& data)
{
  double norm = ::getNorm(getNorm());
  return getNormalized(getResult(data), norm);
}
//----------------------------------------------------------------------------
int pVariable::getIxResult(prfData& data)
{
  if(prfData::tBitData == Type) {
    DWDATA j = data.U.dw;
    for(uint i = 0; i < nBit; ++i, j >>= 1) {
      if(1 & j) {
        data.U.dw = i + 1;
        break;
        }
      }
    if(Neg && nBit == 1) {
      if(data.U.dw)
        data.U.dw = 0;
      else
        data.U.dw = 1;
      }

    }
  else {
    if(nBit) {
      data.U.dw >>= Offs;
      if(nBit < 32)
        data.U.dw &= (1 << nBit) - 1;
      }
    }
  switch(Type) {
    case prfData::tFRData:
      return (int)ROUND_REAL(data.U.fw);

    case prfData::tRData:   // 64 bit float
      return (int)ROUND_REAL(data.U.rw);
      break;

    default:
      return (int)data.U.dw;
    }
}
//----------------------------------------------------------------------------
bool pVariable::getDeNormalized(prfData& target, REALDATA input)
{
  target.lAddr = getAddr();

  REALDATA nrm = ::getNorm(getNorm());
  target.typeVar = getType();

  switch(getType()) {

    case prfData::tBData:
      getDenormalizedInt(target.U.b, input, nrm);
      break;
    case prfData::tBsData:
      getDenormalizedInt(target.U.sb, input, nrm);
      break;

    case prfData::tWData:
      getDenormalizedInt(target.U.w, input, nrm);
      break;
    case prfData::tWsData:
      getDenormalizedInt(target.U.sw, input, nrm);
      break;

    case prfData::tBitData: // il campo di bit viene trattato come dword
    case prfData::tDWData:
      getDenormalizedInt(target.U.dw, input, nrm);
      break;
    case prfData::tDWsData:
      getDenormalizedInt(target.U.sdw, input, nrm);
      break;

    case prfData::tFRData:
      getDenormalizedReal(target.U.fw, input, nrm);
      break;

    case prfData::ti64Data: // 64 bit
      getDenormalizedInt(target.U.li.QuadPart, input, nrm);
      break;

    case prfData::tRData:   // 64 bit float
      getDenormalizedReal(target.U.rw, input, nrm);
      break;

//    case prfData::tStrData:
//      break;

    default:
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
void getLocaleDecimalPoint(LPTSTR buff, size_t dim)
{
  int res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buff, dim);
  if(!res)
    *buff = _T('.');
}
//----------------------------------------------------------------------------
void getLocaleThousandSep(LPTSTR buff, size_t dim)
{
  int res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, buff, dim);
  if(!res)
    *buff = _T('.');
}
//----------------------------------------------------------------------------
void makeBinaryString(DWORD val, LPTSTR buffer, size_t lenBuff)
{
  TCHAR t[256];
  int i;
  for(i = 0; val; ++i) {
    if(val & 1)
      t[i] = _T('1');
    else
      t[i] = _T('0');
    val >>= 1;
    }
  t[i] = 0;
  buffer[i] = 0;
  --i;
  for(int j = 0; i >= 0; ++j, --i)
    buffer[j] = t[i];
}
//----------------------------------------------------------------------------
void makeHexString(DWORD val, LPTSTR buffer, size_t lenBuff)
{
  wsprintf(buffer, _T("%02x"), val);
}
//----------------------------------------------------------------------------
static DWORD getBinary(LPCTSTR source)
{
  DWORD code = 0;
  int len = _tcslen(source);
  for(int i = len - 1, j = 0; i >= 0; --i, ++j) {
    if(_T('1') == source[i])
      code |= 1 << j;
    }
  return code;
}
//----------------------------------------------------------------------------
static int getDec(TCHAR car)
{
  if(car >= _T('0') && car <= _T('9'))
    return car - _T('0');
  if(car >= _T('a') && car <= _T('f'))
    return car - _T('a') + 10;
  if(car >= _T('A') && car <= _T('F'))
    return car - _T('A') + 10;
  return 0;
}
//----------------------------------------------------------------------------
static DWORD getHex(LPCTSTR buff)
{
  int code = 0;
  int len = _tcslen(buff);
  for(int i = len - 1, j = 0; i >= 0; --i, ++j) {
    int val = getDec(buff[i]);
    val <<= j * 4;
    code += val;
    }
  return code;
}
//----------------------------------------------------------------------------
bool pVariable::getDeNormalized(P_BaseBody* owner, prfData& target, LPCTSTR source)
{
  if(prfData::tStrData == getType()) {
    target.lAddr = getAddr();

    int len = _tcslen(source);
    if((int)getDec() < len)
      setDec(len);
    target.setStr(getDec(), source);
    // inutile proseguire, non serve altro
    return true;
    }

  if(getNorm() < 0) {
    target.lAddr = getAddr();
    for(uint i = 0; i < 32 / getnBit(); ++i) {
      smartPointerConstString text = owner->getPageStringLang(getDec() + i);
      if(!_tcscmp(text, source))
        target.U.dw = i << getOffs();
      }
    target.typeVar = prfData::tDWData;
    return true;
    }

  switch(getType()) {

    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
      unformat_data(source, target.U.ft, whichData());
      break;

    case prfData::tHour:    // 64 bit per ora
      unformat_time(source, target.U.ft);
      break;
    case prfData::tFRData:
    case prfData::tRData:
      BaseNum = ebTen;   // con i numeri real la base è sempre dieci
    default:
      if(ebTen == BaseNum) {
        LPTSTR dummy;
        REALDATA input = _tcstod(source, &dummy);
        return getDeNormalized(target, input);
        }
      else {
        REALDATA input;
        switch(BaseNum) {
          case ebBinary:
            input = getBinary(source);
            return getDeNormalized(target, input);
          case ebHex:
            input = getHex(source);
            return getDeNormalized(target, input);
          }
        }
    }

  target.lAddr = getAddr();
  target.typeVar = getType();
  return true;
}
//----------------------------------------------------------------------------
void pVariable::fillAttr(prfData& data, uint addrOffs)
{
  data.lAddr = Addr + addrOffs;

  if(prfData::tStrData == Type)
    data.setStr(nBit);
  else {
    data.typeVar = Type;
    if(prfData::tBitData == Type)
      data.U.dw = MAKELONG(nBit, Offs);
    else
      data.U.li.QuadPart = 0;
    }
}
//----------------------------------------------------------------------------
void replaceWithPoint(LPTSTR buff)
{
   TCHAR decpoint[8];
   getLocaleDecimalPoint(decpoint, SIZE_A(decpoint));
   if(_T('.') != decpoint[0]) {
     LPTSTR p = buff;
     while(*p) {
       if(decpoint[0] == *p)
         *p = _T('.');
       ++p;
       }
     }
}
//----------------------------------------------------------------------------
void replaceWithComma(LPTSTR buff)
{
   TCHAR decpoint[8];
   getLocaleDecimalPoint(decpoint, SIZE_A(decpoint));
   if(_T('.') != decpoint[0]) {
     LPTSTR p = buff;
     while(*p) {
       if(_T('.') == *p)
         *p = decpoint[0];
       ++p;
       }
     }
}
//----------------------------------------------------------------------------
void makeFixedStringConvertPoint(REALDATA rVal, LPTSTR buffer, size_t lenBuff, uint ndec)
{
  makeFixedString(rVal, buffer, lenBuff, ndec);
  if(ndec)
    replaceWithPoint(buffer);
}
//----------------------------------------------------------------------------
void makeFixedStringUConvertPoint(REALDATA rVal, LPTSTR buffer, size_t lenBuff, uint ndec)
{
  makeFixedStringU(rVal, buffer, lenBuff, ndec);
  if(ndec)
    replaceWithPoint(buffer);
}
//----------------------------------------------------------------------------
void pVariable::makeString(LPTSTR buffer, size_t lenBuffer, prfData& data)
{
  // se norm == zero deve tornare uno
  REALDATA nrm = ::getNorm(getNorm());
  REALDATA val;
  uint ndec = Dec;
  if(ndec > 5)
    ndec = 1;
  switch(Type) {

    case prfData::tBData:
      shiftAndMakeStringU(data.U.b, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;
    case prfData::tBsData:
      shiftAndMakeString(data.U.sb, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;

    case prfData::tWData:
      shiftAndMakeStringU(data.U.w, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;
    case prfData::tWsData:
      shiftAndMakeString(data.U.sw, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;

    case prfData::tDWData:
      shiftAndMakeStringU(data.U.dw, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;
    case prfData::tDWsData:
      shiftAndMakeString(data.U.sdw, nBit, Offs, ndec, nrm, buffer, lenBuffer, BaseNum);
      break;

  // negli altri casi non ha senso trattare la word come pacchetto di bit

    case prfData::tBitData: // il campo di bit viene trattato come dword
      if(Neg)
        data.U.dw = ~data.U.dw;
      val = getNormalized(data.U.dw, nrm);
      switch(BaseNum) {
        case ebTen:
          makeFixedStringU(val, buffer, lenBuffer, ndec);
          break;
        case ebBinary:
          makeBinaryString(ROUND_REAL(val), buffer, lenBuffer);
          break;
        case ebHex:
          makeHexString(ROUND_REAL(val), buffer, lenBuffer);
          break;
        }
      break;

    case prfData::tFRData:
      val = getNormalized(data.U.fw, nrm);
      makeFixedStringConvertPoint(val, buffer, lenBuffer, ndec);
      break;

    case prfData::ti64Data: // 64 bit
      val = getNormalized(data.U.li.QuadPart, nrm);
      makeFixedString(val, buffer, lenBuffer, ndec);
      break;

    case prfData::tRData:   // 64 bit float
      val = getNormalized(data.U.rw, nrm);
      makeFixedStringConvertPoint(val, buffer, lenBuffer, ndec);
      break;

    case prfData::tDateHour:// 64 bit per data + ora
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else
        set_format_data(buffer, lenBuffer, data.U.ft, whichData(), _T(" - "));
      break;

    case prfData::tDate:    // 64 bit per data
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else {
        set_format_data(buffer, lenBuffer, data.U.ft, whichData(), _T(" "));
        buffer[10] = 0;
        }
      break;

    case prfData::tHour:    // 64 bit per ora
      set_format_time(buffer, lenBuffer, data.U.ft);
      break;

    case prfData::tStrData:
      copyStrZ(buffer, data.U.str.buff, data.U.str.len + 1);
      break;
    default:
      buffer[0] = 0;
      break;
    }
}
//----------------------------------------------------------------------------
bool pVisibility::isVisible(P_BaseBody* owner)
{
  prfData data;
  if(prfData::okData > getData(owner, data))
    return true;
  bool visib = toBool(data.U.dw);
  if(!getnBit())
    visib = getOffs() == data.U.dw;
  return getDec() ? !visib : visib;
}
//----------------------------------------------------------------------------
void manageObjId::adjustId()
{
  uint id;
  if(getBaseId() < 1000)
    id = getId() % 10;
  else
    id = getId() % 100;
  Id = getBaseId() + id - 1;
}
//----------------------------------------------------------------------------
uint manageObjId::calcBase(uint id)
{
  if(OFFS_INIT_VAR_EXT <= id)
    return ((id - OFFS_INIT_VAR_EXT) / 1000) * 100 + 1;
  if(OFFS_INIT_SIMPLE_BMP <= id)
    return ID_INIT_BMP;
  if(id < 1000)
    return (id / 100) * 100 + 1;
  return (id / 1000) * 1000 + 1;
}
//----------------------------------------------------------------------------
uint manageObjId::calcBaseExtendId(uint ix)
{
  switch(getBaseId()) {
    case ID_INIT_BARGRAPH:
      if(ix < GRAPH_VISIBILITY_OFFS)
        return getBaseId() + ix;
      return ix + ADD_INIT_GRAPH + 1;

    case ID_INIT_BMP:
      if(ix < ID_INIT_BARGRAPH - ID_INIT_BMP)
        return ix + getBaseId();
      return ix + OFFS_INIT_SIMPLE_BMP + 1 - (ID_INIT_BARGRAPH - ID_INIT_BMP);
    }
  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(ix < idSec)
    return ix + getBaseId();

  return ix + (getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT - ADD_INIT_VAR;
}
//----------------------------------------------------------------------------
uint manageObjId::calcAndSetExtendId(uint ix)
{
  Id = calcBaseExtendId(ix);
  return Id;
}
//----------------------------------------------------------------------------
uint manageObjId::getFirstExtendId()
{
  if(ID_INIT_BARGRAPH == getBaseId()) {
    if(getId() < getBaseId() + GRAPH_VISIBILITY_OFFS)
      return getId() + GRAPH_VISIBILITY_OFFS;
    return getId() - getBaseId() + ADD_INIT_GRAPH + 1;
    }
  if(ID_INIT_BMP == getBaseId()) {
    int ix = getId();
    if(ix >= OFFS_INIT_SIMPLE_BMP)
      ix -= OFFS_INIT_SIMPLE_BMP - 1 - ID_INIT_BMP;
    return ix + ADD_INIT_SIMPLE_BMP;
    }

  if(ID_INIT_SIMPLE_PANEL == getBaseId())
    return getId() + ADD_INIT_VAR * 2;

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR;

  return getId() + ADD_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
uint manageObjId::getSecondExtendId()
{
  if(ID_INIT_BMP == getBaseId())
    return getFirstExtendId();

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
#define MAX_THIRD_OFFSET MAX_NORMAL_OPEN_MODELESS
//----------------------------------------------------------------------------
uint manageObjId::getThirdExtendId()
{
  if(ID_INIT_BMP == getBaseId())
    return getFirstExtendId();
  if(ID_INIT_VAR_BTN == getBaseId()) {
    if(getId() < MAX_NORMAL_OPEN_MODELESS)
      return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;
    // se l'id è già esteso occorre ricavare l'indice aumentato di ADD_INIT_VAR prima di sommarlo all'offset
    if(getId() > OFFS_INIT_VAR_EXT)
      return getId() - ((getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT) + OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN + ADD_INIT_VAR;
    return getId() + OFFSET_BTN_OPEN_MODELES;
    }

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : MAX_THIRD_OFFSET;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT + ADD_INIT_THIRD_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
baseObj::baseObj(P_BaseBody* owner, uint id) : Owner(owner), Id(id),
    Visibility(0), Visible(true), BlinkObj(0), MoveInfo(0) { }
//----------------------------------------------------------------------------
baseObj::~baseObj()
{
  delete Visibility;
  delete BlinkObj;
  delete MoveInfo;
}
//----------------------------------------------------------------------------
void baseObj::setRect(const PRect& r)
{
  Rect = r;
}
//----------------------------------------------------------------------------
void baseObj::invalidate(const PRect& r, bool alsoBkg)
{
  InvalidateRect(*Owner, r, alsoBkg);
}
//----------------------------------------------------------------------------
void baseObj::invalidate(bool alsoBkg)
{
  if(!get_Rect().Width() || !get_Rect().Height())
    return;
  InvalidateRect(*Owner, get_Rect(), alsoBkg);
}
//----------------------------------------------------------------------------
bool baseObj::checkVisibility(bool force)
{
  bool chg = isVisible() ^ needVisible();
  if(chg) {
    setVisibility(!isVisible());
    if(BlinkObj)
      BlinkObj->setShow(isVisible());
    invalidate();
    if(MoveInfo)
      MoveInfo->updateMoving(force);
    return true;
    }
  if(MoveInfo)
    MoveInfo->updateMoving(force);
  if(BlinkObj)
    force |= BlinkObj->update();
  return force;
}
//----------------------------------------------------------------------------
void baseObj::allocBlink(DWORD tick)
{
  delete BlinkObj;
  BlinkObj = new blinkingObj(tick);
}
//----------------------------------------------------------------------------
void baseObj::allocVisibility(LPCTSTR p)
{
  p = findNextParamTrim(p, 2);
  if(!p)
    return;
  uint next = 6;
  do {
    uint prph = _ttoi(p);
    if(!prph)
      break;
    p = findNextParamTrim(p, 1);
    if(!p)
      return;
    uint addr = _ttoi(p);
    p = findNextParamTrim(p, 1);
    if(!p)
      return;
    uint nbit = _ttoi(p);
    p = findNextParamTrim(p, 1);
    if(!p)
      return;
    uint offs = _ttoi(p);
    p = findNextParamTrim(p, 1);
    if(!p)
      return;
    uint neg = _ttoi(p);

    delete Visibility;
    Visibility = new pVisibility(prph, addr, nbit, offs, neg);
    next = 2;
    } while(false);

  p = findNextParamTrim(p, next);
  if(p) {
    int code = _ttoi(p);
    if(code) {
      p = getOwner()->getPageString(code);
      delete MoveInfo;
      MoveInfo = new pInfoMoveObj(this);
      if(!MoveInfo->init(p)) {
        delete MoveInfo;
        MoveInfo = 0;
        }
      }
    }
}
//----------------------------------------------------------------------------
void baseObj::draw(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(!MoveInfo && !rect.Intersec(get_Rect()))
    return;
  if(BlinkObj && blinkingObj::onShow != BlinkObj->getStatus())
    return;
  if(MoveInfo)
    performDraw(hdc, MoveInfo->getLast());
  else
    performDraw(hdc);
}
//----------------------------------------------------------------------------
void baseObj::performDraw(HDC hdc, const POINT& offset)
{
  PRect old(get_Rect());
  PRect r(old);
  r.Offset(offset.x, offset.y);
  setRect(r);
  performDraw(hdc);
  setRect(old);
}
//----------------------------------------------------------------------------
void baseObj::addReqVar(PVect<P_Bits*>& allBits)
{
  if(Visibility)
    addReqVar2(allBits, *Visibility);
  if(MoveInfo) {
    const infoData* Data = MoveInfo->getInfo();
    for(int i = 0; i < pInfoMoveObj::MAX_MOVING_DATA; ++i)
      addReqVar2(allBits, Data[i].prph, Data[i].data.lAddr);
    }
}
//----------------------------------------------------------------------------
void baseObj::addReqVar2(PVect<P_Bits*>& allBits, int prph, uint addr)
{
  prph -= WM_PLC;
  if(prph < 0 || prph >= MAX_PERIF)
    return;
  allBits[prph]->set(addr);
}
//----------------------------------------------------------------------------
void baseObj::addReqVar2(PVect<P_Bits*>& allBits, const pVariable& pv, int offs_addr)
{
  int prph = pv.getPrph() - WM_PLC;
  if(prph < 0 || prph >= MAX_PERIF)
    return;
  offs_addr += pv.getAddr();
  allBits[prph]->set(offs_addr);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void baseVar::makeVar(pVariable& target, LPCTSTR p1, LPCTSTR p2, bool useRealForNorm)
{
  while(p1) {
    int val = _ttoi(p1);
    target.setPrph(val);
    p1 = findNextParamTrim(p1, 1);
    if(!p1)
      break;
    if(PRPH_4_CONST == val) {
      target.makeConst(p1);
      return;
      }
    val = _ttoi(p1);
    target.setAddr(val);

    p1 = findNextParamTrim(p1, 1);
    if(!p1)
      break;
    val = _ttoi(p1);
    if(val & prfData::tNegative) {
      target.setNeg(true);
      val &= ~prfData::tNegative;
      }
    else
      target.setNeg(false);
    target.setType(val);

    p1 = findNextParamTrim(p1, 1);
    if(!p1)
      break;

    switch(target.getType()) {
      case prfData::tFRData:
      case prfData::tRData:
        if(useRealForNorm) {
          double v = _tstof(p1);
          target.setNorm(*(__int64*)&v);
          break;
          }
        // fall through
      default:
        do {
          __int64 v = _tstoi64(p1);
          target.setNorm(v);
          } while(false);
        break;
      }

    p1 = findNextParamTrim(p1, 1);
    if(!p1)
      break;
    val = _ttoi(p1);
    target.setDec(val);

    // il successivo è stato usato in altri casi, quindi occorre saltarlo
    p1 = findNextParamTrim(p1, 2);
    if(!p1)
      break;
    val = _ttoi(p1);
    target.setBaseNum(val);
    break;
    }

  while(p2) {
    int val = _ttoi(p2);
    target.setnBit(val);
    p2 = findNextParamTrim(p2, 1);
    if(!p2)
      break;
    val = _ttoi(p2);
    target.setOffs(val);
    break;
    }
}
//----------------------------------------------------------------------------
void baseVar::makeOtherVar(pVariable& var, uint id)
{
  manageObjId moi(id);
  LPCTSTR p1 = getOwner()->getPageString(moi.getFirstExtendId());
  LPCTSTR p2 = getOwner()->getPageString(moi.getSecondExtendId());
  makeVar(var, p1, p2);
}
//----------------------------------------------------------------------------
bool baseVar::makeStdVars(bool useRealForNorm)
{
  manageObjId moi(getId(), getBaseId());
  uint idAdd = moi.getFirstExtendId();

  LPCTSTR p1 = getOwner()->getPageString(idAdd);
//  LPCTSTR p1 = getOwner()->getPageString(getId() + ADD_INIT_VAR);
  bool success = toBool(p1);

  idAdd = moi.getSecondExtendId();
  LPCTSTR p2 = getOwner()->getPageString(idAdd);
//  LPCTSTR p2 = getOwner()->getPageString(getId() + ADD_INIT_VAR + ADD_INIT_SECOND);
  success &= toBool(p2);

  allocVisibility(p2);
  makeVar(BaseVar, p1, p2, useRealForNorm);
  return success;
}
//----------------------------------------------------------------------------
void baseActive::invalidate(bool alsoBkg)
{
  if(getHwnd())
    InvalidateRect(getHwnd(), 0, alsoBkg);
  baseClass::invalidate(alsoBkg);
}
//----------------------------------------------------------------------------
void baseActive::invalidateWithBorder()
{
  if(getHwnd())
    InvalidateRect(getHwnd(), 0, true);
  PRect r = get_Rect();
  r.Inflate(R__X(3), R__Y(3));
  baseClass::invalidate(r, true);
}
//----------------------------------------------------------------------------
void baseActive::draw(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  if(MoveInfo)
    performDraw(hdc, MoveInfo->getLast());
}
//----------------------------------------------------------------------------
void baseActive::performDraw(HDC hdc, const POINT& offset)
{
  if(getHwnd()) {
    PRect r;
    GetWindowRect(getHwnd(), r);
    PRect r2(r);
    MapWindowPoints(getHwnd(), *getOwner(), (LPPOINT)(LPRECT)r2, 2);
    POINT orig = MoveInfo->getOrig();
    r.MoveTo(orig.x + offset.x, orig.y + offset.y);
    if(r != r2)
      SetWindowPos(getHwnd(), 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
    }
}
//----------------------------------------------------------------------------
bool baseActive::update(bool force)
{
  bool result = baseClass::update(force);
  if(isVisible() && MoveInfo)
    performDraw(0, MoveInfo->getLast());
  return result;
}
//----------------------------------------------------------------------------
bool verifyRange(uint idNorm, const prfData& data)
{
  if(idNorm > 0) {
    REALDATA val = 0.0;
    switch(data.typeVar) {
      case prfData::tFRData:
        val = data.U.fw;
        break;
      case prfData::tRData:
        val = data.U.rw;
        break;

      case prfData::tWData:
        val = data.U.w;
        break;
      case prfData::tWsData:
        val = data.U.sw;
        break;
      case prfData::tDWData:
        val = data.U.dw;
        break;
      case prfData::tDWsData:
        val = data.U.sdw;
        break;
      default:
        return true;
      }
    val *= ::getNorm(idNorm);
    return isOnRange((fREALDATA)val, idNorm);
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pAround::pAround()
{
  for(int i = 0; i < maxAround; ++i)
    Obj[i] = 0;
}
//----------------------------------------------------------------------------
void pAround::setFocus(around where)
{
  if(Obj[where]) {
    HWND hwnd = Obj[where]->getHwnd();
    if(hwnd) {
      if(!IsWindowEnabled(hwnd)) {
        Obj[where]->Around.setFocus(where);
        return;
        }
      SetFocus(hwnd);
      PEdit* ed = dynamic_cast<PEdit*>(PWin::getWindowPtr(hwnd));
      if(ed)
        PostMessage(hwnd, EM_SETSEL, 0, (LPARAM) -1);
      }
    }
  else {
    switch(where) {
      case aLeft:
        if(Obj[aTop])
          setFocus(aTop);
        else if(Obj[aBottom])
          setFocus(aBottom);
        break;
      case aRight:
        if(Obj[aBottom])
          setFocus(aBottom);
        else if(Obj[aTop])
          setFocus(aTop);
        break;
      case aTop:
        if(Obj[aLeft])
          setFocus(aLeft);
        else if(Obj[aRight])
          setFocus(aRight);
        break;
      case aBottom:
        if(Obj[aRight])
          setFocus(aRight);
        else if(Obj[aLeft])
          setFocus(aLeft);
        break;
      }
    }
}
//----------------------------------------------------------------------------
HWND pAround::getHwnd(around where)
{
  HWND hwnd = 0;
  baseActive* obj = Obj[where];
  while(obj) {
    hwnd = obj->getHwnd();
    if(hwnd && IsWindowEnabled(hwnd))
      break;
    obj = obj->Around.Obj[where];
    }
  return hwnd;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//#define CALC_CENTER(r, vm, vM) (r.vm)
#define CALC_CENTER(r, vm, vM) ((r.vm + r.vM)/2)
//----------------------------------------------------------------------------
bool wrapObjX::operator <(const wrapObjX& other) const
{
  PRect r = Owner->get_Rect();
  PRect r2 = other.Owner->get_Rect();
  return CALC_CENTER(r, left, right) < CALC_CENTER(r2, left, right);
}
//----------------------------------------------------------------------------
bool wrapObjY::operator <(const wrapObjY& other) const
{
  PRect r = Owner->get_Rect();
  PRect r2 = other.Owner->get_Rect();
  return CALC_CENTER(r, top, bottom) < CALC_CENTER(r2, top, bottom);
}

//----------------------------------------------------------------------------
void pAround::setJoin(around where, baseActive* sender, baseActive* join)
{
  sender->Around.Obj[where] = join;
}
//----------------------------------------------------------------------------
#define AROUND_DIM_SEARCH_OBJ 3
#define MAX_DIST 500
//----------------------------------------------------------------------------
void pAround::findAround(baseActive* sender, const PVect<wrapObjX>& wrapX, const PVect<wrapObjY>& wrapY)
{
  wrapObjX tmpX(sender);
  uint posX = 0;
  int nElem = wrapX.getElem();
  // trova se stesso
  if(!wrapX.find(tmpX, posX)) // ??
    return;

  wrapObjY tmpY(sender);
  uint posY = 0;
  // trova se stesso
  if(!wrapY.find(tmpY, posY)) // ??
    return;

  PVect<pVActive> objects;
  objects.setDim(4);

  int ix = int(posX) - 1;
  PRect r = sender->get_Rect();
  uint j = 0;
  while(ix >= 0) {
    baseActive* obj = wrapX[ix].Owner;
    if(r.left + 1 >= obj->get_Rect().right) {
//      if(r.left - obj->get_Rect().right > MAX_DIST && j > 2)
//        break;
      objects[0][j++] = obj;
      }
    --ix;
    }
  ix = int(posX) + 1;
  j = 0;
  while(ix < nElem) {
    baseActive* obj = wrapX[ix].Owner;
    if(r.right <= obj->get_Rect().left  + 1) {
//      if(obj->get_Rect().left - r.right > MAX_DIST && j > 2)
//        break;
      objects[1][j++] = obj;
      }
    ++ix;
    }
  ix = int(posY) - 1;
  j = 0;
  while(ix >= 0) {
    baseActive* obj = wrapY[ix].Owner;
    if(r.top + 1 >= obj->get_Rect().bottom) {
//      if(obj->get_Rect().bottom - r.top > MAX_DIST && j > 2)
//        break;
      objects[2][j++] = obj;
      }
    --ix;
    }
  j = 0;
  ix = int(posY) + 1;
  while(ix < nElem) {
    baseActive* obj = wrapY[ix].Owner;
    if(r.bottom <= obj->get_Rect().top + 1) {
//      if(obj->get_Rect().top - r.bottom > MAX_DIST && j > 2)
//        break;
      objects[3][j++] = obj;
      }
    ++ix;
    }
  findAround(sender, objects);
}
//----------------------------------------------------------------------------
void pAround::findAround(baseActive* sender, PVect<pVActive>& objSet)
{
  rectAround rA;

  if(Obj[aLeft]) {
    rA.left.found = true;
    rA.left.r = Obj[aLeft]->get_Rect();
    rA.left.obj = Obj[aLeft];
    }
  if(Obj[aRight]) {
    rA.right.found = true;
    rA.right.r = Obj[aRight]->get_Rect();
    rA.right.obj = Obj[aRight];
    }
  if(Obj[aTop]) {
    rA.top.found = true;
    rA.top.r = Obj[aTop]->get_Rect();
    rA.top.obj = Obj[aTop];
    }
  if(Obj[aBottom]) {
    rA.bottom.found = true;
    rA.bottom.r = Obj[aBottom]->get_Rect();
    rA.bottom.obj = Obj[aBottom];
    }

  PRect rectSender = sender->get_Rect();
  do {
    pVActive& obj = objSet[0];
    int nElem = obj.getElem();
    for(int i = 0; i < nElem; ++i) {
//      if(obj[i] == sender)
//        continue;
      findLeft(rA, rectSender, obj[i]);
      }
    } while(false);

  do {
    pVActive& obj = objSet[1];
    int nElem = obj.getElem();
    for(int i = 0; i < nElem; ++i) {
//      if(obj[i] == sender)
//        continue;
      findRight(rA, rectSender, obj[i]);
      }
    } while(false);

  do {
    pVActive& obj = objSet[2];
    int nElem = obj.getElem();
    for(int i = 0; i < nElem; ++i) {
//      if(obj[i] == sender)
//        continue;
      findTop(rA, rectSender, obj[i]);
      }
    } while(false);

  do {
    pVActive& obj = objSet[3];
    int nElem = obj.getElem();
    for(int i = 0; i < nElem; ++i) {
//      if(obj[i] == sender)
//        continue;
      findBottom(rA, rectSender, obj[i]);
      }
    } while(false);

  bool foundAll = true;
  if(rA.left.found)
    setJoin(aLeft, sender, rA.left.obj);
  else
    foundAll = false;

  if(rA.top.found)
    setJoin(aTop, sender, rA.top.obj);
  else
    foundAll = false;

  if(rA.right.found)
    setJoin(aRight, sender, rA.right.obj);
  else
    foundAll = false;

  if(rA.bottom.found)
    setJoin(aBottom, sender, rA.bottom.obj);
  else
    foundAll = false;

  if(!foundAll) {
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static inline long getXDistL(const PRect& r1, const PRect& r2) {  return r1.left - r2.right;  }
//----------------------------------------------------------------------------
static inline long getXDistR(const PRect& r1, const PRect& r2) {  return r2.left - r1.right; }
//----------------------------------------------------------------------------
static inline long getXDistTB(const PRect& r1, const PRect& r2) {  return r1.left - r2.left; }
//----------------------------------------------------------------------------
static inline long getYDistT(const PRect& r1, const PRect& r2) {  return r1.top - r2.bottom; }
//----------------------------------------------------------------------------
static inline long getYDistB(const PRect& r1, const PRect& r2) {  return r2.top - r1.bottom; }
//----------------------------------------------------------------------------
static inline long getYDistLR(const PRect& r1, const PRect& r2) {  return r1.top - r2.top; }
//----------------------------------------------------------------------------
template <int priorX, int priorY>
long calcDistL(const PRect& r1, const PRect& r2)
{
  long c1 = getXDistL(r1, r2);
  long c2 = getYDistLR(r1, r2);
  return c1 * c1 * priorX + c2 * c2 * priorY;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
long calcDistT(const PRect& r1, const PRect& r2)
{
  long c1 = getXDistTB(r1, r2);
  long c2 = getYDistT(r1, r2);
  return c1 * c1 * priorX + c2 * c2 * priorY;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
long calcDistR(const PRect& r1, const PRect& r2)
{
  long c1 = getXDistR(r1, r2);
  long c2 = getYDistLR(r1, r2);
  return c1 * c1 * priorX + c2 * c2 * priorY;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
long calcDistB(const PRect& r1, const PRect& r2)
{
  long c1 = getXDistTB(r1, r2);
  long c2 = getYDistB(r1, r2);
  return c1 * c1 * priorX + c2 * c2 * priorY;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
bool needChangeLeft(const PRect& currR, const PRect& newR, const PRect& oldR)
{
  long distNew = calcDistL<priorX, priorY>(currR, newR);
  long distOld = calcDistL<priorX, priorY>(currR, oldR);
  return distNew < distOld;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
bool needChangeRight(const PRect& currR, const PRect& newR, const PRect& oldR)
{
  long distNew = calcDistR<priorX, priorY>(currR, newR);
  long distOld = calcDistR<priorX, priorY>(currR, oldR);
  return distNew < distOld;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
bool needChangeTop(const PRect& currR, const PRect& newR, const PRect& oldR)
{
  long distNew = calcDistT<priorX, priorY>(currR, newR);
  long distOld = calcDistT<priorX, priorY>(currR, oldR);
  return distNew < distOld;
}
//----------------------------------------------------------------------------
template <int priorX, int priorY>
bool needChangeBottom(const PRect& currR, const PRect& newR, const PRect& oldR)
{
  long distNew = calcDistB<priorX, priorY>(currR, newR);
  long distOld = calcDistB<priorX, priorY>(currR, oldR);
  return distNew < distOld;
}
//----------------------------------------------------------------------------
#define MIN_PRIOR 5
#define MAX_PRIOR 1
//----------------------------------------------------------------------------
#define CHECK_OBJ(a, b) \
  if(!rA.b.found) {\
    rA.b.r = rect;\
    rA.b.obj = testing;\
    rA.b.found = true;\
    return;\
    }
//----------------------------------------------------------------------------
void pAround::findLeft(rectAround& rA, const PRect& rectSender, baseActive* testing)
{
  PRect rect = testing->get_Rect();
  if(getXDistL(rectSender, rect) < -1)
    return;

  CHECK_OBJ(aLeft, left)
  if(needChangeLeft<MAX_PRIOR, MIN_PRIOR>(rectSender, rect, rA.left.r))
  {
    rA.left.r = rect;
    rA.left.obj = testing;
    }
}
//----------------------------------------------------------------------------
void pAround::findRight(rectAround& rA, const PRect& rectSender, baseActive* testing)
{
  PRect rect = testing->get_Rect();
  if(getXDistR(rectSender, rect) < -1)
    return;

  CHECK_OBJ(aRight, right)

  if(needChangeRight<MAX_PRIOR, MIN_PRIOR>(rectSender, rect, rA.right.r)) {
    rA.right.r = rect;
    rA.right.obj = testing;
    }
}
//----------------------------------------------------------------------------
void pAround::findTop(rectAround& rA, const PRect& rectSender, baseActive* testing)
{
  PRect rect = testing->get_Rect();
  if(getYDistT(rectSender, rect) < -1)
    return;

  CHECK_OBJ(aTop, top)
  if(needChangeTop<MIN_PRIOR, MAX_PRIOR>(rectSender, rect, rA.top.r)) {
    rA.top.r = rect;
    rA.top.obj = testing;
    }
}
//----------------------------------------------------------------------------
void pAround::findBottom(rectAround& rA, const PRect& rectSender, baseActive* testing)
{
  PRect rect = testing->get_Rect();
  if(getYDistB(rectSender, rect) < -1)
    return;

  CHECK_OBJ(aBottom, bottom)

  if(needChangeBottom<MIN_PRIOR, MAX_PRIOR>(rectSender, rect, rA.bottom.r)) {
    rA.bottom.r = rect;
    rA.bottom.obj = testing;
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void baseColor::makeColors(P_BaseBody* owner, uint idInit)
{
  Colors.reset();
  for(uint i = 0;; ++i) {
    LPCTSTR p = owner->getPageString(idInit++);
    if(!p)
      break;
    int col[6];
    memset(col, 0, sizeof(col));
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
          &col[0], &col[1], &col[2], &col[3], &col[4], &col[5]);
    Colors[i].fg = RGB(col[0], col[1], col[2]);
    Colors[i].bg = RGB(col[3], col[4], col[5]);
    }
}
//----------------------------------------------------------------------------
bool baseColor::getColors(COLORREF& fg, COLORREF& bg, uint ix)
{
  const PVect<colors>& C = Colors;
  findIxByVal<colors, uint> f(C, ix);
  int result = f.getIx();
  if(result < 0)
    return false;
  fg = C[result].fg;
  bg = C[result].bg;
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void baseVarColor::makeColors(P_BaseBody* owner, uint idInit)
{
  ValColors.reset();
  LPCTSTR p = owner->getPageString(idInit++);
  if(!p)
    return;
  uint nElem = _ttoi(p);
  for(uint i = 0; i < nElem; ++i) {
    p = owner->getPageString(idInit++);
    if(!p)
      break;
    int col[6];
    memset(col, 0, sizeof(col));
    ValColors[i].Val = _tstof(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
          &col[0], &col[1], &col[2], &col[3], &col[4], &col[5]);
    ValColors[i].fg = RGB(col[0], col[1], col[2]);
    ValColors[i].bg = RGB(col[3], col[4], col[5]);
    }
}
//----------------------------------------------------------------------------
void baseVarColor::makeColorsOnRow(P_BaseBody* owner, uint idInit, uint step)
{
  ValColors.reset();
  LPCTSTR p = owner->getPageString(idInit);
  if(!p)
    return;
  if(step < 2)
    step = 4;
  uint i = 0;
  while(p)  {
    ValColors[i].Val = _tstof(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int r = 255;
    int g = 255;
    int b = 255;
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
    ValColors[i].fg = RGB(r, g, b);
    ValColors[i].bg = RGB(r, g, b);
    p = findNextParam(p, step - 1);
    ++i;
    }
}
//----------------------------------------------------------------------------
void baseVarColor::makeFullColorsOnRow(P_BaseBody* owner, uint idInit, uint step)
{
  ValColors.reset();
  LPCTSTR p = owner->getPageString(idInit);
  if(!p)
    return;
  if(step < 2)
    step = 7;
  uint i = 0;
  while(p)  {
    ValColors[i].Val = _tstof(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int fr = 0;
    int fg = 0;
    int fb = 0;
    int br = 255;
    int bg = 255;
    int bb = 255;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &fr, &fg, &fb, &br, &bg, &bb);
    ValColors[i].fg = RGB(fr, fg, fb);
    ValColors[i].bg = RGB(br, bg, bb);
    p = findNextParam(p, step - 1);
    ++i;
    }
}
//----------------------------------------------------------------------------
bool baseVarColor::getReverseColors(COLORREF& fg, COLORREF& bg, double val)
{
  uint nElem = ValColors.getElem();
  if(!nElem)
    return false;

  const PVect<val_colors>& C = ValColors;
  for(uint i = 0; i < nElem; ++i) {
    if(val >= C[i].Val) {
      fg = C[i].fg;
      bg = C[i].bg;
      return true;
      }
    }
  --nElem;
  fg = C[nElem].fg;
  bg = C[nElem].bg;
  return true;
}
//----------------------------------------------------------------------------
bool baseVarColor::getColors(COLORREF& fg, COLORREF& bg, double val, P_BaseBody* owner, uint idText, LPTSTR buff, size_t lenBuff)
{
  const PVect<val_colors>& C = ValColors;
  findIxByVal<val_colors, double> f(C, val);
  int result = f.getPos();
  if(result < 0) {
    if(buff)
      makeFixedString(val, buff, lenBuff);
    return false;
    }
  lastIx = result;
  fg = C[result].fg;
  bg = C[result].bg;
  if(buff) {
    smartPointerConstString sp = owner->getPageStringLang(idText + result);
    if(sp)
      _tcscpy_s(buff, lenBuff, sp);
    else
      makeFixedString(val, buff, lenBuff);
    }
  return true;
}
//----------------------------------------------------------------------------
bool baseVarColor::getExactColors(COLORREF& fg, COLORREF& bg, double val, P_BaseBody* owner, uint idText, LPTSTR buff, size_t lenBuff)
{
  const PVect<val_colors>& C = ValColors;
  findIxByVal<val_colors, double> f(C, val);
  int result = f.getExact();
  if(buff)
    *buff = 0;
  if(result < 0) {
    fg = RGB(0, 0, 0);
    bg = RGB(255, 255, 255);
    if(buff)
      makeFixedString(val, buff, lenBuff);
    return false;
    }
  lastIx = result;
  fg = C[result].fg;
  bg = C[result].bg;
  if(buff) {
    smartPointerConstString sp = owner->getPageStringLang(idText + result);
    if(sp)
      _tcscpy_s(buff, lenBuff, sp);
    else
      makeFixedString(val, buff, lenBuff);
    }
  return true;
}
//----------------------------------------------------------------------------
void blinkingObj::toggle()
{
  if(hide == Show)
    return;
  if(notShow == Show)
    Show = onShow;
  else
    Show = notShow;
}
//----------------------------------------------------------------------------
bool blinkingObj::update()
{
  if(hide == Show)
    return false;
  DWORD tick = GetTickCount();
  if(tick - lastTick > Tick) {
    lastTick = tick;
    toggle();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
