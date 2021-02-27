//-------- sv_make_dll_readRemote.cpp ------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include "p_file.h"
#include "PCrt_lck.h"
#include <stdio.h>
#include <stdlib.h>
#include "sv_manage_express.h"
//------------------------------------------------------------------
static PVect<svManExpression> Man_Expr;
//------------------------------------------------------------------
static int Prf;
static DWORD Mask;
static SV_prfData Data;
//------------------------------------------------------------------
static void makeAddrReload(LPCTSTR p)
{
  if(!p)
    return;
  bool ok = false;
  do {
    Prf = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    Data.lAddr = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    Data.typeVar = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    int nBit = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    int offs = _ttoi(p);
    if(SV_prfData::tBitData == Data.typeVar) {
      Data.U.dw = MAKELONG(nBit, offs);
      Mask = (DWORD)-1;
      }
    else {
      Mask = nBit << offs;
      if(!Mask)
        Mask = (DWORD)-1;
      }
    ok = true;
    } while(false);
  if(!ok)
    Prf = 0;
}
//------------------------------------------------------------------
static bool needReload()
{
  if(!Prf)
    return false;
  SV_prfData t = Data;
  if(SV_prfData::isChanged != npSV_Get(Prf, &t))
    return false;
  if(t.U.dw & Mask) {
    t.U.dw &= ~Mask;
    npSV_Set(Prf, &t);
    return true;
    }
  return false;
}
//------------------------------------------------------------------
#define ID_SCRIPT_NAME    10
#define ID_SCRIPT_RELOAD  11
#define ID_PRECISION_CALC 12

#define ID_ERROR_EXPR_TIT 20
#define ID_ERROR_EXPR     21

#define MAX_ADD_EXPRESS    10
#define ID_SCRIPT_NAME_ADD 51
//------------------------------------------------------------------
double dPRECISION;
//------------------------------------------------------------------
static void errMsg(int ix, LPCTSTR altern = 0)
{
  LPCTSTR msg = npSV_GetLocalString(ID_ERROR_EXPR, getHinstance());
  LPCTSTR tit = npSV_GetLocalString(ID_ERROR_EXPR_TIT, getHinstance());
  if(!msg)
    msg = _T("There are some errors on the source script or on some variables or the script file is not valid");
  if(!tit)
    tit = _T("Script Error - Express dll");
  TCHAR titleName[_MAX_PATH];
  if(-1 == ix)
    wsprintf(titleName, _T("%s - All Script"), tit);
  else if(altern)
    wsprintf(titleName, _T("%s - [%s]"), tit, altern);
  else {
    uint id = 0;
    if(0 == ix)
      id = ID_SCRIPT_NAME;
    else
      id = ID_SCRIPT_NAME_ADD + ix - 1;
    for(; ix < MAX_ADD_EXPRESS; ++ix, ++id) {
      LPCTSTR p = npSV_GetLocalString(id, getHinstance());
      if(p) {
        wsprintf(titleName, _T("%s - [%s]"), tit, p);
        break;
        }
      }
    }
  MessageBox(0, msg, titleName, MB_OK | MB_ICONSTOP);
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  static int checked = 0;
  static bool inExec;
  if(inExec)
    return;
  inExec = true;
  if(needReload()) {
    npSV_GetBodyRoutine(eSV_RELOAD_TEXT_SET, LPDWORD(getHinstance()), 0);
    checked = 0;
    }

  if(!checked) {
    checked = -1;
    Man_Expr.reset();
    resetCountOnMem();
    LPCTSTR p = npSV_GetLocalString(ID_SCRIPT_NAME, getHinstance());
    if(p) {
      TCHAR path[_MAX_PATH];
      GetCurrentDirectory(SIZE_A(path), path);
      appendPath(path, p);
      checked = Man_Expr[0].makeExpression(path) ? 1 : -1;
      }
    p = npSV_GetLocalString(ID_SCRIPT_RELOAD, getHinstance());
    makeAddrReload(p);
    dPRECISION = 0.00001;
    p = npSV_GetLocalString(ID_PRECISION_CALC, getHinstance());
    if(p)
      dPRECISION = _tstof(p);
    if(-1 == checked)
      errMsg(0);
    else {
      for(uint i = 0, j = 1; i < MAX_ADD_EXPRESS; ++i) {
        p = npSV_GetLocalString(ID_SCRIPT_NAME_ADD + i, getHinstance());
        if(p) {
          TCHAR path[_MAX_PATH];
          GetCurrentDirectory(SIZE_A(path), path);
          appendPath(path, p);
          if(!Man_Expr[j].makeExpression(path))
            errMsg(0, p);
          ++j;
          }
        }
      }
    }
  if(-1 != checked) {
    uint nExpr = Man_Expr.getElem();
    PVect<bool> success;
    bool failed = false;
    for(uint i = 0; i < nExpr; ++i) {
      success[i] = Man_Expr[i].performExpression();
      if(!success[i])
        failed = true;
      }
    if(failed) {
      for(uint i = 0; i < nExpr; ++i) {
        if(success[i]) {
          failed = false;
          break;
          }
        }
      // non tutti sono in errore, allora visualizziamo solo quelli in errore
      if(!failed) {
        for(uint i = 0; i < nExpr; ++i) {
          if(!success[i])
            errMsg(i);
          }
        }
      else {
        checked = -1;
        // se c'è solo lo script principale
        if(1 == nExpr)
          errMsg(0);
        else
          // tutti gli script hanno errori
          errMsg(-1);
        }
      }
    }
  inExec = false;
}
//------------------------------------------------------------------
