//----------------- p_operation.cpp --------------------------------------
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include "p_operation.h"
double d_PRECISION = 0.00001;
//------------------------------------------------------------------------
P_Expr::errCode operat::calcWithCheckNullVal(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  P_Expr::errCode err = P_Expr::NO_ERR;
  if(v1.nullVal || v2.nullVal) {
    err = P_Expr::ONE_HAS_NO_ACTION;
    if(v1.nullVal) {
      if(v2.nullVal) {
        result.nullVal = true;
        return P_Expr::NO_ACTION;
        }
      result = v2;
      }
    else //if(v2.nullVal)
      result = v1;
    }
  else {
    if(P_Expr::tvReal == v1.type || P_Expr::tvReal == v2.type) {
      result.type = P_Expr::tvReal;
      err = calcDouble(result, v1, v2);
      }
    else {
      result.type = P_Expr::tvInt;
      err = calcI64(result, v1, v2);
      }
    }
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode operat::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::exprVar v1;
  if(sender->left) {
    P_Expr::errCode err = sender->left->calc(owner, v1);
    if(err > P_Expr::NO_ERR)
      return err;
    }
  P_Expr::exprVar v2;
  if(sender->right) {
    P_Expr::errCode err = sender->right->calc(owner, v2);
    if(err > P_Expr::NO_ERR)
      return err;
    }

  P_Expr::errCode err = calcWithCheckNullVal(result, v1, v2);
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode operatMore::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::exprVar v1;
  P_Expr::exprVar v2;
  P_Expr::errCode err = baseClass::calc(owner, result, sender);
  switch(err) {
    case P_Expr::NO_ERR:
      count = 2;
      break;
    case P_Expr::ONE_HAS_NO_ACTION:
      count = 1;
      break;
    case P_Expr::NO_ACTION:
      count = 0;
      break;
    default:
      return err;
    }

  pTree more = sender->more;
  while(more && P_Expr::NO_ERR >= err) {
    if((err = more->calc(owner, v2)) > P_Expr::NO_ERR)
      return err;
    v1 = result;

    bool add = P_Expr::NO_ACTION != err;
    err = calcWithCheckNullVal(result, v1, v2);
    if(P_Expr::NO_ACTION != err && add)
      ++count;
    more = more->more;
    }
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode opMoreShortCircuit::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::errCode err;
  P_Expr::exprVar v1;
  if(sender->left) {
    err = sender->left->calc(owner, v1);
    if(err > P_Expr::NO_ERR)
      return err;
    }
  P_Expr::exprVar v2;
  if(sender->right) {
    err = sender->right->calc(owner, v2);
    if(err > P_Expr::NO_ERR)
      return err;
    }

  err = calcWithCheckNullVal(result, v1, v2);
  if(err > P_Expr::NO_ERR)
    return err;

  pTree more = sender->more;
  while(more && P_Expr::NO_ERR >= err) {
    if(P_Expr::NO_ACTION != err && onShortCircuit(result))
      break;
    if((err = more->calc(owner, v2)) > P_Expr::NO_ERR)
      return err;

    err = calcWithCheckNullVal(result, v1, v2);
    more = more->more;
    }
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode opMoreShortCircuitSingle::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::errCode err;
  if(sender->left) {
    err = sender->left->calc(owner, result);
    if(err > P_Expr::NO_ERR || onShortCircuit(result))
      return err;
    }
  if(sender->right) {
    err = sender->right->calc(owner, result);
    if(err > P_Expr::NO_ERR || onShortCircuit(result))
      return err;
    }

  pTree more = sender->more;
  while(more) {
    err = more->calc(owner, result);
    if(err > P_Expr::NO_ERR || onShortCircuit(result))
      break;
    more = more->more;
    }
  return err;
}
//------------------------------------------------------------------------
bool operOR_L::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return ret;
}
//------------------------------------------------------------------------
bool operAND_L::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return !ret;
}
//------------------------------------------------------------------------
bool operEQU::onShortCircuit(P_Expr::exprVar& result)
{
 return !result.getInt();
}
//------------------------------------------------------------------------
P_Expr::errCode operIF::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::exprVar v;
  P_Expr::errCode err = sender->left->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;
  if(v.getInt()) {
    if(!sender->right)
      return P_Expr::INVALID_EXPRESSION;
    err = sender->right->calc(owner, result);
    }
  else {
    if(!sender->more)
      return P_Expr::INVALID_EXPRESSION;
    err = sender->more->calc(owner, result);
    }
  return err;
}
//------------------------------------------------------------------------
extern void setIdVarG(int v);
extern void setIdVarGFunct(int v);
//------------------------------------------------------------------------
P_Expr::errCode operVariable::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  int id;
  int offset;
  int step;
  sender->Val.getVariable(id, offset, step);
  offset += step * owner->getCycle();
  bool exist = true;
  result = owner->getVariable(id, offset, exist);
  if(!exist) {
    setIdVarG(id);
    return P_Expr::UNDEFINED_VARIABLE;
    }
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operFunctVariable::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_CustFunct* F = owner->getCurrFunct();
  if(!F)
    return P_Expr::INVALID_EXPRESSION;
  int id;
  int offset;
  int step;
  sender->Val.getVariable(id, offset, step);
  offset += step * owner->getCycle();
  bool exist = true;
  result = F->getVariable(id, offset, exist);
  if(!exist) {
    setIdVarGFunct(id);
    return P_Expr::UNDEFINED_VARIABLE;
    }
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operatCustomFunct::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  uint nArg = Funct->getNumArg();
  PVect<P_Expr::exprVar> v;
  v.setDim(nArg);
  if(nArg > 0) {
    if(sender->left) {
      P_Expr::errCode err = sender->left->calc(owner, v[0]);
      if(err > P_Expr::NO_ERR)
        return err;
      }
    }
  if(nArg > 1) {
    if(sender->right) {
      P_Expr::errCode err = sender->right->calc(owner, v[1]);
      if(err > P_Expr::NO_ERR)
        return err;
      }
    }
  if(nArg > 2) {
    pTree more = sender->more;
    P_Expr::errCode err = P_Expr::NO_ERR;
    for(uint i = 2; i < nArg; ++i) {
      if(!more)
        break;
      P_Expr::errCode err =  more->calc(owner, v[i]);
      if(err > P_Expr::NO_ERR)
        return err;
      more = more->more;
      }
    }
  if(!owner->pushFunct(Funct))
    return P_Expr::TO_MANY_CALL;
  P_Expr::errCode err = Funct->execute(owner, result, v);
  if(err > P_Expr::NO_ERR)
    return P_Expr::ERR_ON_FUNCT;
  owner->popFunct();
  return err;
//  return err > P_Expr::NO_ERR ? P_Expr::ERR_ON_FUNCT : err;
}
//------------------------------------------------------------------------
P_Expr::errCode operAVR::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::errCode err = baseClass::calc(owner, result, sender);
  if(err > P_Expr::NO_ERR)
    return err;
  if(!count)
    return P_Expr::DIVISION_BY_0;
  double v = result.getReal();
  v /= count;
  result.type = P_Expr::tvReal;
  result.dValue = v;
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
P_Expr::errCode operNOW::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  SYSTEMTIME st;
  FILETIME ft;
  GetLocalTime(&st);
  SystemTimeToFileTime(&st, &ft);
  result.iValue = MK_I64(ft);
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operDATE::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  baseClass::calcI64(result, v1, v2);
  result.iValue /= DAY_TO_I64;
  result.iValue *= DAY_TO_I64;
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operTIME::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  baseClass::calcI64(result, v1, v2);
  result.iValue %= DAY_TO_I64;
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operSWAB2::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  __int64 v = v1.getInt();
  LPSTR lpw = (LPSTR)&v;
  _swab(lpw, lpw, sizeof(__int64) / sizeof(WORD));
  result.iValue = v;
  return P_Expr::NO_ERR;
}
//-----------------------------------------------------
// azzera sempre dopo lo shift
// non dovrebbe essere necessario se valori unsigned
  #define SWAB__S(v) ( (((v) >> 24) & 0xff)     | \
                      (((v) << 24) & 0xff000000)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
// se valori unsigned
  #define SWAB__(v) ( ((v) >> 24)     | \
                      ((v) << 24)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
//-----------------------------------------------------
static void mySwab(void *from, void *to)
{
  DWORD *t = reinterpret_cast<DWORD*>(to);
  DWORD *s = reinterpret_cast<DWORD*>(from);
  t[0] = SWAB__S(s[0]);
  t[1] = SWAB__S(s[1]);
}
//------------------------------------------------------------------------
P_Expr::errCode operSWAB4::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  __int64 v = v1.getInt();
  mySwab(&v, &v);
  result.iValue = v;
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
static __int64 calcSwab8(__int64 v)
{
  LPBYTE lpb = (LPBYTE)&v;
  for(uint i = 0, j = sizeof(__int64) - 1; i < sizeof(__int64) / 2; ++i, --j) {
    BYTE t = lpb[i];
    lpb[i] = lpb[j];
    lpb[j] = t;
    }
  return v;
}
//------------------------------------------------------------------------
P_Expr::errCode operSWAB8::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = calcSwab8(v1.getInt());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
static __int64 calcToBCD_BE(__int64 v)
{
  __int64 res = 0;
  while(true) {
    __int64 t = v % 100;
    t = t % 10 | (t / 10 << 4);
    res |= t;
    v /= 100;
    if(!v)
      break;
    res <<= 8;
    }
  return res;
}
//------------------------------------------------------------------------
static __int64 calcFromBCD_BE(__int64 v)
{
  __int64 res = 0;
  while(v) {
    res *= 100;
    __int64 t = v & 0xff;
    t = (t & 0xf) + (t >> 4) * 10;
    res += t;
    v >>= 8;
    }
  return res;
}
//------------------------------------------------------------------------
static __int64 calcToBCD_LE(__int64 v)
{
  if(!v)
    return 0;
  __int64 t = v % 100;
  t = t % 10 | (t / 10 << 4);
  t |= calcToBCD_LE(v / 100) << 8;
  return t;
}
//------------------------------------------------------------------------
static __int64 calcFromBCD_LE(__int64 v)
{
  if(!v)
    return 0;
  __int64 t = v & 0xff;
  t = (t & 0xf) + (t >> 4) * 10;
  t += calcFromBCD_LE(v >> 8) * 100;
  return t;
}
//------------------------------------------------------------------------
P_Expr::errCode operTOBCD_LE::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = calcToBCD_LE(v1.getInt());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operFROMBCD_LE::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = calcFromBCD_LE(v1.getInt());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operTOBCD_BE::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = calcToBCD_BE(v1.getInt());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operFROMBCD_BE::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = calcFromBCD_BE(v1.getInt());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operRAND::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = rand();
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operRAND::calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.dValue = rand();
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operRANDMINMAX::calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.iValue = ROUND_REAL_TO_i64((((double) rand() / (double) RAND_MAX) * (v2.getReal() - v1.getReal()) + v1.getReal()));
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operRANDMINMAX::calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
{
  result.dValue = (((double) rand() / (double) RAND_MAX) * (v2.getReal() - v1.getReal()) + v1.getReal());
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
P_Expr::errCode operBlock::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  if(!sender->right || !sender->more)
    return P_Expr::INVALID_EXPRESSION;

  int id;
  int offset;
  int step;
  sender->left->Val.getVariable(id, offset, step);
  offset += step * owner->getCycle(); // per compatibilità, ma non sono ammessi cicli
  bool exist = true;
  result = owner->getVariable(id, offset, exist);
  if(!exist)
    return P_Expr::UNDEFINED_VARIABLE;

  P_Expr::exprVar v;
  P_Expr::errCode err = sender->right->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  count = (int)v.getInt();
  if(!count)
    count = 1;

  err = sender->more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  uint blkStep = (int)v.getInt();
  if(!blkStep)
    blkStep = 1;

  for(int i = 1; i < count; ++i) {
    // la variabile result (output) non dovrebbe essere toccata dalla funzione chiamata
    // ma le viene solo assegnato il valore quindi possiamo usarla sia in input che in output
    P_Expr::exprVar v = owner->getVariable(id, offset + i * blkStep, exist);
    err = calcWithCheckNullVal(result, result, v);
    if(err > P_Expr::NO_ERR || onShortCircuit(result))
      break;
    }
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode operBlockAVR::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  P_Expr::errCode err = baseClass::calc(owner, result, sender);
  if(err > P_Expr::NO_ERR)
    return err;
  if(!count)
    return P_Expr::DIVISION_BY_0;
  double v = result.getReal();
  v /= count;
  result.type = P_Expr::tvReal;
  result.dValue = v;
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operBlockDevStd::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  if(!sender->right || !sender->more)
    return P_Expr::INVALID_EXPRESSION;

  int id;
  int offset;
  int step;
  sender->left->Val.getVariable(id, offset, step);
  offset += step * owner->getCycle(); // per compatibilità, ma non sono ammessi cicli
  bool exist = true;
  result = owner->getVariable(id, offset, exist);
  if(!exist)
    return P_Expr::UNDEFINED_VARIABLE;

  P_Expr::exprVar v;
  P_Expr::errCode err = sender->right->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  count = (int)v.getInt();
  if(!count)
    count = 1;

  err = sender->more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  uint blkStep = (int)v.getInt();
  if(!blkStep)
    blkStep = 1;

  double s1 = result.getReal();
  s1 *= s1;
  for(int i = 1; i < count; ++i) {
    P_Expr::exprVar v = owner->getVariable(id, offset + i * blkStep, exist);
    err = calcWithCheckNullVal(result, result, v);
    double t = v.getReal();
    s1 += t * t;
    if(err > P_Expr::NO_ERR)
      return err;
    }

  // media dei quadrati
  s1 /= count;
  double t = result.getReal() / count;
  Avr = t;
  // quadrato della media
  t *= t;
  s1 -= t;
  result.dValue = sqrt(s1);
  result.type = P_Expr::tvReal;

  return err;
}
//------------------------------------------------------------------------
static P_Expr::errCode getTolerance(P_Expression* owner, P_Expr::node* sender, double& tollP, double& tollM)
{
  if(!sender->more->more || !sender->more->more->more)
    return P_Expr::INVALID_EXPRESSION;
  P_Expr::exprVar v;
  P_Expr::errCode err = sender->more->more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;
  tollP = v.getReal();
  err = sender->more->more->more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;
  tollM = v.getReal();
  return P_Expr::NO_ERR;
}
//------------------------------------------------------------------------
P_Expr::errCode operBlockCMK::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  double tollP;
  double tollM;
  P_Expr::errCode err = getTolerance(owner, sender, tollP, tollM);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  err = baseClass::calc(owner, result, sender);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  result.dValue = (tollP - tollM) / (6 * result.getReal());
  result.type = P_Expr::tvReal;
  return err;
}
//------------------------------------------------------------------------
P_Expr::errCode operBlockCPK::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  double tollP;
  double tollM;
  P_Expr::errCode err = getTolerance(owner, sender, tollP, tollM);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  err = baseClass::calc(owner, result, sender);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  result.dValue = min(tollP - Avr, Avr - tollM) / (3 * result.getReal());
  result.type = P_Expr::tvReal;
  return err;
}
//------------------------------------------------------------------------
bool operBlockORL::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return ret;
}
//------------------------------------------------------------------------
bool operBlockANDL::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return !ret;
}
//------------------------------------------------------------------------
bool operBlockORL2::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return ret;
}
//------------------------------------------------------------------------
bool operBlockANDL2::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
  result = P_Expr::exprVar(__int64(ret));
  return !ret;
}
//------------------------------------------------------------------------
bool operBlockEQU_DIFF::onShortCircuit(P_Expr::exprVar& result)
{
  bool ret = toBool(result.getInt());
   result = P_Expr::exprVar(__int64(ret));
  if(ret)
    return false;

  onShort = true;
  return true;
}
//------------------------------------------------------------------------
P_Expr::errCode operBlock2::calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
{
  // fz($var1, step1, $var2, step2, ndata)
  if(!sender->right || !sender->more || !sender->more->more || !sender->more->more->more)
    return P_Expr::INVALID_EXPRESSION;

  int id1;
  int offset;
  int step;
  sender->left->Val.getVariable(id1, offset, step);
  bool exist = true;
  result = owner->getVariable(id1, 0, exist);
  if(!exist)
    return P_Expr::UNDEFINED_VARIABLE;

  P_Expr::exprVar v;
  P_Expr::errCode err = sender->right->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  uint blkStep1 = (int)v.getInt();
  if(!blkStep1)
    blkStep1 = 1;

  opNode* more = sender->more;
  int id2;
  more->Val.getVariable(id2, offset, step);
  v = owner->getVariable(id2, 0, exist);
  if(!exist)
    return P_Expr::UNDEFINED_VARIABLE;
  // il primo calcolo viene fatto prima del ciclo così si controlla l'esistenza delle variabili
  err = calcWithCheckNullVal(result, result, v);
  // se c'è errore oppure siamo già in corto circuito, inutile continuare
  if(err > P_Expr::NO_ERR || onShortCircuit(result))
    return err;

  more = more->more;
  err = more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  uint blkStep2 = (int)v.getInt();
  if(!blkStep2)
    blkStep2 = 1;

  more = more->more;
  err = more->calc(owner, v);
  if(err > P_Expr::NO_ERR || P_Expr::NO_ACTION == err)
    return err;

  count = (int)v.getInt();
  if(!count)
    count = 1;

  for(int i = 1; i < count; ++i) {
    P_Expr::exprVar v1 = owner->getVariable(id1, i * blkStep1, exist);
    P_Expr::exprVar v2 = owner->getVariable(id2, i * blkStep2, exist);
    err = calcWithCheckNullVal(result, v1, v2);
    if(err > P_Expr::NO_ERR || onShortCircuit(result))
      break;
    }
  return err;
}
//------------------------------------------------------------------------
operatBase* allocOper(unsigned car)
{
  switch(car) {
    case _T('+'):
      return new operAdd;
    case _T('-'):
      return new operSub;
    case _T('*'):
      return new operMul;
    case _T('/'):
      return new operDiv;
    case _T('%'):
      return new operMod;

    case _T('|'):
      return new operOR;
    case _T('&'):
      return new operAND;
    case _T('^'):
      return new operXOR;

    case _T('@'):
      return new operConst;
    case _T('>'):
      return new operGREAT;
    case _T('<'):
      return new operLESS;

    case fVAR:
      return new operVariable;
    case fFUNCTVAR:
      return new operFunctVariable;

    case fNOW:
      return new operNOW;
    case fDATE:
      return new operDATE;
    case fTIME:
      return new operTIME;
    case fYEAR:
      return new operYEAR;
    case fMONTH:
      return new operMONTH;
    case fDAY:
      return new operDAY;
    case fHOUR:
      return new operHOUR;
    case fMINUTE:
      return new operMINUTE;
    case fSEC:
      return new operSEC;

    case fNEG:
      return new operNEG;
    case fNOTL:
    case _T('!'):
      return new operNOTL;
    case fNOTB:
    case _T('~'):
      return new operNOTB;
    case fPOW:
      return new operPOW;
    case oSHL:
    case fSHL:
      return new operSHL;
    case oSHR:
    case fSHR:
      return new operSHR;
    case fIF:
      return new operIF;
    case fSUM:
      return new operSUM;
    case fAVR:
      return new operAVR;

    case fMIN:
      return new operMIN;
    case fMAX:
      return new operMAX;
    case oEQU:
    case fEQU:
      return new operEQU;
    case fRAD:
      return new operRAD;
    case fGRAD:
      return new operGRAD;
    case fABS:
      return new operABS;
    case fSIN:
      return new operSIN;
    case fCOS:
      return new operCOS;
    case fEXP:
      return new operEXP;
    case fSQRT:
      return new operSQRT;
    case fLOG:
      return new operLOG;
    case fTG:
      return new operTG;
    case fCTG:
      return new operCTG;
    case fASIN:
      return new operASIN;
    case fACOS:
      return new operACOS;
    case fATG:
      return new operATG;

    case fBLKSUM:
      return new operBlockSum;
    case fBLKAVR:
      return new operBlockAVR;
    case fBLKDEVSTD:
      return new operBlockDevStd;

    case fBLK_CMK:
      return new operBlockCMK;
    case fBLK_CPK:
      return new operBlockCPK;

    case fBLK_ORL:
      return new operBlockORL;
    case fBLK_ORL_2:
      return new operBlockORL2;
    case fBLK_ORB:
      return new operBlockORB;

    case fBLK_ANDL:
      return new operBlockANDL;
    case fBLK_ANDL_2:
      return new operBlockANDL2;
    case fBLK_ANDB:
      return new operBlockANDB;

    case fBLK_EQU_2:
      return new operBlockEQU2;
    case fBLK_DIFF_2:
      return new operBlockDIFF2;

    case oDIFF:
    case fDIFF:
      return new operNotEQU;
    case oANDL:
    case fANDL:
      return new operAND_L;
    case fANDB:
      return new operAND_B;
    case oORL:
    case fORL:
      return new operOR_L;
    case fORB:
      return new operOR_B;
    case oXORL:
    case fXORL:
      return new operXOR_L;
    case fXORB:
      return new operXOR_B;

    case fSWAB2:
      return new operSWAB2;
    case fSWAB4:
      return new operSWAB4;
    case fSWAB8:
      return new operSWAB8;
    case fTOBCD_LE:
      return new operTOBCD_LE;
    case fFROMBCD_LE:
      return new operFROMBCD_LE;
    case fTOBCD_BE:
      return new operTOBCD_BE;
    case fFROMBCD_BE:
      return new operFROMBCD_BE;

    case fRAND:
      return new operRAND;
    case fRANDMINMAX:
      return new operRANDMINMAX;

    case oLESS_OR_EQU:
      return new operLESS_OR_EQU;
    case oGREAT_OR_EQU:
      return new operGREAT_OR_EQU;

    default:
      return 0;
    }
}
//------------------------------------------------------------------------
operatBase* allocFunct(P_Expression* owner, LPCTSTR name)
{
  // ricerca nome nelle funzioni allocate, se non esiste viene creato un oggetto vuoto
  // probabilmente è una funzione richiamata all'interno di un'altra e non ancora parsata
  P_CustFunct* f = owner->getOwnerFunct(name);
  if(!f)
    return 0;
  return new operatCustomFunct(f);
}
