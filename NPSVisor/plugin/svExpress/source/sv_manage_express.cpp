//-------- sv_manage_express.cpp -----------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "sv_manage_express.h"
#include "replaceVarName.h"
#include "sv_manage_skip.h"
//------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>
//------------------------------------------------------------------
svManExpression::svManExpression() : currState(sUndef), pManVars(allocManVars()), ReplaceVar(0), pManFunct(0), Error(false) {}
//------------------------------------------------------------------
svManExpression::~svManExpression()
{
  int nElem = pExpr.getElem();
  for(int i = 0; i < nElem; ++i)
    delete pExpr[i].Expr;
  delete pManVars;
  delete ReplaceVar;
  delete pManFunct;
}
//------------------------------------------------------------------
void svManExpression::shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR functName)
{
  pManVars->shoMsg(id, var, num2, row, functName);
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------
LPTSTR getLine(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(int j = i; j < len; ++j, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//------------------------------------------------------------------
LPTSTR findAndLoadblock(LPCTSTR buff, LPCTSTR marker, uint& dim)
{
  TCHAR mark[32] = _T("<");
  _tcscat_s(mark, marker);
  LPTSTR p = StrStrI(buff, mark);
  if(p) {
    p += _tcslen(mark);
    while(*p && (unsigned)*p <= _T(' '))
      ++p;
    mark[1] = _T('/');
    mark[2] = 0;
    _tcscat_s(mark, marker);
    LPTSTR p2 = StrStrI(p, mark);
    if(p2) {
      --p2;
      while(*p2 && (unsigned)*p2 <= _T(' ') && p2 > p)
        --p2;
      ++p2;
      dim = p2 - p + 1;
      if(dim > 5) {
        LPTSTR ret = new TCHAR[dim + 4];
        LPTSTR p3 = ret;
        while(p < p2)
          *p3++ = *p++;
        *p3 = 0;
        return ret;
        }
      }
    }
  return 0;
}
//------------------------------------------------------------------
static LPCTSTR Marker[] = {
    _T("VARS>"), // zona delle variabili
    _T("FUNCT>"), // zona delle funzioni
    _T("CALC>"), // zona del calcolo
    _T("INIT>"), // zona delle inizializzazioni delle variabili
    };
enum ePartBlock { epVars, epFunct, epCalc, epInit };
//------------------------------------------------------------------
// da spostare sul file principale
#define TO_UP(a) _totupper((unsigned)(a))
//------------------------------------------------------------------
bool equMarker(LPCTSTR mark, LPCTSTR test)
{
  if(!mark || !test)
    return false;
  while(*mark) {
    if(TO_UP(*mark) != TO_UP(*test))
      return false;
    if(_T('>') == *mark)
      return true;
    ++mark;
    ++test;
    }
  return false;
}
//------------------------------------------------------------------
void svManExpression::manageInit(LPTSTR p)
{
  pManVars->manageInit(p);
}
//------------------------------------------------------------------
void svManExpression::manageVar(LPTSTR p)
{
  pManVars->manageVar(p);
}
//------------------------------------------------------------------
int svManExpression::getTypeResult(int id)
{
  return pManVars->getTypeResult(id);
}
//------------------------------------------------------------------
void svManExpression::manageCalc(LPTSTR p)
{
  infoExpr ie;
  ZeroMemory(&ie, sizeof(ie));
  ie.step = 1;
  ie.cycle = 1;
  ie.idResult = _ttoi(p);
  for(;*p; ++p) {
    if(_T('[') == *p) {
      ++p;
      if(VAR_CHAR == *p) {
        ++p;
        ie.offset = makeOffsVar(_ttoi(p));
        }
      else
        ie.offset = _ttoi(p);
      ++p;
      int next = 0;
      while(*p) {
        if(_T(']') == *p) {
          ++p;
          break;
          }
        if(_T(',') == *p) {
          ++p;
          if(!next) {
            ie.step = _ttoi(p);
            ++next;
            }
          else if(1 == next) {
            ie.cycle = _ttoi(p);
            ++next;
            }
          }
        else
          ++p;
        }
      }
    if(_T('=') == *p) {
      ++p;
      break;
      }
    }
  if(!*p)
    return;
  ie.typeResult = getTypeResult(ie.idResult);
  ie.Expr = new svExpr(this, p);
  int result = ie.Expr->UpdateTree();
  if(-1 != result) {
    LPCTSTR v = ReplaceVar->getName(ie.idResult);
//    uint t = v ? _ttoi(v) : ie.idResult;
    shoMsg(ID_EXPR_ERROR, v, result, pExpr.getElem());
    Error = true;
    delete ie.Expr;
#if 1
    // meglio non far proseguire, potrebbero innescarsi loop infiniti
    // se la riga incriminata agisce su test o contatori
    ie.idResult = 2;
    LPCTSTR skip = _T("1");
//    LPCTSTR skip = _T("$_ABORT=1");

    ie.Expr = new svExpr(this, skip);
    ie.Expr->UpdateTree();

    // anzi, meglio far abortire completamente lo script
    uint nEl = pExpr.getElem();
    // se ci sono elementi, sostituisce il primo con l'abort e termina
    // altrimenti esegue l'assegnazione normale
    if(nEl) {
      delete pExpr[0].Expr;
      pExpr[0] = ie;
      return;
      }
#else
    return;
#endif
    }
  pExpr[pExpr.getElem()] = ie;
}
//------------------------------------------------------------------
void svManExpression::manageLine(LPTSTR p)
{
  if(!p)
    return;
  while(*p && *p != VAR_CHAR)
    ++p;

  if(VAR_CHAR == *p) {
    if(sVars == currState)
      manageVar(p + 1);
    else if(sCalc == currState)
      manageCalc(p + 1);
    else if(sInitialize == currState)
      manageInit(p + 1);
    }
}
//------------------------------------------------------------------
/*
void svManExpression::manageLine(LPTSTR p)
{
  if(!p)
    return;
  while(*p && *p != VAR_CHAR && *p != VAR_FUNCT_CHAR && *p != _T(';') && *p != _T('<'))
    ++p;

  if(_T('<') == *p)
    manageMarker(p + 1);

  else if(VAR_CHAR == *p) {
    if(sVars == currState)
      manageVar(p + 1);
    else if(sCalc == currState)
      manageCalc(p + 1);
    else if(sInitialize == currState)
      manageInit(p + 1);
    }
  else if(VAR_FUNCT_CHAR == *p) {
    if(sFunct == currState)
      manageFunctVar(p + 1);
    }
}
*/
//------------------------------------------------------------------
bool isReservedVar(LPCTSTR realName)
{
  return !_tcsicmp(realName, SKIP_NAME) || !_tcsicmp(realName, ABORT_NAME);
}
//------------------------------------------------------------------
#define SKIP_CHAR_INIT _T('#')
//------------------------------------------------------------------
static
void replaceBreakLine(LPTSTR buff, int len)
{
  // salvaguarda gli ultimi due caratteri, dovrebbero essere CR-NL
  // e comunque non dovrebbe essere necessario arrivare alla fine del buffer
  len -= 2;
  // prima sostituisce i tab con spazi (per eliminare il problema di caratteri minori spazio
  for(int i = 0; i < len; ++i)
    if(_T('\t') == buff[i])
      buff[i] = _T(' ');

  for(int i = 0; i < len; ++i) {
    if(_T('\\') == buff[i]) {
      buff[i++] = _T(' ');
      // cicla fino a trovare un carattere minore di spazio, tipicamente CR
      for(; i < len; ++i) {
        if(buff[i] < _T(' '))
          break;
        buff[i] = _T(' ');
        }
      // poi continua il ciclo per trovare l'inizio della riga successiva
      for(; i < len; ++i) {
        if(buff[i] >= _T(' '))
          break;
        buff[i] = _T(' ');
        }
      }
    }
}
//------------------------------------------------------------------
LPTSTR replaceSkip(LPTSTR buff, uint& len, manExpressionVars* manV, bool& canDelete)
{
  if(!_tcschr(buff, SKIP_CHAR_INIT))
    return buff;
  LPTSTR b = svManSkip(manV).run(buff, len);
  if(b) {
    if(canDelete)
      delete []buff;
    canDelete = true;
    return b;
    }
  if(!len) {
    if(canDelete)
      delete []buff;
    return 0;
    }
  return buff;
}
//---------------------------------------------------------------------
const int CRYPT_STEP = 1;
int get_CRYPT_STEP() { return CRYPT_STEP; }
LPCSTR Header = "NPS sys ";
LPCSTR Key = "eNnePiSoft";
//---------------------------------------------------------------------
#ifdef FROM_SVX_EDITOR
//---------------------------------------------------------------------
static bool openClear(LPCTSTR file, infoFileCr& result)
{
  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  result.dim = (int)pf.get_len();
  if(!result.dim)
    return false;

  result.buff = new BYTE[result.dim];
  pf.P_read(result.buff, result.dim);
  return true;
}
#endif
//---------------------------------------------------------------------
LPTSTR openAndDecrypt(LPCTSTR file, uint& dim)
{
  infoFileCr result;
  result.header = (LPCBYTE)Header;
  result.lenHeader = strlen(Header);
  if(!decryptFile(file, (LPCBYTE)Key, strlen(Key), CRYPT_STEP, result))
#ifdef FROM_SVX_EDITOR
    if(!openClear(file, result))
#endif
      return 0;
  LPBYTE t = new BYTE[result.dim + 2];
  CopyMemory(t, result.buff, result.dim);
  t[result.dim] = 0;
  t[result.dim + 1] = 0;
  delete []result.buff;
  dim = result.dim + 2;
  LPTSTR text = autoConvert(t, dim);
  if(!text) {
    delete []t;
    return 0;
    }
  return text;
}
//------------------------------------------------------------------
bool svManExpression::makeExpression(LPCTSTR path)
{
  int nElem = pExpr.getElem();
  for(int i = 0; i < nElem; ++i)
    delete pExpr[i].Expr;
  pExpr.reset();
  pManVars->reset();
  // non va messo qui, è globale e resetta il contatore anche per lo script precedentemente processato
//  resetCountOnMem();
  resetError();
  uint len = 0;
  LPTSTR buff = openAndDecrypt(path, len);
  if(!buff)
    return false;
  struct infoBuff
  {
    LPTSTR buff;
    uint dim;
    infoBuff() : buff(0), dim(0) {}
  };
  infoBuff partBuff[SIZE_A_c(Marker)];
  for(uint i = 0; i < SIZE_A_c(Marker); ++i) {
    partBuff[i].buff = findAndLoadblock(buff, Marker[i], partBuff[i].dim);
    if(partBuff[i].buff) {
      replaceBreakLine(partBuff[i].buff, partBuff[i].dim);
      if(epFunct != i) {
        bool canDelete = true;
        partBuff[i].buff = replaceSkip(partBuff[i].buff, partBuff[i].dim, pManVars, canDelete);
        }
      }
    }

  if(!partBuff[epCalc].buff || !partBuff[epVars].buff) {
    for(uint i = 0; i < SIZE_A(Marker); ++i)
      delete []partBuff[i].buff;
    return false;
    }

  delete ReplaceVar;
  ReplaceVar = allocReplaceVarName();
  if(ReplaceVar) {
    // epVars, epFunct, epCalc, epInit
    int lValue[] = { 1, 0, 0, 2 };
    bool first = true;
    for(uint i = 0; i < SIZE_A(Marker); ++i) {
      bool canDelete = true;
      partBuff[i].buff = ReplaceVar->run(partBuff[i].buff, partBuff[i].dim, canDelete, lValue[i], first);
      first = false;
      }
    }

  pManVars->setReplaceVar(ReplaceVar);

  if(partBuff[epFunct].buff && partBuff[epFunct].dim > 20) {
    pManFunct = new manExpressionFunct(this);
    pManFunct->parse(partBuff[epFunct].buff, partBuff[epFunct].dim);
    }

  struct infoManageLine
  {
    uint state;
    uint ix;
  };

  infoManageLine iManLine[] = {
    { sVars, epVars },
    { sCalc, epCalc },
    { sInitialize, epInit },
    };

  for(uint i = 0; i < SIZE_A(iManLine); ++i) {
    currState = iManLine[i].state;
    uint ix = iManLine[i].ix;
    LPTSTR p = partBuff[ix].buff;
    uint len = partBuff[ix].dim;
    while(len && p) {
      LPTSTR p2 = getLine(p, len);
      int offs = p2 ? p2 - p : len;
      manageLine(p);
      len -= offs;
      p = p2;
      }
    }
  for(uint i = 0; i < SIZE_A(Marker); ++i)
    delete []partBuff[i].buff;

  nElem = pExpr.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(-1 == pExpr[i].typeResult) {
      pExpr[i].typeResult = getTypeResult(pExpr[i].idResult);
      if(-1 == pExpr[i].typeResult) {
        LPCTSTR v = ReplaceVar->getName(pExpr[i].idResult);
        if(!isReservedVar(v)) {
          shoMsg(ID_VAR_NOT_FOUND, v, 0, i);
          return false;
          }
        }
      }
    }
  pManVars->performInit();
  return !hasError();
}
//------------------------------------------------------------------
bool operator ==(const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  if(v1.type != v2.type)
    return false;
  if(P_Expr::tvReal == v1.type) {
    double v = v1.dValue - v2.dValue;
    if(v < 0)
      v = -v;
    return v <= dPRECISION;
    }
  return v1.iValue == v2.iValue;
}
//------------------------------------------------------------------
void svManExpression::sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName)
{
  pManVars->sendResult(ie, val, cycle, realName);
}
//------------------------------------------------------------------
static bool checkReservedVar(int& curr, int nElem, P_Expr::exprVar val, LPCTSTR realName)
{
  if(!_tcsicmp(realName, SKIP_NAME)) {
    int t = (int)val.getInt();
    curr += t;
    if(t < 0)
      --curr;
    if(curr < -1)
      curr = -1;
    return true;
    }
  if(!_tcsicmp(realName, ABORT_NAME)) {
    if(val.getInt())
      curr = nElem;
    return true;
    }
  return false;
}
//------------------------------------------------------------------
static int gVarId;
void setIdVarG(int v) { gVarId = v; }
//------------------------------------------------------------------
static int gVarIdFunct;
void setIdVarGFunct(int v) { gVarIdFunct = v; }
//------------------------------------------------------------------
#define MAX_ITER 1000000
//------------------------------------------------------------------
LPCTSTR svManExpression::getRealName(uint id)
{
  if(ReplaceVar)
    return ReplaceVar->getName(id);
  return 0;
}
//------------------------------------------------------------------
bool svManExpression::performExpression()
{
  int nElem = pExpr.getElem();
  if(!nElem)
    return false;
  pManVars->reloadCache();
  gVarId = 0;
  pManVars->setCurrRow(0);
  for(int i = 0, k = 0; i < nElem; ++i, ++k) {
    if(k >= MAX_ITER)
      return false;
    infoExpr& ie = pExpr[i];
    int repeat = ie.cycle;
    if(!repeat)
      repeat = 1;
    pManVars->setCurrRow(i);
    for(int j = 0; j < repeat; ++j) {
      P_Expr::exprVar val(ie.typeResult);
      P_Expr::errCode err = ie.Expr->getValue(val, j);
      if(P_Expr::NO_ERR == err) {
        LPCTSTR realName = 0;
        if(ReplaceVar)
          realName = ReplaceVar->getName(ie.idResult);
        if(1 != repeat || !checkReservedVar(i, nElem, val, realName))
          sendResult(ie, val, j, realName);
        }
      else if(P_Expr::NO_ERR < err) {
        // msg
        if(P_Expr::UNDEFINED_VARIABLE == err) {
          LPCTSTR realName = 0;
          if(ReplaceVar && gVarId)
            realName = ReplaceVar->getName(gVarId);
          gVarId = 0;
          shoMsg(ID_VAR_NOT_FOUND, realName, -1, i);
          }
        else if(P_Expr::INVALID_EXPRESSION == err) {
          // i primi due valori sono abort e skip, probabilmente solo abort
          // può dare errore
          if(ie.idResult > 2)
            shoMsg(0, 0, -1, i);
          }
        else if(P_Expr::ERR_ON_FUNCT == err) {
          // int id, LPCTSTR var, int num2, int row, LPCTSTR funct_name
          LPCTSTR funct_name = this->FunctStack.getCurr()->getName();
          shoMsg(0, 0, -1, i, funct_name);
          popFunct();
          }
        return false;
        }
      }
    }
  pManVars->sendFinal();
  return true;
}
//------------------------------------------------------------------
P_Expr::exprVar svManExpression::getVariable(int id, int offset, int offsByStep, bool& exist)
{
  return pManVars->getVariable(id, offset, offsByStep, exist);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#define MARK_END_FUNCT _T("}")
//---------------------------------------------------------------------
void manExpressionFunct::parse(LPTSTR buff, uint dim)
{
  do {
    while(*buff && *buff != VAR_FUNCT_CHAR)
      ++buff;
    LPTSTR p = StrStrI(buff, MARK_END_FUNCT);
    if(p) {
      ++p;
      int len = p - buff + 1;
      LPTSTR ret = new TCHAR[len + 4];
      LPTSTR p3 = ret;
      while(buff < p)
        *p3++ = *buff++;
      *p3 = 0;
      P_CustFunct* pF = new svCustFunct(this);
      if(pF->parse(ret, len)) {
        uint nElem = Functs.getElem();
        bool exist = false;
        for(uint i = 0; i < nElem; ++i) {
          LPCTSTR functname = pF->getName();
          if(!_tcsicmp(functname, Functs[i]->getName())) {
            Functs[i]->moveFrom(pF);
            delete pF;
            exist = true;
            break;
            }
          }
        if(!exist)
          Functs[Functs.getElem()] = pF;
        }
      else
        delete pF;
      delete []ret;
      }
    buff = p;
    } while(buff && *buff);
}
//---------------------------------------------------------------------
P_CustFunct* manExpressionFunct::alloc(LPCTSTR functname)
{
  uint nElem = Functs.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_tcsicmp(functname, Functs[i]->getName()))
      return Functs[i];

  P_CustFunct* pF = new svCustFunct(this);
  pF->setName(functname);
  Functs[nElem] = pF;
  return pF;
}
//------------------------------------------------------------------
LPCTSTR manExpressionFunct::getRealName(uint id)
{
  return Owner->getRealName(id);
}
//------------------------------------------------------------------
void  manExpressionFunct::shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR functName)
{
  Owner->shoMsg(id, var, num2, row, functName);
}
//---------------------------------------------------------------------
manExpressionVars* manExpressionFunct::getManVars() { return Owner->getManVars(); }
//---------------------------------------------------------------------
//---------------------------------------------------------------------
svCustFunct::~svCustFunct()
{
  int nElem = pExpr.getElem();
  for(int i = 0; i < nElem; ++i)
    delete pExpr[i].Expr;
  delete ReplaceVar;
}
//---------------------------------------------------------------------
void svCustFunct::moveFrom(const P_CustFunct* other)
{
  if(other == this)
    return;
  svCustFunct* o = (svCustFunct*)other;
  nArg = o->nArg;
  int nElem = pExpr.getElem();
  for(int i = 0; i < nElem; ++i)
    delete pExpr[i].Expr;

  nElem = o->pExpr.getElem();
  pExpr.setDim(nElem);
  for(int i = 0; i < nElem; ++i) {
    pExpr[i] = o->pExpr[i];
    o->pExpr[i].Expr = 0;
    }
  o->pExpr.reset();
  delete ReplaceVar;
  ReplaceVar = o->ReplaceVar;
  o->ReplaceVar = 0;
  Owner = o->Owner;
}
//---------------------------------------------------------------------
void svCustFunct::setName(LPCTSTR name)
{
  if(!ReplaceVar)
    ReplaceVar = allocReplaceVarName4Funct();
  ReplaceVar->setStartId(ID_FUNCT_NAME);
  uint id = ReplaceVar->getIdAdd(name);
  ReplaceVar->resetStartId();
}
//---------------------------------------------------------------------
void svCustFunct::shoMsg(int id, LPCTSTR var, int num2, int row)
{
  Owner->shoMsg(id, var, num2, row, getName());
}
//---------------------------------------------------------------------
bool svCustFunct::parse(LPTSTR buff, uint len)
{
  bool canDelete1 = false;
  LPTSTR buff1 = replaceSkip(buff, len, Owner->getManVars(), canDelete1);

  if(!ReplaceVar)
    ReplaceVar = allocReplaceVarName4Funct();
  bool canDelete = canDelete1;
  LPTSTR buff2 = 0;
  if(ReplaceVar)
    buff2 = ReplaceVar->run(buff1, len, canDelete, 0, true);
  else
    return false;

  LPTSTR p = buff2;
  LPTSTR p2 = 0;
  while(true) {
    p2 = getLine(p, len);
    int offs = p2 ? p2 - p : len;
    int result = manageNameParam(p);
    if(result < 0) {
      if(buff2 != buff || canDelete)
        delete []buff2;
      if(!canDelete && canDelete1)
        delete []buff1;
      return false;
      }
    len -= offs;
    p = p2;
    if(result > 0)
      break;
    }
  while(len && p) {
    p2 = getLine(p, len);
    int offs = p2 ? p2 - p : len;
    manageLine(p);
    len -= offs;
    p = p2;
    }
  if(buff2 != buff || canDelete)
    delete []buff2;
  if(!canDelete && canDelete1)
    delete []buff1;
  return true;
}
//---------------------------------------------------------------------
LPCTSTR svCustFunct::getName() const
{
  return ReplaceVar->getName(ID_FUNCT_NAME);
}
//---------------------------------------------------------------------
P_Expr::errCode svCustFunct::execute(P_Expression* owner, P_Expr::exprVar& result, PVect<P_Expr::exprVar>& v)
{
  param.copy(v, 0);

  gVarIdFunct = 0;
  int nElem = pExpr.getElem();
  if(!nElem)
    return P_Expr::INVALID_EXPRESSION;
  for(int i = 0, k = 0; i < nElem; ++i, ++k) {
    if(k >= MAX_ITER)
      return P_Expr::INVALID_EXPRESSION;
    infoExpr& ie = pExpr[i];
    P_Expr::exprVar val(ie.typeResult);
    P_Expr::errCode err = ie.Expr->getValue(val, 0);
    if(P_Expr::NO_ERR == err) {
      // se è l'id corrispondente al nome della funzione, esce e torna il valore
      if(ID_FUNCT_NAME == ie.idResult) {
        result = val;
        return P_Expr::NO_ERR;
        }
      LPCTSTR realName = 0;
      if(ReplaceVar)
        realName = ReplaceVar->getName(ie.idResult);
      if(!checkReservedVar(i, nElem, val, realName))
        sendResult(ie, val, 0, realName);
      }
    else if(P_Expr::NO_ERR < err) {
      // msg
      if(P_Expr::UNDEFINED_VARIABLE == err) {
        LPCTSTR realName = 0;
        if(ReplaceVar && gVarIdFunct)
          realName = ReplaceVar->getName(gVarIdFunct);
        else if(gVarId) {
          realName = Owner->getRealName(gVarId);
          gVarId = 0;
          }
        gVarIdFunct = 0;
        shoMsg(ID_VAR_NOT_FOUND, realName, -1, i);
        }
      else if(P_Expr::INVALID_EXPRESSION == err) {
        // i primi valori sono abort, skip e nome funzione, probabilmente solo abort
        // può dare errore
        if(ie.idResult > 3)
          shoMsg(0, 0, -1, i);
        }
      return err;
      }
    }
  // se esce qui o non è stato impostato il nome della funzione come left value oppure ...
  result.nullVal = true;
  return P_Expr::NO_ACTION;
}
//------------------------------------------------------------------
void svCustFunct::sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName)
{
  // per ora non gestiamo array locali
  if(isOffsVar(ie.offset))
    return;

  uint id = ie.idResult;
  if(id >= MAX_PARAM_ID) {
    id -= MAX_PARAM_ID;
    local[id] = val;
    }
  else {
    id -= OFFS_PARAM;
    if((uint)id < (int)param.getElem())
      param[id] = val;
    }
}
//------------------------------------------------------------------
P_Expr::exprVar svCustFunct::getVariable(int id, int offset, int offsByStep, bool& exist)
{
  exist = true;
  if(id >= MAX_PARAM_ID) {
    id -= MAX_PARAM_ID;
    return local[id];
    }
  else {
    id -= OFFS_PARAM;
    if((uint)id < (int)param.getElem())
      return param[id];
    }
  exist = false;
  P_Expr::exprVar v;
  return v;
}
//------------------------------------------------------------------
void svCustFunct::manageCalc(LPTSTR p)
{
  infoExpr ie;
  ZeroMemory(&ie, sizeof(ie));
  ie.step = 1;
  ie.cycle = 1;
  ie.idResult = _ttoi(p);
  for(;*p; ++p) {
    if(_T('=') == *p) {
      ++p;
      break;
      }
    }
  if(!*p)
    return;
  ie.typeResult = -1;
  ie.Expr = new svExpr(Owner->getMainOwner(), p);
  int result = ie.Expr->UpdateTree();
  if(-1 != result) {
    LPCTSTR v = ReplaceVar->getName(ie.idResult);
    shoMsg(ID_EXPR_ERROR, v, result, pExpr.getElem());
    delete ie.Expr;
#if 1
    // meglio non far proseguire, potrebbero innescarsi loop infiniti
    // se la riga incriminata agisce su test o contatori
    ie.idResult = 2;
    LPCTSTR skip = _T("1");
//    LPCTSTR skip = _T("$_ABORT=1");

    ie.Expr = new svExpr(Owner->getMainOwner(), skip);
    ie.Expr->UpdateTree();

    // anzi, meglio far abortire completamente lo script
    uint nEl = pExpr.getElem();
    // se ci sono elementi, sostituisce il primo con l'abort e termina
    // altrimenti esegue l'assegnazione normale
    if(nEl) {
      delete pExpr[0].Expr;
      pExpr[0] = ie;
      return;
      }
#else
    return;
#endif
    }
  pExpr[pExpr.getElem()] = ie;
}
//------------------------------------------------------------------
void svCustFunct::manageLine(LPTSTR p)
{
  if(!p)
    return;
  while(*p && *p != VAR_FUNCT_CHAR)
    ++p;

  if(VAR_FUNCT_CHAR == *p)
    manageCalc(p + 1);
}
//---------------------------------------------------------
inline
bool isValid(unsigned v)
{
  return _istalnum(v) || _T('_') == v;
}
//------------------------------------------------------------------
int svCustFunct::manageNameParam(LPTSTR p)
{
  if(!p)
    return -1;
  while(*p && *p != VAR_FUNCT_CHAR)
    ++p;

  // non ha trovato l'inizio del nome della funzione, non è una riga valida
  // prova con la successiva
  if(!*p || VAR_FUNCT_CHAR != *p)
    return 0;

  ++p;
  while(*p && *p != _T('(') && isValid(*p))
    ++p;

  // non ha trovato l'inizio dei parametri della funzione, c'è qualche errore
  // termina il ciclo
  if(!*p || _T('(') != *p)
    return -1;

  // semplice verifica di quanti VAR_FUNCT_CHAR(@) ci sono ...
  nArg = 0;
  while(*p) {
    if(*p == VAR_FUNCT_CHAR)
      ++nArg;
    ++p;
    }
  return 1;
}
