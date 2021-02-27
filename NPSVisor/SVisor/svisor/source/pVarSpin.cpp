//------ PVarSpin.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "p_basebody.h"
#include "p_util.h"
#include "PVarSpin.h"
#include "newNormal.h"
//----------------------------------------------------------------------------
float operator +(prfData& data, float v) { return data.U.fw + v; }
const prfData& operator +=(prfData& data, float v) { data.U.fw += v;  return data; }
//----------------------------------------------------------------------------
int operator +(prfData& data, int v) { return data.U.sdw + v; }
const prfData& operator +=(prfData& data, int v) { data.U.sdw += v;  return data; }
//----------------------------------------------------------------------------
PVarSpin::PVarSpin(P_BaseBody* owner, uint id) :  baseClass(owner, id), Stat(idleSpin),
      memVar(1, 0, prfData::tDWsData, 0), idSpin(0), timeBeforeSend(500),
      firstStart(true), integerStep(true)
{
  StepUp.iv = 0;
  StepDn.iv = 0;

}
//----------------------------------------------------------------------------
static bool isReal(LPCTSTR p)
{
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i)
    if(_T('.') == p[i])
      return true;
    else if(_T(',') == p[i])
      return false;
  return false;
}
//----------------------------------------------------------------------------
bool PVarSpin::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int addrMem = 0;

  int nscan = _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &idSpin, &addrMem, &timeBeforeSend);
  if(!idSpin || !addrMem)
    return false;

  p = findNextParamTrim(p, nscan);
  LPCTSTR p2 = findNextParamTrim(p);
  if(!p2)
    p2 = p;
  if(p && p2) {
    bool useReal = isReal(p) || isReal(p2);
    if(useReal) {
      StepUp.fv = (float)_tstof(p);
      StepDn.fv = (float)_tstof(p2);
      if(!StepDn.fv)
        StepDn.fv = StepUp.fv;
      integerStep = false;
      memVar.setType(prfData::tFRData);
      }
    else {
      StepUp.iv = _ttoi(p);
      StepDn.iv = _ttoi(p2);
      if(!StepDn.iv)
        StepDn.iv = StepUp.iv;
      }
    }
  memVar.setAddr(addrMem);

  return makeStdVars();
}
//----------------------------------------------------------------------------
void PVarSpin::action(int up)
{
  prfData data;
  prfData::tResultData result = memVar.getData(getOwner(), data, getOffs());
  if(prfData::failed >= result)
    return;

  if(integerStep)
    data += up ? StepUp.iv : -StepDn.iv;
  else
    data += up ? StepUp.fv : -StepDn.fv;

  genericPerif* prph = getOwner()->getGenPerif(memVar.getPrph());
  if(!prph)
    return;

  prph->set(data);
  lastTime = GetTickCount();
  Stat = waitTimeout;
}
//----------------------------------------------------------------------------
bool PVarSpin::update(bool force)
{
  if(firstStart) { // azzera la variabile in memoria
    firstStart = false;
    prfData dataMem;
    prfData::tResultData result = memVar.getData(getOwner(), dataMem, getOffs());
    if(prfData::failed >= result)
      return false;
    dataMem.U.sdw = 0;
    genericPerif* prph = getOwner()->getGenPerif(memVar.getPrph());
    if(!prph)
      return false;
    prph->set(dataMem);
    }

  if(idleSpin == Stat)
    return false;

  if(waitTimeout == Stat) {
    DWORD currTime = GetTickCount();
    if(currTime - lastTime >= timeBeforeSend) {
      Stat = idleSpin;
      prfData dataMem;
      prfData::tResultData result = memVar.getData(getOwner(), dataMem, getOffs());
      if(prfData::failed >= result)
        return false;
      prfData data;
      result = BaseVar.getData(getOwner(), data, getOffs());
      if(prfData::failed >= result)
        return false;

      double var = BaseVar.getNormalizedResult(data);
      if(integerStep)
        var += dataMem.U.sdw;
      else
        var += dataMem.U.fw;
      BaseVar.getDeNormalized(data, var);

      if(!verifyRange(BaseVar.getNorm(), data)) {
        fREALDATA vMin;
        fREALDATA vMax;
        getMinMax(vMin, vMax, BaseVar.getNorm());
        if(var < vMin)
          var = vMin;
        else
          var = vMax;
        BaseVar.getDeNormalized(data, var);
        }
      genericPerif* prph = getOwner()->getGenPerif(BaseVar.getPrph());
      if(!prph)
        return false;

      gestPerif::howSend old = prph->setAutoShow(getOwner()->showWhileSend() ? gestPerif::ALL_AND_SHOW : gestPerif::CHECK_ALL_ASYNC);
      prph->set(data, true);
      prph->setAutoShow(old);

      dataMem.U.sdw = 0;
      genericPerif* prphMem = getOwner()->getGenPerif(memVar.getPrph());
      if(!prphMem)
        return false;
      prphMem->set(dataMem, true);
      }
    }
  return false;
}
//----------------------------------------------------------------------------
