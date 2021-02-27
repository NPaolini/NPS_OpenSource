//-------- sv_manage_vars.cpp -----------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "sv_manage_vars.h"
#include "replaceVarName.h"
#include <stdio.h>
#include <stdlib.h>
//------------------------------------------------------------------
manExpressionVars* allocManVars()
{
  return new svManVars;
}
//------------------------------------------------------------------
template <typename T>
int tCmp(const T& v, const T& ins)
{
  return ins.id - v.id;
}
//------------------------------------------------------------------
SV_prfData::tData svRetrieveTypeByPrph::getDataType(uint idPrph)
{
  if(SV_prfData::tNoData == Type) {
    Type = (SV_prfData::tData)npSV_GetBodyRoutine(eSV_GET_DEF_TYPE_PRPH, (LPDWORD)idPrph, 0);
    if(SV_prfData::tNoData == Type)
      Type = SV_prfData::tDWData;
    }
  return Type;
}
//------------------------------------------------------------------
manageBuff prph_Data::mBuff;
//------------------------------------------------------------------
__int64 prph_Data::getValue_p(const infoVar& ivar) const
{
  if((uint)(ivar.addr + Offset) >= DIM_DAT)
    return 0;
  infoDat& idat = mBuff[ivar.prph];
  __int64 result = *(__int64*)&idat[ivar.addr + Offset];

  SV_prfData data;
  data.U.li.QuadPart = result;

  switch(ivar.type) { // verificare se serve gestire il valore negativo
    case SV_prfData::tBData:
      result = data.U.b;
      break;
    case SV_prfData::tBsData:
      result = data.U.sb;
      break;

    case SV_prfData::tWData:
      result = data.U.w;
      break;
    case SV_prfData::tWsData:
      result = data.U.sw;
      break;

    case SV_prfData::tBitData: // il campo di bit viene trattato come dword
    case SV_prfData::tDWData:
    case SV_prfData::tFRData:
      result = data.U.dw;
      break;
    case SV_prfData::tDWsData:
      result = data.U.sdw;
      break;

    case SV_prfData::ti64Data: // 64 bit
    case SV_prfData::tRData:   // 64 bit float
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      break;
    default:
      result = 0;
    }
  return result;
}
//------------------------------------------------------------------
void prph_Data::setValue(const infoVar& ivar, __int64 value)
{
  if((uint)(ivar.addr + Offset) >= DIM_DAT)
    return;

  infoDat& idat = mBuff[ivar.prph];

  switch(ivar.type) {
    case SV_prfData::tBData:
    case SV_prfData::tBsData:
    case SV_prfData::tWData:
    case SV_prfData::tWsData:
    case SV_prfData::tBitData: // il campo di bit viene trattato come dword
    case SV_prfData::tDWData:
    case SV_prfData::tDWsData:
      if(ivar.nbit || ivar.offs) {
        DWORD old = idat[ivar.addr + Offset];
        DWORD t = (1 << ivar.nbit) - 1;
        t <<= ivar.offs;
        idat[ivar.addr + Offset] &= ~t;
        t = (DWORD)value;
        t &= (1 << ivar.nbit) - 1;
        t <<= ivar.offs;
        idat[ivar.addr + Offset] |= t;
        if(ivar.prph && old != idat[ivar.addr + Offset])
          toSend = true;
        }
      else {
        if(ivar.prph && idat[ivar.addr + Offset] != (DWORD)value)
          toSend = true;
        idat[ivar.addr + Offset] = (DWORD)value;
        }
      break;

    case SV_prfData::tFRData:
      if(ivar.prph && idat[ivar.addr + Offset] != (DWORD)value)
        toSend = true;
#if 1
        idat[ivar.addr + Offset] = (DWORD)value;
#else
      {
      float v = *(float*)&value;
      idat[ivar.addr + Offset] = *(LPDWORD)&v;
      }
#endif
      break;

    case SV_prfData::ti64Data: // 64 bit
    case SV_prfData::tRData:   // 64 bit float
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      {
      __int64* pd = (__int64*)&(idat[ivar.addr + Offset]);
      if(ivar.prph && *pd != value)
        toSend = true;
      *pd = value;
      }
      break;
    }
}
//------------------------------------------------------------------
void prph_Data::init(const infoVar& ivar)
{
  if(!ivar.prph)
    return;
  uint addr = (uint)(ivar.addr + Offset);
  if(addr >= DIM_DAT)
    return;

  infoDat& idat = mBuff[ivar.prph];

  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = ivar.type;
  npSV_Get(ivar.prph, &data);
  switch(ivar.type) {
    case tag_SV_prfData::tBsData:
      idat[addr] = data.U.sb;
      break;
    case tag_SV_prfData::tWsData:
      idat[addr] = data.U.sw;
      break;
    case tag_SV_prfData::tDWsData:
      idat[addr] = data.U.sdw;
      break;

    case SV_prfData::ti64Data: // 64 bit
    case SV_prfData::tRData:   // 64 bit float
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      {
      __int64* pd = (__int64*)&(idat[addr]);
      *pd = data.U.li.QuadPart;
      }
      break;
    default:
      idat[addr] = data.U.dw;
    }
}
//------------------------------------------------------------------
__int64 prph_Data::getValueFinal(const infoVar& ivar) const
{
  return getValue_p(ivar);
}
//------------------------------------------------------------------
__int64 prph_Data::getValue(const infoVar& ivar) const
{
  __int64 result = getValue_p(ivar);
  if(ivar.nbit || ivar.offs) {
    result >>= ivar.offs;
    result &= (1 << ivar.nbit) - 1;
    }
  return result;
}
//------------------------------------------------------------------
#define ID_SHOW_ERROR 100
#define ID_INIT_SHOW_ERROR 101
#define ID_VAR_NOT_FOUND ID_INIT_SHOW_ERROR
#define ID_EXPR_ERROR 102
//------------------------------------------------------------------
void svManVars::shoMsg(int id, LPCTSTR var, int num2, int row)
{
}
//------------------------------------------------------------------
svManVars::~svManVars()
{
  clearCache();
}
//------------------------------------------------------------------
void svManVars::manageInit(LPTSTR p)
{
  int id = _ttoi(p);
  for(;*p; ++p) {
    if(_T('=') == *p) {
      ++p;
      break;
      }
    }
  if(!*p)
    return;

  infoInit ii;
  ii.id = id;

  int len = _tcslen(p);
  bool isReal = false;
  for(int i = 0; i < len; ++i) {
    if(_T('.') == p[i]) {
      isReal = true;
      break;
      }
    }
  if(isReal) {
    ii.val.dValue = _tstof(p);
    ii.val.type = P_Expr::tvReal;
    }
  else {
    ii.val.iValue = _ttoi64(p);
    ii.val.type = P_Expr::tvInt;
    }
  Inits[Inits.getElem()] = ii;
}
//------------------------------------------------------------------
void svManVars::manageVar(LPTSTR p)
{
  int id = _ttoi(p);
  for(;*p; ++p) {
    if(_T('=') == *p) {
      ++p;
      break;
      }
    }
  if(!*p)
    return;
  infoVar ivar;
  ZeroMemory(&ivar, sizeof(ivar));
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
          &ivar.prph, &ivar.addr, &ivar.type,
          &ivar.nbit, &ivar.offs, &ivar.norm);
  ivar.id = id;
  if(!ivar.prph && !ivar.type) {
    ivar.type = ivar.addr ? SV_prfData::tRData : SV_prfData::ti64Data;
    ivar.addr = getNextCountOnMem();
    }
#if 1 // per togliere la gestione a bit
  if(SV_prfData::tBitData == ivar.type)
    ivar.type = DefDataType[ivar.prph].getDataType(ivar.prph);
#endif

  Vars.insertEx(ivar, tCmp<infoVar>);
}
//------------------------------------------------------------------
static int getTypeVar(int type)
{
  switch(type) {
    case SV_prfData::tFRData:
    case SV_prfData::tRData:
      return 0;
    }
  return 1;
}
//------------------------------------------------------------------
int svManVars::getTypeResult(int id)
{
  infoVar ivar = { id };
  if(getInfoVar(ivar))
    return getTypeVar(ivar.type);
  return -1;
}
//------------------------------------------------------------------
void svManVars::performInit()
{
  uint nElem = Inits.getElem();
  for(uint i = 0; i < nElem; ++i) {
    infoInit& ii = Inits[i];
    infoVar ivar = { ii.id };
    if(!getInfoVar(ivar)) {
      LPCTSTR realName = _T("??");
      if(ReplaceVar)
        realName = ReplaceVar->getName(ii.id);
      shoMsg(0, realName, -3, 0);
      continue;
      }
    P_Expr::exprVar val = ii.val;
    switch(ivar.type) {
      case SV_prfData::tFRData:
      case SV_prfData::tRData:
        if(P_Expr::tvInt == ii.val.type)
          val = P_Expr::exprVar(ii.val.getReal());
        break;
      default:
        if(P_Expr::tvReal == ii.val.type)
          val = P_Expr::exprVar(ii.val.getInt());
        break;
      }
    P_Expr::exprVar val2 = P_Expr::exprVar(val.getInt() + 1);
    sendCacheResult(ivar, val2, 0);
    sendCacheResult(ivar, val, 0);
    }
}
//------------------------------------------------------------------
void svManVars::sendCacheResult(const infoVar& ivar, P_Expr::exprVar val, int offset)
{
  __int64 v = val.iValue;
  if(SV_prfData::tFRData == ivar.type) {
    float t = (float)val.getReal();
    v = (*(__int64*)&t) & 0xffffffff;
    }
  else if(SV_prfData::tRData == ivar.type) {
    double t = val.getReal();
    v = *(__int64*)&t;
    }
  prph_Data data(ivar.id, offset);
  if(cacheVars.Search(&data)) {
    prph_Data* t = cacheVars.getCurrData();
    t->setValue(ivar, v);
    return;
    }
  prph_Data* t = new (freeSet.get()) prph_Data();

  *t = data;
  t->init(ivar);
  t->setValue(ivar, v);
  cacheVars.Add(t);
}
//------------------------------------------------------------------
void svManVars::clearCache()
{
  while(cacheVars.setFirst())
    freeSet.release((prph_Data*)cacheVars.Rem());
}
//------------------------------------------------------------------
bool svManVars::getInfoVar(infoVar& iVar)
{
  uint pos;
  if(Vars.find(iVar, pos, tCmp<infoVar>)) {
    iVar = Vars[pos];
    return true;
    }
  return false;
}
//------------------------------------------------------------------
void svManVars::sendFinal()
{
  prfDataMultiSet multiset;
  if(!cacheVars.setFirst())
    return;
  do {
    prph_Data* t = cacheVars.getCurrData();
    if(t->needSend()) {
      infoVar ivar = { t->getId() };
      if(getInfoVar(ivar) && ivar.prph) {
        uint prph = ivar.prph - 1;
        prfDataSet& dataset = multiset[prph];
        uint nElem = dataset.getElem();
        dataset[nElem].typeVar = ivar.type;
        dataset[nElem].lAddr = t->getOffset() + ivar.addr;
        dataset[nElem].U.li.QuadPart = t->getValueFinal(ivar);
        }
      }
    } while(cacheVars.setNext());
  clearCache();
  uint nElem = multiset.getElem();
  for(uint i = 0; i < nElem; ++i)
    sendData(i + 1, multiset[i]);
}
//------------------------------------------------------------------
void svManVars::sendData(uint prph, prfDataSet& set)
{
  uint nElem = set.getElem();
  if(!nElem)
    return;
  SV_prfData* data = (SV_prfData*) set.getVect();
  npSV_MultiSet(prph, data, nElem);
}
//------------------------------------------------------------------
void svManVars::sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR /*realName*/)
{
  int offset = getTrueOffset(ie.offset);
//  if(!isOffsVar(ie.offset))
//    offset += cycle * ie.step;
  P_Expr::exprVar old;
  infoVar ivar;
  if(!getVariableAndData(ie.idResult, offset, old, ivar, cycle * ie.step))
    return;
  if(val == old)
    return;

  sendCacheResult(ivar, val, offset);
}
//------------------------------------------------------------------
P_Expr::exprVar svManVars::getTypedVal(const prph_Data* t, const infoVar& ivar)
{
  __int64 v = t->getValue(ivar);
  switch(ivar.type) {
    case SV_prfData::tFRData:
      return P_Expr::exprVar(*(float*)&v);
    case SV_prfData::tRData:
      return P_Expr::exprVar(*(double*)&v);
    }
  return P_Expr::exprVar(v);
}
//------------------------------------------------------------------
P_Expr::exprVar svManVars::loadCacheVal(const infoVar& ivar, int offset)
{
  prph_Data data(ivar.id, offset);
  if(cacheVars.Search(&data)) {
    prph_Data* t = cacheVars.getCurrData();
    return getTypedVal(t, ivar);
    }
  prph_Data* t = new (freeSet.get()) prph_Data;
  *t = data;
  t->init(ivar);
  cacheVars.Add(t);

  return getTypedVal(t, ivar);
}
//------------------------------------------------------------------
int svManVars::getTrueOffset(int offset)
{
  if(isOffsVar(offset)) {
    bool exist;
    P_Expr::exprVar val = getVariable(unmakeOffsVar(offset), 0, 0, exist);
    offset = (int)val.getInt();
    }
  return offset;
}
//------------------------------------------------------------------
P_Expr::exprVar svManVars::loadVal(const infoVar& ivar, int& offset, int offsByStep)
{
  offset = getTrueOffset(offset) + offsByStep;
  int trueAddr = ivar.addr + offset;
  if(trueAddr < 0 || trueAddr >= MAX_VARS)
    offset = 0;

  return loadCacheVal(ivar, offset);
}
//------------------------------------------------------------------
bool svManVars::getVariableAndData(int id, int& offset, P_Expr::exprVar& var, infoVar& idata, int offsByStep)
{
  infoVar ivar = { id };
  if(getInfoVar(ivar)) {
    idata = ivar;
    var = loadVal(ivar, offset, offsByStep);
    return true;
    }
  return false;
}
//------------------------------------------------------------------
P_Expr::exprVar svManVars::getVariable(int id, int offset, int offsByStep, bool& exist)
{
  exist = true;
  infoVar ivar = { id };
  if(getInfoVar(ivar))
    return loadVal(ivar, offset, offsByStep);
  exist = false;
  P_Expr::exprVar val;
  return val;
}
//---------------------------------------------------------------------
