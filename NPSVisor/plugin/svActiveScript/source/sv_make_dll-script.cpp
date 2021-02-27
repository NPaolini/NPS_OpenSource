//------ sv_make_dll-script.cpp -----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#define INITGUID
#include <activscp.h>
#include "p_util.h"
#include "sv_make_dll.h"
//-----------------------------------------------------------
BOOL initProcess(HINSTANCE hI)
{
  return TRUE;
}
//-----------------------------------------------------------
#define ID_TEXT_RELOAD 10
#define ID_SCRIPT_STOP 11
//-----------------------------------------------------------
struct infoData
{
  int Prph;
  DWORD Mask;
  uint offs;
  SV_prfData Data;
  infoData() : Prph(0), Mask(0), offs(0) { ZeroMemory(&Data, sizeof(Data)); }
};
//-----------------------------------------------------------
static LPCTSTR makeInfoData(LPCTSTR p, infoData& target)
{
  if(!p)
    return 0;
  bool ok = false;
  do {
    target.Prph = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    target.Data.lAddr = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    target.Data.typeVar = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    int nBit = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    target.offs = _ttoi(p);
    p = findNextParam(p, 1);
    if(SV_prfData::tBitData == target.Data.typeVar)
      target.Data.typeVar = SV_prfData::tDWData;
    target.Mask = nBit << target.offs;
    if(!target.Mask)
      target.Mask = (DWORD)-1;
    ok = true;
    } while(false);
  if(!ok)
    target.Prph = 0;
  return p;
}
//-----------------------------------------------------------
static LPCTSTR makeInfoData(uint id, infoData& target)
{
  LPCTSTR p = npSV_GetLocalString(id, getHinstance());
  return makeInfoData(p, target);
}
//-----------------------------------------------------------
static infoData iData;
//-----------------------------------------------------------
static infoData iDataStop;
//-----------------------------------------------------------
static void makeAddrReload()
{
  LPCTSTR p = npSV_GetLocalString(ID_TEXT_RELOAD, getHinstance());
  makeInfoData(p, iData);
  p = npSV_GetLocalString(ID_SCRIPT_STOP, getHinstance());
  makeInfoData(p, iDataStop);
}
//-----------------------------------------------------------
static bool isChanged(infoData& idata, bool reset = true)
{
  if(!idata.Prph)
    return false;
  SV_prfData t = idata.Data;
  npSV_Get(idata.Prph, &t);
  if(t.U.dw & idata.Mask) {
    if(reset) {
      t.U.dw &= ~idata.Mask;
      npSV_Set(idata.Prph, &t);
      }
    return true;
    }
  return false;
}
//-----------------------------------------------------------
static bool needReload()
{
  return isChanged(iData);
}
//-----------------------------------------------------------
static bool needStop()
{
  return isChanged(iDataStop, false);
}
//-----------------------------------------------------------
extern bool runScript();
extern bool initScript();
extern void endScript();
//-----------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  if(eEND_DLL == msg) {
    endScript();
    CoUninitialize();
    }
  else if(eINIT_DLL == msg)
    CoInitialize(0);
}
//-----------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  static bool inExec;
  if(inExec)
    return;
  inExec = true;

  static int checked = 0;
  if(needReload()) {
    endScript();
    npSV_GetBodyRoutine(eSV_RELOAD_TEXT_SET, LPDWORD(getHinstance()), 0);
    checked = 0;
    }

  if(!checked) {
    checked = -1;
    makeAddrReload();
    if(initScript())
      checked = 1;
    }
  if(!needStop()) {
    if(checked > 0)
      if(!runScript())
        checked = -1;
    }
  inExec = false;
}
//------------------------------------------------------------------
