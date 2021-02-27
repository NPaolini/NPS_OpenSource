//-------- testManVars.cpp -----------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "testManVars.h"
#include "resource.h"
#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>
//------------------------------------------------------------------
extern PWin* getMain();
//------------------------------------------------------------------
extern HWND getEditCtrlResult(PWin* owner);
extern HWND getEditCtrlInit(PWin* owner);
extern HWND getEditCtrlTest(PWin* owner);
extern HWND getEditCtrlCalc(PWin* owner);
extern HWND getEditCtrlFunct(PWin* owner);
extern bool findFunct(PWin* owner, LPCTSTR functName, CHARRANGE& cr);
//------------------------------------------------------------------
LRESULT testManVars::cbError(const ParserError* pe)
{
  TCHAR t[2048];
  switch(pe->code) {
    case ERR_EXPR_VAR:
      wsprintf(t, _T("Row [%d], Error expression [%s] at position [%d]"), pe->row, pe->varName, pe->col);
      selectErr(pe->col, pe->row, toBool(pe->functName));
      break;
    case ERR_EXPR_PARAM:
      wsprintf(t, _T("Row [%d], Error Parameter"), pe->row);
      selectErr(pe->row, toBool(pe->functName));
      break;
    case ERR_EXPR_VAR_NOT_FOUND:
      wsprintf(t, _T("Row [%d], Variable [%s] not found"), pe->row, pe->varName);
      selectErr(pe->varName, pe->row, toBool(pe->functName));
      break;
    case ERR_EXPR_DUP_LABEL:
      wsprintf(t, _T("Label [%s] duplicated"), pe->varName);
      break;
    default:
      return 0;
    }
  if(pe->functName)
    wsprintf(t + _tcslen(t), _T(" - Function %s"), pe->functName);

  MessageBox(*Owner, t, _T("Error"), MB_OK | MB_ICONSTOP);

  return 0;
}
//------------------------------------------------------------------
void testManVars::selectErr(int col, int row, bool functName)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);

  CHARRANGE cr;
  cr.cpMin = ix + col;
  cr.cpMax = cr.cpMin + 1;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
void testManVars::selectErr(LPCTSTR text, int row, bool functName)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);

  FINDTEXTEXW fte;
  ZeroMemory(&fte, sizeof(fte));
  fte.chrg.cpMin = ix;
  fte.chrg.cpMax = -1;
  fte.lpstrText = text;
  DWORD wParam = FR_DOWN;

  int result = SendMessage(hed, EM_FINDTEXTEXW, wParam, (LPARAM)&fte);
  if(result < 0)
    return;
  CHARRANGE cr = fte.chrgText;
  --cr.cpMin;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
void testManVars::selectErr(int row, bool functName)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);
  int ix2 = SendMessage(hed, EM_LINEINDEX, row + 1, 0);

  CHARRANGE cr;
  cr.cpMin = ix;
  cr.cpMax = ix2;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
extern int getDecimal();
//------------------------------------------------------------------
void testManVars::makeRowOut(LPTSTR buff, size_t dim, const ParserInfo& pi)
{
  uint ix = pi.offset;
  int ndec = getDecimal();
  if(ix) {
    if(itv_Int == pi.type)
      _stprintf_s(buff, dim, _T("$%s[%d]=%I64i\n"), pi.varName, ix, pi.i);
    else {
      if(ndec)
        _stprintf_s(buff, dim, _T("$%s[%d]=%0.*lf\n"), pi.varName, ix, ndec, pi.d);
      else
        _stprintf_s(buff, dim, _T("$%s[%d]=%lf\n"), pi.varName, ix, pi.d);
      zeroTrim(buff);
      }
    }
  else {
    if(itv_Int == pi.type)
      _stprintf_s(buff, dim, _T("$%s=%I64i\n"), pi.varName, pi.i);
    else {
      if(ndec)
        _stprintf_s(buff, dim, _T("$%s=%0.*lf\n"), pi.varName, ndec, pi.d);
      else
        _stprintf_s(buff, dim, _T("$%s=%lf\n"), pi.varName, pi.d);
      zeroTrim(buff);
      }
    }
}
//------------------------------------------------------------------
void testManVars::sendOut(LPCTSTR varValue)
{
  HWND hed = getEditCtrlResult(Owner);
  if(!hed)
    return;
  SendMessage(hed, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessage(hed, EM_REPLACESEL, true, (LPARAM)varValue);
}
//------------------------------------------------------------------
/*
manExpressionVars* allocManVars()
{
  return new testManVars(getMain());
}
//------------------------------------------------------------------
#define ID_SHOW_ERROR 100
#define ID_INIT_SHOW_ERROR 101
#define ID_VAR_NOT_FOUND ID_INIT_SHOW_ERROR
#define ID_EXPR_ERROR 102
//------------------------------------------------------------------
void testManVars::shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR funct_name)
{
  resolveRowCol(num2, row);
  TCHAR buff[500];
  if(num2 >= 0) {
    functName = funct_name;
    selectErr(num2, row);
    if(!var)
      var = _T("???");
    if(funct_name)
      wsprintf(buff, _T("Riga [%d], Errore espressione [%s] alla posizione [%d] - Funzione %s"), row + 1, var, num2 + 1, funct_name);
    else
      wsprintf(buff, _T("Riga [%d], Errore espressione [%s] alla posizione [%d]"), row + 1, var, num2 + 1);
    }
  else {
    switch(num2) {
      case -1:
        if(!id && !var) {
          if(funct_name) {
            wsprintf(buff, _T("Riga [%d], Errore Parametri - Funzione %s"), row + 1, funct_name);
            selectErr(row);
            }
          else {
            selectErr(row);
            wsprintf(buff, _T("Riga [%d], Errore Parametri"), row + 1);
            }
          }
        else {
          if(!var)
            var = _T("???");
          selectErr(var, row);
          if(funct_name)
            wsprintf(buff, _T("Riga [%d], Variabile [%s] non trovata - Funzione %s"), row + 1, var, funct_name);
          else
            wsprintf(buff, _T("Riga [%d], Variabile [%s] non trovata"), row + 1, var);
          }
        break;
      case -2:
        if(funct_name)
          wsprintf(buff, _T("Etichetta [%s] duplicata - Funzione %s"), var, funct_name);
        else
          wsprintf(buff, _T("Etichetta [%s] duplicata"), var);
        break;
      case -3:
        wsprintf(buff, _T("Init -> Variabile [%s] non trovata"), var);
        break;
      }
    }
  MessageBox(*Owner, buff, _T("Error"), MB_OK | MB_ICONSTOP);
}
//------------------------------------------------------------------
void testManVars::selectErr(int col, int row)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);

  CHARRANGE cr;
  cr.cpMin = ix + col;
  cr.cpMax = cr.cpMin + 1;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
void testManVars::selectErr(LPCTSTR text, int row)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);

  FINDTEXTEXW fte;
  ZeroMemory(&fte, sizeof(fte));
  fte.chrg.cpMin = ix;
  fte.chrg.cpMax = -1;
  fte.lpstrText = text;
  DWORD wParam = FR_DOWN;

  int result = SendMessage(hed, EM_FINDTEXTEXW, wParam, (LPARAM)&fte);
  if(result < 0)
    return;
  CHARRANGE cr = fte.chrgText;
  --cr.cpMin;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
void testManVars::selectErr(int row)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  int ix = SendMessage(hed, EM_LINEINDEX, row, 0);
  int ix2 = SendMessage(hed, EM_LINEINDEX, row + 1, 0);

  CHARRANGE cr;
  cr.cpMin = ix;
  cr.cpMax = ix2;
  SendMessage(hed, EM_EXSETSEL, 0, (LPARAM)&cr);
}
//------------------------------------------------------------------
void testManVars::resolveRowCol(int& col, int& row)
{
  if(row >= 0) {
    if(functName) {
      row = getTrueRowFunct(row);
      // non può esserci un errore nella riga della funzione, allora inutile continuare
      if(!row)
        return;
      }
    else {
      int t = 0;
      while(t <= row) {
        if(isFalseRow(t))
          ++row;
        ++t;
        }
      }
    }
  if(col >= 0)
    col = getTrueCol(col, row);
}
//------------------------------------------------------------------
extern bool isReservedVar(LPCTSTR realName);
//------------------------------------------------------------------
static bool isSkip(LPTSTR buff)
{
  TCHAR t[8] = {0};
  for(uint i = 0; i < SIZE_A(t); ++i) {
    if(_T(' ') < buff[i]) {
      buff[i] = 0;
      break;
      }
    t[i] = buff[i];
    }
  return isReservedVar(t);
}
//------------------------------------------------------------------
int testManVars::getTrueRowFunct(int row)
{
  TCHAR funct[MAX_LEN_NAME_VARS_FUNCT] = { VAR_FUNCT_CHAR, 0 };
  _tcscat_s(funct, functName);
  CHARRANGE cr;
  if(!findFunct(Owner, funct, cr))
    return 0;

  HWND hed = getEditCtrlFunct(Owner);
  int ixLine =  SendMessage(hed, EM_LINEFROMCHAR, cr.cpMin, 0) + 1;
  TCHAR buff[4096];
  int initFunct = -1;
  int totLines = SendMessage(hed, EM_GETLINECOUNT, 0, 0);
  for(int i = ixLine, t = 0; i < totLines && t <= row; ++i, ++t) {
    *(LPWORD)buff = SIZE_A(buff);
    int nchar = SendMessage(hed, EM_GETLINE, i, (LPARAM)buff);
    while(nchar > 0) {
      if(buff[nchar - 1] > _T(' ')) {
        buff[nchar] = 0;
        break;
        }
      --nchar;
      }
    if(!nchar) {
      ++row;
      continue;
      }
    for(int i = nchar - 1; i > 0; --i) {
      if(_T('\\') == buff[i] || _T('{') == buff[i] || _T('}') == buff[i]) {
        ++row;
        nchar = 0;
        break;
        }
      if(buff[i] > _T(' '))
        break;
      }

    for(int i = 0; i < nchar; ++i) {
      if(VAR_FUNCT_CHAR == buff[i]) {
        if(isSkip(buff + i + 1)) {
          ++row;
          break;
          }
        }
      else if(_T(';') == buff[i] || _T('#') == buff[i] || _T('{') == buff[i]) {
        ++row;
        break;
        }
      if(buff[i] > _T(' '))
        break;
      }
    }
  return row + ixLine;
}
//------------------------------------------------------------------
int testManVars::isFalseRow(int row)
{
  HWND hed = getEditCtrlCalc(Owner);
  TCHAR buff[4096];
  *(LPWORD)buff = SIZE_A(buff);
  int nchar = SendMessage(hed, EM_GETLINE, row, (LPARAM)buff);
  while(nchar > 0) {
    if(buff[nchar - 1] > _T(' ')) {
      buff[nchar] = 0;
      break;
      }
    --nchar;
    }
  if(!nchar)
    return 1;
  for(int i = nchar - 1; i > 0; --i) {
    if(_T('\\') == buff[i])
      return 1;
    if(buff[i] > _T(' '))
      break;
    }
  for(int i = 0; i < nchar; ++i) {
    switch(buff[i]) {
      case VAR_CHAR:
        if(!isSkip(buff + i + 1))
          break;
      case _T(';'):
      case _T('#'):
        return 1;
      }
    if(buff[i] > _T(' '))
      break;
    }
  return 0;
}
//------------------------------------------------------------------
int testManVars::parseVar(LPTSTR buff, int pos, int& dimVar)
{
  TCHAR t[MAX_LEN_NAME_VARS_FUNCT] = {0};
  ++buff;
  int dim = 0;
  for(uint i = 0, j = pos; i < SIZE_A(t); ++i, ++j) {
    if(_istalnum(buff[j]) || _T('_') == buff[j])
      t[i] = buff[j];
    else {
      t[i] = 0;
      dim = i;
      break;
      }
    }
  if(functName) {
    dimVar = 1;
    return dim - 1;
    }
  uint id = ReplaceVar->getId(t);
  if(!id) {
    dimVar = 1;
    return dim - 1;
    }
  wsprintf(t, _T("%d"), id);
  dimVar = _tcslen(t);
  dim -= dimVar;
  return dim;
}
//------------------------------------------------------------------
int testManVars::getTrueCol(int col, int row)
{
  HWND hed = functName ?  getEditCtrlFunct(Owner) : getEditCtrlCalc(Owner);
  TCHAR buff[4096];
  *(LPWORD)buff = SIZE_A(buff);
  int nchar = SendMessage(hed, EM_GETLINE, row, (LPARAM)buff);
  if(!nchar)
    return 0;

  int i;
  for(i = 0; i < nchar; ++i) {
    if(_T('=') == buff[i]) {
      ++i;
      break;
      }
    }
  int ret = i;
  int t = col;
  for(int j = 0; i < nchar && j <= col; ++i) {
    if(_T(' ') == buff[i])
      ++ret;
    else if(VAR_CHAR == buff[i] || VAR_FUNCT_CHAR == buff[i]) {
      int addJ = 0;
      int dim = parseVar(buff, i, addJ);
      ret += dim;
      i += dim;
      j += addJ + 1;
      }
    else
      ++j;
    }
  return col + ret;
}
//------------------------------------------------------------------
void testManVars::manageInit(LPTSTR p)
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

  P_Expr::exprVar val;

  int len = _tcslen(p);
  bool isReal = false;
  for(int i = 0; i < len; ++i) {
    if(_T('.') == p[i]) {
      isReal = true;
      break;
      }
    }
  if(isReal) {
    val.dValue = _tstof(p);
    val.type = P_Expr::tvReal;
    }
  else {
    val.iValue = _ttoi64(p);
    }
  replaceVal(val, id, 0);
  Inits[Inits.getElem()] = id;
}
//------------------------------------------------------------------
void testManVars::manageVar(LPTSTR p)
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
          &ivar.prf, &ivar.addr, &ivar.type,
          &ivar.nbit, &ivar.offs, &ivar.norm);
  ivar.id = id;
  if(!ivar.prf && !ivar.type) {
    ivar.type = ivar.addr ? 7 : 6; //SV_prfData::tRData : SV_prfData::ti64Data;
    ivar.addr = getNextCountOnMem();
    }
  Vars[Vars.getElem()] = ivar;
}
//------------------------------------------------------------------
int testManVars::getTypeResult(int id)
{
  int nElem = Vars.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(Vars[i].id == id) {
      switch(Vars[i].type) {
        case 5: //SV_prfData::tFRData:
        case 7: //SV_prfData::tRData:
          return P_Expr::tvReal;
        default:
          return P_Expr::tvInt;
        }
      }
    }
  return -1;
}
//------------------------------------------------------------------
extern int getDecimal();
//------------------------------------------------------------------
void testManVars::sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName)
{
  HWND hed = getEditCtrlResult(Owner);
  if(!hed)
    return;
  bool exist;
  int offset = getTrueOffset(ie.offset, exist);
//  if(!isOffsVar(ie.offset))
    offset += cycle * ie.step;

  TCHAR buff[100];
  TCHAR tId[20];
  if(!realName) {
    wsprintf(tId, _T("%d"), ie.idResult);
    realName = tId;
    }

  if(P_Expr::tvInt == val.type) {
    if(offset)
      _stprintf_s(buff, SIZE_A(buff), _T("$%s[%d] = %I64d\r\n"), realName, offset, val.getInt());
    else
      _stprintf_s(buff, SIZE_A(buff), _T("$%s = %I64d\r\n"), realName, val.getInt());
    }
  else {
    TCHAR t[50];
    int ndec = getDecimal();
    _stprintf_s(t, SIZE_A(t), _T("%0.*f"), ndec, val.getReal());
    zeroTrim(t);
    if(offset)
      _stprintf_s(buff, SIZE_A(buff), _T("$%s[%d] = %s\r\n"), realName, offset, t);
    else
      _stprintf_s(buff, SIZE_A(buff), _T("$%s = %s\r\n"), realName, t);
    }

  SendMessage(hed, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessage(hed, EM_REPLACESEL, true, (LPARAM)buff);
  replaceVal(val, ie.idResult, offset);
}
//------------------------------------------------------------------
void testManVars::setVal(const P_Expr::exprVar& var, const infoVar& ivar, int offset)
{
  offset += ivar.addr;
  if((uint)offset >= DIM_DAT)
    return;
  infoDat& dat = Dat[ivar.prf];
  switch(ivar.type) {
    case SV_prfData::tFRData:
      {
      float v = (float)var.getReal();
      dat[offset] = *(LPDWORD)&v;
      }
      break;
    case SV_prfData::tRData:
      {
      double v = var.getReal();
      double* pd = (double*)&(dat[offset]);
      *pd = v;
      }
      break;

    case SV_prfData::tBsData:
    case SV_prfData::tWsData:
    case SV_prfData::tDWsData:
    case SV_prfData::tBData:
    case SV_prfData::tWData:
    case SV_prfData::tDWData:
    case SV_prfData::tStrData:
    case SV_prfData::tBitData:
      if(ivar.nbit || ivar.offs) {
        DWORD t = (1 << ivar.nbit) - 1;
        t <<= ivar.offs;
        dat[offset] &= ~t;
        t = (DWORD)var.getInt();
        t &= (1 << ivar.nbit) - 1;
        t <<= ivar.offs;
        dat[offset] |= t;
        }
      else
        dat[offset] = (DWORD)var.getInt();
      break;

    case SV_prfData::ti64Data:
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      {
      __int64* pd = (__int64*)&(dat[offset]);
      *pd = var.getInt();
      }
      break;
    }
}
//------------------------------------------------------------------
void testManVars::replaceVal(const P_Expr::exprVar& var, int id, int offset)
{
  if(var.nullVal)
    return;

  if(!Dat.getElem()) {
    loadEditField(getEditCtrlInit(Owner));
//    loadEditField(IDC_EDIT_TEST);
    }

  uint nElem = Vars.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Vars[i].id == id) {
      setVal(var, Vars[i], offset);
      break;
      }
    }
}
//------------------------------------------------------------------
void testManVars::loadRowVals(LPCTSTR p)
{
  while(p && *p && *p != VAR_CHAR && *p != _T(';'))
    ++p;

  if(!p || !*p || VAR_CHAR != *p)
    return;
  ++p;

  int id = _ttoi(p);
  for(;*p; ++p) {
    if(_T('=') == *p) {
      ++p;
      break;
      }
    }
  if(!*p)
    return;

  int type = getTypeResult(id);
  if(type < 0)
    return;

  uint nElem = Vars.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Vars[i].id == id) {
      const infoVar& ivar = Vars[i];
      int ix = 0;
      if(ivar.type > 0) {
        do {
          P_Expr::exprVar expr(_ttoi64(p));
          setVal(expr, ivar, ix);
          p = findNextParam(p, 1);
          ++ix;
          } while(p);
        }
      else {
        do {
          P_Expr::exprVar expr(_tstof(p));
          setVal(expr, ivar, ix);
          p = findNextParam(p, 1);
          ++ix;
          } while(p);
        }
      break;
      }
    }
}
//------------------------------------------------------------------
void testManVars::loadEditField(HWND hed)
{
//  HWND hed = GetDlgItem(*Owner, idc);
  if(!hed)
    return;
  uint size = SendMessage(hed, WM_GETTEXTLENGTH, 0, 0);
  LPTSTR buff = new TCHAR[size + 2];
  SendMessage(hed, WM_GETTEXT, size + 1, (LPARAM)buff);
  buff[size] = 0;
  buff[size + 1] = 0;

  bool canDelete = true;
  if(ReplaceVar)
    buff = ReplaceVar->run(buff, size, canDelete, 2, false);

  LPTSTR p = buff;
  while(size && p) {
    LPTSTR p2 = getLine(p, size);
    int offs = p2 ? p2 - p : size;
    loadRowVals(p);
    size -= offs;
    p = p2;
    }
  delete []buff;
}
//------------------------------------------------------------------
P_Expr::exprVar testManVars::loadVal(const infoVar& ivar, int offset, int offsByStep)
{
  P_Expr::exprVar dummy;
  if(!Dat.getElem()) {
    loadEditField(getEditCtrlInit(Owner));
//    loadEditField(IDC_EDIT_TEST);
    }
  bool exist = true;
  offset = getTrueOffset(offset, exist);
  if(!exist) {
    LPCTSTR realName = 0;
    if(ReplaceVar)
      realName = ReplaceVar->getName(offset);
    shoMsg(ID_VAR_NOT_FOUND, realName, -1, getCurrRow());
    }
  offset += ivar.addr + offsByStep;
  if((uint)offset >= DIM_DAT)
    return dummy;

  P_Expr::exprVar var;
  infoDat& dat = Dat[ivar.prf];
  switch(ivar.type) {
    case SV_prfData::tFRData:
      var.type = P_Expr::tvReal;
      var.dValue = *(float*)&dat[offset];
      break;
    case SV_prfData::tRData:
      var.type = P_Expr::tvReal;
      var.dValue = *(double*)&dat[offset];
      break;

    case SV_prfData::tBsData:
    case SV_prfData::tWsData:
    case SV_prfData::tDWsData:
      var.iValue = (int)dat[offset];
      break;

    case SV_prfData::tBData:
    case SV_prfData::tWData:
    case SV_prfData::tDWData:
    case SV_prfData::tStrData:
    case SV_prfData::tBitData:
      var.iValue = (uint)dat[offset];
      if(ivar.nbit || ivar.offs) {
        var.iValue >>= ivar.offs;
        var.iValue &= (1 << ivar.nbit) - 1;
        }
      break;

    case SV_prfData::ti64Data:
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      var.iValue = *(__int64*)&dat[offset];
      break;
    default:
      return dummy;
    }
  return var;
}
//------------------------------------------------------------------
bool testManVars::findId(int id)
{
  uint nElem = Vars.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(Vars[i].id == id)
      return true;
  return false;
}
//------------------------------------------------------------------
void testManVars::performInit()
{
  uint nElem = Inits.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!findId(Inits[i])) {
      LPCTSTR realName = _T("??");
      if(ReplaceVar)
        realName = ReplaceVar->getName(Inits[i]);
      shoMsg(0, realName, -3, 0);
      }
    }
  loadEditField(getEditCtrlTest(Owner));
}
//------------------------------------------------------------------
int testManVars::getTrueOffset(int offset, bool& exist)
{
  if(isOffsVar(offset)) {
    P_Expr::exprVar val = getVariable(unmakeOffsVar(offset), 0, 0, exist);
    offset = (int)val.getInt();
    }
  return offset;
}
//------------------------------------------------------------------
P_Expr::exprVar testManVars::getVariable(int id, int offset, int offsByStep, bool& exist)
{
  exist = true;
  int nElem = Vars.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(Vars[i].id == id)
      return loadVal(Vars[i], offset, offsByStep);
    }
  exist = false;
  P_Expr::exprVar val((__int64)id);
  return val;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
*/
