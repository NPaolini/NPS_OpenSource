//-------- sv_make_dll.h -------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include <stdio.h>
#include <stdlib.h>
#include "PDSetForeground.h"
//------------------------------------------------------------------
#include "p_vect.h"
//------------------------------------------------------------------
#define ID_RELOAD_SET  10
#define ID_RUN_SET 11
//------------------------------------------------------------------
struct infoData
{
  int Prph;
  DWORD Mask;
  uint offs;
  SV_prfData Data;
  infoData() : Prph(0), Mask(0), offs(0) { ZeroMemory(&Data, sizeof(Data)); }
};
//------------------------------------------------------------------
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
//------------------------------------------------------------------
static LPCTSTR makeInfoData(uint id, infoData& target)
{
  LPCTSTR p = npSV_GetLocalString(id, getHinstance());
  return makeInfoData(p, target);
}
//------------------------------------------------------------------
static infoData iData;
//------------------------------------------------------------------
static infoData iDataRun;
//------------------------------------------------------------------
static int makeAddrReload()
{
  LPCTSTR p = npSV_GetLocalString(ID_RELOAD_SET, getHinstance());
  makeInfoData(p, iData);
  p = npSV_GetLocalString(ID_RUN_SET, getHinstance());
  makeInfoData(p, iDataRun);
  return iDataRun.Prph > 0 ? 1 : -1;
}
//------------------------------------------------------------------
static bool isChanged(infoData& idata)
{
  if(!idata.Prph)
    return false;
  SV_prfData t = idata.Data;
  npSV_Get(idata.Prph, &t);
  if(t.U.dw & idata.Mask) {
    t.U.dw &= ~idata.Mask;
    npSV_Set(idata.Prph, &t);
    return true;
    }
  return false;
}
//------------------------------------------------------------------
static bool needReload()
{
  return isChanged(iData);
}
//-----------------------------------------------------------
PWin* getMainWin()
{
  HWND hw = (HWND)npSV_GetBodyRoutine(eSV_GET_HWND_MAIN, 0, 0);
  PWin* owner = 0;
  if(hw)
    owner = PWin::getWindowPtr(hw);
  return owner;
}
//------------------------------------------------------------------
static void runCode()
{
  if(isChanged(iDataRun)) {
    PDSetForeground fg(getMainWin(), IDD_CLIENT, getHinstance());
    fg.modal();
    }
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
    npSV_GetBodyRoutine(eSV_RELOAD_TEXT_SET, LPDWORD(getHinstance()), 0);
    checked = 0;
    }

  if(!checked)
    checked = makeAddrReload();

  if(1 == checked)
    runCode();

  inExec = false;
}
//------------------------------------------------------------------
