//----------------- p_express.cpp ----------------------------------------
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include <stdlib.h>
#include "p_express.h"
#include "p_operation.h"
//------------------------------------------------------------------------
static uint gCountForMem;
//------------------------------------------------------------------------
uint getNextCountOnMem()
{
  uint old = gCountForMem;
  ++gCountForMem;
  ++gCountForMem;
  return old;
}
//------------------------------------------------------------------------
void resetCountOnMem()
{
  gCountForMem = 0;
}
//------------------------------------------------------------------------
P_Expr::node::node(const P_Expr::node& other) : oper(0)
{
  *this = other;
}
//------------------------------------------------------------------------
const P_Expr::node& P_Expr::node::operator=(const P_Expr::node& other)
{
  if(&other != this) {
    free();
    left = 0;
    right = 0;
    more = 0;
    if(other.oper)
      oper = other.oper->makeObj();
    else
      oper = 0;
    Val = other.Val;
    }
  return *this;
}
//------------------------------------------------------------------------
void P_Expr::node::free()
{
  if(left)
    left->free();
  if(right)
    right->free();
  if(more)
    more->free();
  delete oper;
/*
  left = 0;
  right = 0;
  more = 0;
  oper = 0;
*/
}
//------------------------------------------------------------------------
P_Expr::errCode P_Expr::node::calc(P_Expression* owner, P_Expr::exprVar& result)
{
  return oper->calc(owner, result, this);
}
//------------------------------------------------------------------------
LPCTSTR P_Expr::str_newdupStripSpace(LPCTSTR p)
{
  if(!p)
    p = _T("\0");
  int len = _tcslen(p);
  LPTSTR t = new TCHAR[len + 2];
  int j = 0;
  for(int i = 0; i < len; ++i)
    if(_T(' ') != p[i])
      t[j++] = p[i];
  for(; j < len + 2; ++j)
    t[j] = 0;
  return t;
}
//------------------------------------------------------------------------
P_Expression::P_Expression(LPCTSTR expr) : Radix(0), currPos(0),
          Expr(P_Expr::str_newdupStripSpace(expr)), desiredType(0), Cycle(0)
{
}
//------------------------------------------------------------------------
P_Expression::~P_Expression()
{
  delete Radix;
  delete []Expr;
}
//------------------------------------------------------------------------
int P_Expression::ChangeExpression(LPCTSTR expr)
{
  delete []Expr;
  Expr = P_Expr::str_newdupStripSpace(expr);
  return UpdateTree();
}
//------------------------------------------------------------------------
int P_Expression::UpdateTree()
{
  if(!Expr || !*Expr)
    return 0;
  delete Radix;
  Radix = 0;
  currPos = 0;
  Radix = makeLevel0_0();
  if(Expr[currPos])  {
    delete Radix;
    Radix = 0;
    }

  return Radix ? -1 : currPos;
}
//------------------------------------------------------------------------
P_Expr::errCode P_Expression::getValue(P_Expr::exprVar& val, int cycle)
{
  if(!Radix)
    return P_Expr::INVALID_EXPRESSION;

  Cycle = cycle;
  desiredType = val.type;
  Result = P_Expr::NO_ERR;
  val = vexp(Radix);
  if(val.nullVal)
    return P_Expr::NO_ACTION;

  if(desiredType >= 0 && desiredType != val.type) {
    if(desiredType)
      val.iValue = ROUND_REAL_TO_i64(val.dValue);
    else
      val.dValue = (double)val.iValue;
    val.type = desiredType;
    }
  return(Result);
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel0_0()
{
  pTree left = makeLevel0_1();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('|')) && (Expr[currPos + 1] == _T('|')))  {
    operatBase* op = allocOper(oORL);
    pTree nod = new Tree(op, left);
    ++currPos;
    ++currPos;
    nod->right = makeLevel0_1();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel0_1()
{
  pTree left = makeLevel0_2();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('^')) && (Expr[currPos + 1] == _T('^')))  {
    operatBase* op = allocOper(oXORL);
    pTree nod = new Tree(op, left);
    ++currPos;
    ++currPos;
    nod->right = makeLevel0_2();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel0_2()
{
  pTree left = makeLevel1_0();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('&')) && (Expr[currPos + 1] == _T('&')))  {
    operatBase* op = allocOper(oANDL);
    pTree nod = new Tree(op, left);
    ++currPos;
    ++currPos;
    nod->right = makeLevel1_0();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel1_0()
{
  pTree left = makeLevel1_1();
  if(!left)
    return 0;
  while(Expr[currPos] == _T('|') && Expr[currPos + 1] != _T('|'))  {
    operatBase* op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel1_1();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel1_1()
{
  pTree left = makeLevel1_2();
  if(!left)
    return 0;
  while(Expr[currPos] == _T('^') && Expr[currPos + 1] != _T('^'))  {
    operatBase* op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel1_2();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel1_2()
{
  pTree left = makeLevel2();
  if(!left)
    return 0;
  while(Expr[currPos] == _T('&') && Expr[currPos + 1] != _T('&'))  {
    operatBase* op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel2();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel2()
{
  pTree left = makeLevel3();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('=') && Expr[currPos + 1] == _T('=')) || (Expr[currPos] == _T('!') && Expr[currPos + 1] == _T('=')))  {
    int o = _T('=') == Expr[currPos] ? oEQU : oDIFF;
    operatBase* op = allocOper(o);
    pTree nod = new Tree(op, left);
    ++currPos;
    ++currPos;
    nod->right = makeLevel3();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel3()
{
  pTree left = makeLevel4();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('<') && Expr[currPos + 1] != _T('<')) || (Expr[currPos] == _T('>') && Expr[currPos + 1] != _T('>')))  {
    operatBase* op;
    if(_T('=') == Expr[currPos + 1]) {
      int o = _T('<') == Expr[currPos] ? oLESS_OR_EQU : oGREAT_OR_EQU;
      op = allocOper(o);
      ++currPos;
      }
    else
      op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel4();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel4()
{
  pTree left = makeLevel5();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('<') && Expr[currPos + 1] == _T('<')) || (Expr[currPos] == _T('>') && Expr[currPos + 1] == _T('>')))  {
    int o = _T('<') == Expr[currPos] ? oSHL : oSHR;
    operatBase* op = allocOper(o);
    pTree nod = new Tree(op, left);
    ++currPos;
    ++currPos;
    nod->right = makeLevel5();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel5()
{
  pTree left = makeLevel6();
  if(!left)
    return 0;
  while((Expr[currPos] == _T('-')) || (Expr[currPos] == _T('+')))  {
    operatBase* op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel6();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
#define makeLevel7 makeLevelHigh
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevel6()
{
  pTree left = makeLevel7();
  if(!left)
    return 0;

  while((Expr[currPos] == _T('*')) || (Expr[currPos] == _T('/')) || (Expr[currPos] == _T('%')))  {
    operatBase* op = allocOper(Expr[currPos]);
    pTree nod = new Tree(op, left);
    ++currPos;
    nod->right = makeLevel7();
    if(!nod->right)  {
      delete nod;
      return 0;
      }
    left = nod;
    }
  return left;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevelHigh()
{
  if(Expr[currPos] == _T('!') || Expr[currPos] == _T('~') || Expr[currPos] == _T('-'))  {
    TCHAR car = Expr[currPos];
    if(_T('-') == car)
      car = (TCHAR)fNEG;
    pTree nod = new Tree(allocOper(car));
    ++currPos;
    if(!nod)
      return 0;
    nod->left = makeLevelHigh();
    if(!nod->left) {
      delete nod;
      return 0;
      }
    return nod;
    }

  // per funzioni con più argomenti occorre verificare anche la virgola
  if(Expr[currPos] == _T('(') || Expr[currPos] == _T(',')) {
    ++currPos;
    pTree nod = makeLevel0_0();
    if(!nod)
      return 0;

    if(Expr[currPos] != _T(')') && Expr[currPos] != _T(','))  {
      delete nod;
      return 0;
      }
    // se ci sono altri argomenti non deve saltare la virgola
    if(Expr[currPos] != _T(','))
      ++currPos;
    return nod;
    }
  return makeLevelLast(); //makeIdentificator();
}
//------------------------------------------------------------------------
#define CODE_NAME(a) { _T(#a), f##a }
//------------------------------------------------------------------------
static
int getCodeFunct(LPCTSTR t)
{
  static struct
  {
    LPCTSTR name;
    int code;
  } codename [] = {
    CODE_NAME(ABS),
    CODE_NAME(NEG),
    CODE_NAME(NOTL),
    CODE_NAME(NOTB),
    CODE_NAME(SIN),
    CODE_NAME(COS),
    CODE_NAME(EXP),
    CODE_NAME(SQRT),
    CODE_NAME(LOG),
    CODE_NAME(TG),
    CODE_NAME(CTG),
    CODE_NAME(ASIN),
    CODE_NAME(ACOS),
    CODE_NAME(ATG),
    CODE_NAME(RAD),
    CODE_NAME(GRAD),

    CODE_NAME(SWAB2),
    CODE_NAME(SWAB4),
    CODE_NAME(SWAB8),
    CODE_NAME(TOBCD_LE),
    CODE_NAME(FROMBCD_LE),
    CODE_NAME(TOBCD_BE),
    CODE_NAME(FROMBCD_BE),
    CODE_NAME(RANDMINMAX),

    CODE_NAME(POW),
    CODE_NAME(MIN),
    CODE_NAME(MAX),
    CODE_NAME(SHL),
    CODE_NAME(SHR),
    CODE_NAME(EQU),

    CODE_NAME(IF),
    CODE_NAME(SUM),
    CODE_NAME(AVR),

    CODE_NAME(BLKSUM),
    CODE_NAME(BLKAVR),
    CODE_NAME(BLKDEVSTD),
    CODE_NAME(BLK_CMK),
    CODE_NAME(BLK_CPK),
    CODE_NAME(BLK_ORL),
    CODE_NAME(BLK_ORL_2),
    CODE_NAME(BLK_ORB),

    CODE_NAME(BLK_ANDL),
    CODE_NAME(BLK_ANDL_2),
    CODE_NAME(BLK_ANDB),

    CODE_NAME(BLK_EQU_2),
    CODE_NAME(BLK_DIFF_2),

    CODE_NAME(ANDL),
    CODE_NAME(ANDB),
    CODE_NAME(ORL),
    CODE_NAME(ORB),
    CODE_NAME(XORL),
    CODE_NAME(XORB),

    CODE_NAME(DIFF),
    };
  for(uint i = 0; i < SIZE_A(codename); ++i)
    if(!_tcscmp(t, codename[i].name))
      return codename[i].code;
  return 0;
}
//------------------------------------------------------------------------
static
int getCodeFunctNoArg(LPCTSTR t)
{
  static struct
  {
    LPCTSTR name;
    int code;
  } codename [] = {
    CODE_NAME(NOW),
    CODE_NAME(DATE),
    CODE_NAME(TIME),
    CODE_NAME(YEAR),
    CODE_NAME(MONTH),
    CODE_NAME(DAY),

    CODE_NAME(HOUR),
    CODE_NAME(MINUTE),
    CODE_NAME(SEC),

    CODE_NAME(RAND),
    };
  for(uint i = 0; i < SIZE_A(codename); ++i)
    if(!_tcscmp(t, codename[i].name))
      return codename[i].code;
  return 0;
}
//------------------------------------------------------------------------
enum fCodeConst {
    fFIRST_CONST = 1,
    fPI = fFIRST_CONST,
    fE,
    fFSEC,
    fFMIN,
    fFHOUR,
    fNULL,
    };
//------------------------------------------------------------------------
static
int getCodeConst(LPCTSTR t)
{
  static struct
  {
    LPCTSTR name;
    int code;
  } codename [] = {
    CODE_NAME(PI),
    CODE_NAME(E),
    CODE_NAME(FSEC),
    CODE_NAME(FMIN),
    CODE_NAME(FHOUR),
    CODE_NAME(NULL),
    };
  for(uint i = 0; i < SIZE_A(codename); ++i)
    if(!_tcscmp(t, codename[i].name))
      return codename[i].code;
  return 0;
}
//------------------------------------------------------------------------
static
void getValConst(int code, P_Expr::exprVar& val)
{
  if(fNULL == code) {
    val.nullVal = true;
    val.iValue = 0;
    val.type = P_Expr::tvInt;
    return;
    }
  static P_Expr::exprVar values[] = {
    P_Expr::exprVar(M_PI),
    P_Expr::exprVar(M_E),
    P_Expr::exprVar(SECOND_TO_I64),
    P_Expr::exprVar(MINUTE_TO_I64),
    P_Expr::exprVar(HOUR_TO_I64)
    };
  code -= fFIRST_CONST;
  val = values[code];
}
//------------------------------------------------------------------------
static
bool hasMoreArgFunct(int code)
{
  if(code >= fFIRST_MORE_ARG && code < fMAX_FUNCT)
    return true;
  return false;
}
//------------------------------------------------------------------------
void P_Expression::getOffsStep(int& offs, int& step)
{
  if(_T('[') == Expr[currPos]) {
    ++currPos;
    // viene usata una variabile come offset
    if(VAR_CHAR == Expr[currPos]) {
      ++currPos;
      offs = makeOffsVar(_ttoi(Expr + currPos));
      // non si devono accettare gli altri dati
      while(Expr[currPos]) {
        if(_T(']') == Expr[currPos]) {
          ++currPos;
          return;
          }
        ++currPos;
        }
      }
    else
      offs = _ttoi(Expr + currPos);
    while(Expr[currPos]) {
      if(_T(']') == Expr[currPos]) {
        ++currPos;
        break;
        }
      if(_T(',') == Expr[currPos]) {
        ++currPos;
        step = _ttoi(Expr + currPos);
        }
      else
        ++currPos;
      }
    }
}
//------------------------------------------------------------------------
int P_Expression::getEndFunct()
{
  int bracket = 1;
  int end = currPos + 1;
  while(Expr[end] && bracket >  0) {
    if(_T('(') == Expr[end])
      ++bracket;
    else if(_T(')') == Expr[end])
      --bracket;
    ++end;
    }
  return end;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeConstValue()
{
  TCHAR t[50];
  uint len = 0;

  int typeData = 0; // 0 = int, 1 = float, 2 = hex
  if(_T('0') == Expr[currPos]) {
    typeData = 2;
    int old = currPos;
    TCHAR car = 0;
    while(Expr[currPos]) {
      car = _totupper((unsigned)Expr[currPos]);
      if(_T('H') == car) {
        ++currPos;
        break;
        }
      if(!_istdigit((unsigned)car) && (_T('A') > car || car > _T('F'))) {
        currPos = old;
        break;
        }
      t[len++] = Expr[currPos++];
      if(len >= SIZE_A(t) - 1)
        break;
      }
    if((!Expr[currPos] && _T('H') != car) || currPos == old) {
      currPos = old;
      typeData = 0;
      len = 0;
      }
    }
  if(!typeData) {
    while(_istdigit((unsigned)Expr[currPos]) || (Expr[currPos] == _T('.'))) {
      if(_T('.') == Expr[currPos])
        typeData = 1;
      t[len++] = Expr[currPos++];
      if(len >= SIZE_A(t) - 1)
        break;
      }
    }
  t[len] = 0;

  pTree nod = new Tree(allocOper(_T('@')));
  LPTSTR dummy;
  switch(typeData) {
    case 0:
      nod->Val.iValue = _tstoi64(t);
      nod->Val.type = P_Expr::tvInt;
      break;
    case 1:
      nod->Val.dValue = _tstof(t);
      nod->Val.type = P_Expr::tvReal;
      break;
    case 2:
      nod->Val.iValue = _tcstoi64(t, &dummy, 16);
      nod->Val.type = P_Expr::tvInt;
      break;
    }
  return nod;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeVarValue(uint code)
{
  TCHAR t[50];
  uint len = 0;
  ++currPos;
  while(_istdigit((unsigned)Expr[currPos])) {
    t[len++] = Expr[currPos++];
    if(len >= SIZE_A(t) - 1)
      break;
    }
  t[len] = 0;
  pTree nod = new Tree(allocOper(code));
  int ivar = _ttoi(t);
  int offs = 0;
  int step = 0;
  getOffsStep(offs, step);
  nod->Val.setVariable(ivar, offs, step);
  return nod;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeFunctValue()
{
  TCHAR t[MAX_LEN_NAME_VARS_FUNCT];
  uint len = 0;

  while(_istalnum((unsigned)Expr[currPos]) || _T('_') == Expr[currPos]) {
    t[len++] = _totupper((unsigned)Expr[currPos++]);
    if(len >= SIZE_A(t) - 1)
      break;
    }
  t[len] = 0;
  int code = getCodeConst(t);
  if(code) {
    pTree nod = new Tree(allocOper(_T('@')));
    getValConst(code, nod->Val);
    return nod;
    }
  code = getCodeFunctNoArg(t);
  if(code) {
    pTree nod = new Tree(allocOper(code));
    return nod;
    }
  code = getCodeFunct(t);
  int end = getEndFunct();
  pTree left = makeLevelHigh();
  if(!left && code)
    return 0;
  pTree more = 0;
  pTree right = 0;
  if(!code || hasMoreArgFunct(code)) {
    while(currPos < end && _T(',') == Expr[currPos]) {
      pTree tRight = makeLevelHigh();
      if(!tRight) {
        delete left;
        delete right;
        delete more;
        return 0;
        }
      if(more) {
        pTree t = more;
        while(t->more)
          t = t->more;
        t->more = tRight;
        }
      else if(right)
        more = tRight;
      else
        right = tRight;
      }
    }
  operatBase* op = code ? allocOper(code) : allocFunct(this, t);
  if(!op) {
    delete left;
    delete right;
    while(more) {
      pTree t = more;
      more = t->more;
      delete t;
      }
    return 0;
    }
  pTree nod = new Tree(op, left, right);
  nod->more = more;
  return nod;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::makeLevelLast()
{
  if(!Expr[currPos])
    return 0;

  // costante numerica
  if(_istdigit((unsigned)Expr[currPos]) || (Expr[currPos] == _T('.')))
    return makeConstValue();

  // variabile
  if(VAR_CHAR == Expr[currPos])
    return makeVarValue(fVAR);

  // variabile dentro la funzione
  if(VAR_FUNCT_CHAR == Expr[currPos])
    return makeVarValue(fFUNCTVAR);

  // funzione
  if(_istalpha((unsigned)Expr[currPos]))
    return makeFunctValue();
  return 0;
}
//------------------------------------------------------------------------
P_Expr::exprVar P_Expression::vexp(pTree a)
{
  P_Expr::exprVar v;
  Result = a->calc(this, v);
  return v;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::GetRadix()
{
  return Radix;
}
//------------------------------------------------------------------------
P_Expression::P_Expression(const P_Expression& other)
{
  *this = other;
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::CloneTree() const
{
  return clone(Radix);
}
//------------------------------------------------------------------------
P_Expression::pTree P_Expression::clone(pTree tree) const
{
  if(!tree)
    return 0;
  pTree cloneTree = new Tree;
  *cloneTree = *tree;
  cloneTree->left = clone(tree->left);
  cloneTree->right = clone(tree->right);
  cloneTree->more = clone(tree->more);
  return cloneTree;
}
//------------------------------------------------------------------------
const P_Expression& P_Expression::operator=(const P_Expression &expr)
{
  if(&expr != this) {
    delete []Expr;
    Expr = P_Expr::str_newdupStripSpace(expr.Expr);
    currPos = 0;
    Radix = expr.CloneTree();
    }
  return *this;
}
//------------------------------------------------------------------------
