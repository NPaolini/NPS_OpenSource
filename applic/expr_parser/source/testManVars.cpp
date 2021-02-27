//-------- testManVars.cpp -----------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "testManVars.h"
#include "replaceVarName.h"
#include <stdio.h>
#include <stdlib.h>
//------------------------------------------------------------------
manExpressionVars* allocManVars(pInfoCallBack p_info_fz)
{
  return new testManVars(p_info_fz);
}
//------------------------------------------------------------------
#define ID_SHOW_ERROR 100
#define ID_INIT_SHOW_ERROR 101
#define ID_VAR_NOT_FOUND ID_INIT_SHOW_ERROR
#define ID_EXPR_ERROR 102
//------------------------------------------------------------------
void testManVars::shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR funct_name)
{
  if(!infoB)
    return;
  resolveRowCol(num2, row);
  ParserError pe;
  pe.col = num2 + 1;
  pe.row = row + 1;
  pe.varName = var;
  pe.functName = funct_name;

  if(num2 >= 0) {
    functName = funct_name;
    if(!var)
      pe.varName = _T("???");
    pe.code = ERR_EXPR_VAR;
    }
  else {
    switch(num2) {
      case -1:
        if(!id && !var) {
          pe.code = ERR_EXPR_PARAM;
          pe.col = 0;
          }
        else {
          if(!var)
            pe.varName = _T("???");
          pe.code = ERR_EXPR_VAR_NOT_FOUND;
          pe.col = 0;
          }
        break;
      case -2:
        pe.code = ERR_EXPR_DUP_LABEL;
        pe.col = 0;
        break;
/*
      case -3:
        wsprintf(buff, _T("Init -> Variabile [%s] non trovata"), var);
        break;
*/
      }
    }
  pInfoFz->fz_callBack(cb_MSG_ERROR, &pe, pInfoFz->customData);
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
      int offs = 0;
      while(t <= row) {
        if(isFalseRow(offs))
          ++row;
        ++t;
        }
      }
    }
  if(col >= 0)
    col = getTrueCol(col, row);
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------
static LPCTSTR getNextLine(LPCTSTR buff, int len)
{
  if(len <= 0)
    return 0;
  int i = 0;
  LPCTSTR p = buff;
  for(; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  if(++i == len)
    return 0;
  ++p;

  for(int j = i; j < len; ++j, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;
  return(i < len ? p : 0);
}
//------------------------------------------------------------------
struct info_line
{
  int len;
  int dim;
  LPCTSTR buff;
  info_line(PCTSTR buff = 0) : buff(buff), len(0), dim(0) {}
};
//------------------------------------------------------------------
static bool getNextLineLimit(struct info_line& il)
{
  int dim = il.dim;
  LPCTSTR p1 = getNextLine(il.buff, dim);
  if(!p1)
    return false;
  uint offs = (p1 - il.buff) / sizeof(TCHAR);
  LPCTSTR p2 = getNextLine(p1, dim - offs);
  int nchar = p2 ? (p2 - p1) / sizeof(TCHAR) : dim - offs;
  il.buff = p1;
  il.dim -= nchar;
  il.len = nchar;
  return true;
}
//------------------------------------------------------------------
extern bool isReservedVar(LPCTSTR realName);
//------------------------------------------------------------------
static bool isSkip(LPTSTR buff)
{
  TCHAR t[8] = {0};
  for(uint i = 0; i < SIZE_A(t); ++i) {
    if(_T(' ') >= buff[i] || _T('=') == buff[i]) {
      buff[i] = 0;
      break;
      }
    t[i] = buff[i];
    }
  return isReservedVar(t);
}
//------------------------------------------------------------------
static bool matchFunct(LPCTSTR buff, uint len, LPCTSTR match)
{
  uint j = 0;
  for(uint i = 0; i < len; ++i) {
    if(buff[i] == match[j]) {
      ++j;
      if(!match[j])
        return true;
      }
    else if(j) {
      i -= j;
      j = 0;
      }
    }
  return false;
}
//------------------------------------------------------------------
static int findFunct(LPCTSTR buff, LPCTSTR match, uint len, int &row)
{
  struct info_line il(buff);
  il.dim = len;
  row = 0;
  len = 0;
  LPCTSTR p = buff;
  for(;; ++row) {
    bool next = getNextLineLimit(il);
    if(!next) {
      if(matchFunct(il.buff, il.len, match))
        break;
      else
        return -1;
      }
    else {
      if(matchFunct(p, len, match))
        break;
      }
    p = il.buff;
    len = il.len;
    }
  return il.buff + il.len - buff;
}
//------------------------------------------------------------------
int testManVars::getTrueRowFunct(int row)
{
  TCHAR funct[MAX_LEN_NAME_VARS_FUNCT] = { VAR_FUNCT_CHAR, 0 };
  _tcscat_s(funct, functName);

  uint ix = epFunct;
  int dim = infoB[ix].dim;
  if(!dim)
    return 0;
  LPCTSTR buff = infoB[ix].buff;
  int ixLine = 0;
  int offs = findFunct(buff, funct, dim, ixLine);
  if(-1 == offs)
    return 0;
  buff += offs;
  dim -= offs;

  do {
    LPCTSTR next = getNextLine(buff, dim);
    offs = next ? next - buff : dim;
    int nchar = offs / sizeof(TCHAR);
    while(nchar > 0) {
      if(buff[nchar - 1] > _T(' ')) {
        break;
        }
      --nchar;
      }
    if(!nchar)
      ++row;
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
/*
      if(VAR_FUNCT_CHAR == buff[i]) {
        if(isSkip(buff + i + 1)) {
          ++row;
          break;
          }
        }
      else
*/
      if(_T(';') == buff[i] || _T('#') == buff[i] || _T('{') == buff[i]) {
        ++row;
        break;
        }
      if(buff[i] > _T(' '))
        break;
      }
    buff = next;
    dim -= offs;
    } while(dim > 0 && buff);
  return row + ixLine;
}
//------------------------------------------------------------------
int testManVars::isFalseRow(int& offs)
{
  uint ix = epCalc;
  int dim = infoB[ix].dim - offs;
  if(!dim)
    return 0;
  LPCTSTR buff = infoB[ix].buff + offs;

  LPCTSTR next = getNextLine(buff, dim);
  offs = next ? next - buff : dim;
  int nchar = offs / sizeof(TCHAR);

  while(nchar > 0) {
    if(buff[nchar - 1] > _T(' '))
      break;
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
//      case VAR_CHAR:
//        if(!isSkip(buff + i + 1))
//          break;
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
int testManVars::getTrueCol(int col, int row)
{
  uint ix = functName ?  epFunct : epCalc;
  int nchar = infoB[ix].dim;
  if(!nchar)
    return 0;
  LPCTSTR buff = infoB[ix].buff;

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
//------------------------------------------------------------------
//------------------------------------------------------------------
int testManVars::parseVar(LPCTSTR buff, int pos, int& dimVar)
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
  infoVar ivar(id, _ttoi(p));
  Vars[Vars.getElem()] = ivar;
}
//------------------------------------------------------------------
int testManVars::getTypeResult(int id)
{
  int nElem = Vars.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(Vars[i].id == id)
      return Vars[i].type;
    }
  return -1;
}
//------------------------------------------------------------------
void testManVars::sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName)
{
  bool exist;
  int offset = getTrueOffset(ie.offset, exist);
  if(!isOffsVar(ie.offset))
    offset += cycle * ie.step;
  ParserInfo pi;
  pi.id = ie.idResult;
  pi.offset = offset;
  if(!realName && ReplaceVar)
    realName = ReplaceVar->getName(ie.idResult);
  pi.varName = realName;
//  if(P_Expr::tvInt == val.type) {
  if(P_Expr::tvInt == ie.typeResult) {
    pi.i = val.getInt();
    pi.type = itv_Int;
    }
  else {
    pi.d = val.getReal();
    pi.type = itv_real;
    }

  pInfoFz->fz_callBack(cb_MSG_SET_VAR, &pi, pInfoFz->customData);
}
//------------------------------------------------------------------
P_Expr::exprVar testManVars::loadVal(const infoVar& ivar, int offset)
{
  P_Expr::exprVar dummy;
  bool exist = true;
  offset = getTrueOffset(offset, exist);
  LPCTSTR realName = 0;
  if(ReplaceVar)
    realName = ReplaceVar->getName(ivar.id);
  if(!exist)
    shoMsg(ID_VAR_NOT_FOUND, realName, -1, getCurrRow());

  ParserInfo pi;
  pi.id = ivar.id;
  pi.offset = offset;
  pi.varName = realName;
  pi.i = 0;
  if(P_Expr::tvInt == ivar.type)
    pi.type = itv_Int;
  else
    pi.type = itv_real;

  pInfoFz->fz_callBack(cb_MSG_GET_VAR, &pi, pInfoFz->customData);
  if(P_Expr::tvInt == ivar.type)
    return P_Expr::exprVar(pi.i);
  return P_Expr::exprVar(pi.d);
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
int testManVars::getTrueOffset(int offset, bool& exist)
{
  if(isOffsVar(offset)) {
    P_Expr::exprVar val = getVariable(unmakeOffsVar(offset), 0, exist);
    offset = (int)val.getInt();
    }
  return offset;
}
//------------------------------------------------------------------
P_Expr::exprVar testManVars::getVariable(int id, int offset, bool& exist)
{
  exist = true;
  int nElem = Vars.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(Vars[i].id == id)
      return loadVal(Vars[i], offset);
    }
  exist = false;
  P_Expr::exprVar val((__int64)id);
  return val;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
