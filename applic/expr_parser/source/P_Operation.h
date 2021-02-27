//----------------- p_operation.h ----------------------------------------
#ifndef P_OPERATION_H_
#define P_OPERATION_H_
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#define  _USE_MATH_DEFINES
#include "math.h"
#include "p_express.h"
//------------------------------------------------------------------------
extern operatBase* allocOper(unsigned car);
extern operatBase* allocFunct(P_Expression* owner, LPCTSTR name);
//------------------------------------------------------------------------
enum fCodeName {
    // variabile normale e dentro la funzione
    fVAR = 256,
    fFUNCTVAR,
    // senza arg
    fNOW,
    fDATE,
    fTIME,
    fYEAR,
    fMONTH,
    fDAY,

    fHOUR,
    fMINUTE,
    fSEC,

    fRAND,

    // unarie
    fABS,
    fNEG,
    fNOTL,
    fNOTB,
    fSIN,
    fCOS,
    fEXP,
    fSQRT,
    fLOG,
    fTG,
    fCTG,
    fASIN,
    fACOS,
    fATG,
    fRAD,
    fGRAD,

    fSWAB2,
    fSWAB4,
    fSWAB8,
    fTOBCD_LE,
    fFROMBCD_LE,
    fTOBCD_BE,
    fFROMBCD_BE,

    fFIRST_MORE_ARG,
    // binarie
    fPOW = fFIRST_MORE_ARG,
    fSHL,
    fSHR,
    fDIFF,

    fRANDMINMAX,

    // n_arie
    fIF,

    fMIN,
    fMAX,
    fSUM,
    fAVR,
    fEQU,

    fANDL,
    fANDB,
    fORL,
    fORB,
    fXORL,
    fXORB,

    fBLKSUM,
    fBLKAVR,
    fBLKDEVSTD,

    fBLK_CMK,
    fBLK_CPK,

    fBLK_ORL,
    fBLK_ORL_2,
    fBLK_ORB,

    fBLK_ANDL,
    fBLK_ANDL_2,
    fBLK_ANDB,

    fBLK_EQU_2,
    fBLK_DIFF_2,

    fMAX_FUNCT,

    oANDL,
    oORL,
    oXORL,
    oLESS_OR_EQU,
    oGREAT_OR_EQU,
    oEQU,
    oDIFF,
    oSHL,
    oSHR,
    };
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#ifndef MK_I64
//------------------------------------------------------------------------
#define MK_I64(ft)  ( (reinterpret_cast<LARGE_INTEGER*> (&ft) )->QuadPart)
#define I64_TO_FT(val) i64ToFT(val)
//--------------------------------------------------------------------
inline FILETIME  i64ToFT(__int64 val)
{
  FILETIME ft;
  MK_I64(ft) = val;
  return ft;
}
//------------------------------------------------------------------------
#define SECOND_TO_I64  10000000i64
#define MINUTE_TO_I64 (SECOND_TO_I64 * 60)
#define HOUR_TO_I64 (MINUTE_TO_I64 * 60)
#define MK_I64_HOUR(a) hourToI64(a)
#define MK_I64_MINUTE(a) minToI64(a)
//--------------------------------------------------------------------
inline __int64 minToI64(__int64 v)
{
  return MINUTE_TO_I64 * v;
}
//--------------------------------------------------------------------
inline __int64 hourToI64(__int64 v)
{
  __int64 tmp = MK_I64_MINUTE(v);
  return tmp * 60;
}
//------------------------------------------------------------------------
#endif
#define DAY_TO_I64 (HOUR_TO_I64 * 24)
//------------------------------------------------------------------------
class operat : public operatBase
{
  public:
    operat() {}
    virtual ~operat() {}
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
        { return P_Expr::NO_ERR; }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
        { return P_Expr::NO_ERR; }
    virtual P_Expr::errCode calcWithCheckNullVal(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
};
//------------------------------------------------------------------------
class operatMore : public operat
{
  public:
    operatMore() : count(0) {}
    virtual ~operatMore() {}

    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    int count;
  private:
    typedef operat baseClass;
};
//------------------------------------------------------------------------
class opMoreShortCircuit : public operat
{
  public:
    opMoreShortCircuit() {}
    virtual ~opMoreShortCircuit() {}

    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    virtual bool onShortCircuit(P_Expr::exprVar& result) = 0;
  private:
    typedef operat baseClass;
};
//------------------------------------------------------------------------
class opMoreShortCircuitSingle : public operat
{
  public:
    opMoreShortCircuitSingle() {}
    virtual ~opMoreShortCircuitSingle() {}

    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    virtual bool onShortCircuit(P_Expr::exprVar& result) = 0;
  private:
    typedef operat baseClass;
};
//------------------------------------------------------------------------
template< class BaseT, class DerivedT >
class clonable_impl : public BaseT
{
  public:
    virtual BaseT* makeObj() {  return new DerivedT;   }
 };
//------------------------------------------------------------------------
class operIF : public clonable_impl<operatBase, operIF>
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operConst : public clonable_impl<operat, operConst>
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender)
    {
      result = sender->Val;
      if(result.nullVal)
        return P_Expr::NO_ACTION;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operVariable : public clonable_impl<operat, operVariable>
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operFunctVariable : public clonable_impl<operat, operFunctVariable>
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operatCustomFunct : public operat
{
  public:
    operatCustomFunct(P_CustFunct* f) : Funct(f) {}
    virtual operat* makeObj() {  return new operatCustomFunct(Funct);   }
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    P_CustFunct* Funct;
};
//------------------------------------------------------------------------
class operAdd : public clonable_impl<operat, operAdd>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() + v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() + v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operSub : public clonable_impl<operat, operSub>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() - v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() - v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operMul : public clonable_impl<operat, operMul>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() * v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() * v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operDiv : public clonable_impl<operat, operDiv>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(!v2.iValue)
        return P_Expr::DIVISION_BY_0;

      result.iValue = v1.getInt() / v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(!v2.dValue)
        return P_Expr::DIVISION_BY_0;
      result.dValue = v1.getReal() / v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operMod : public clonable_impl<operat, operMod>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(!v2.iValue)
        return P_Expr::DIVISION_BY_0;

      result.iValue = v1.getInt() % v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(!v2.dValue)
        return P_Expr::DIVISION_BY_0;
      result.dValue = fmod(v1.getReal(), v2.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class operNotEQU : public clonable_impl<operat, operNotEQU>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() != v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = fabs(v1.getReal() - v2.getReal()) >= d_PRECISION;
      result.type = P_Expr::tvInt;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operOR : public clonable_impl<operat, operOR>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() | v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operXOR : public clonable_impl<operat, operXOR>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() ^ v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operAND : public clonable_impl<operat, operAND>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() & v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operOR_B : public clonable_impl<operatMore, operOR_B>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() | v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operXOR_B : public clonable_impl<operatMore, operXOR_B>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() ^ v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operAND_B : public clonable_impl<operatMore, operAND_B>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() & v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operOR_L : public clonable_impl<opMoreShortCircuitSingle, operOR_L>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() || v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
class operXOR_L : public clonable_impl<operatMore, operXOR_L>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = toBool(v1.getInt()) ^ toBool(v2.getInt());
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operAND_L : public clonable_impl<opMoreShortCircuitSingle, operAND_L>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() && v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return calcI64(result, v1, v2);
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
//#define PI 3.14159265358979
//------------------------------------------------------------------------
class operRAD : public clonable_impl<operat, operRAD>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getInt() / 360.0 * 2 * M_PI;
      result.type = P_Expr::tvReal;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() / 360.0 * 2 * M_PI;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operGRAD : public clonable_impl<operat, operGRAD>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() * 360.0 / (2 * M_PI);
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operSIN : public clonable_impl<operat, operSIN>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = sin(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operCOS : public clonable_impl<operat, operCOS>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = cos(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operEXP : public clonable_impl<operat, operEXP>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = exp(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operSQRT : public clonable_impl<operat, operSQRT>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(v1.getReal() < 0)
        return P_Expr::INVALID_DOMAIN;
      result.iValue = ROUND_REAL_TO_i64(sqrt(v1.getReal()));
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(v1.getReal() < 0)
        return P_Expr::INVALID_DOMAIN;
      result.dValue = sqrt(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operLOG : public clonable_impl<operat, operLOG>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(v1.getInt() < 0)
        return P_Expr::INVALID_DOMAIN;
      result.dValue = log(v1.getReal());
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      if(v1.getReal() < 0)
        return P_Expr::INVALID_DOMAIN;
      result.dValue = log(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operTG : public clonable_impl<operat, operTG>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = tan(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operCTG : public clonable_impl<operat, operCTG>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = 1 / tan(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operASIN : public clonable_impl<operat, operASIN>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = asin(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operACOS : public clonable_impl<operat, operACOS>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = acos(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operATG : public clonable_impl<operat, operATG>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = atan(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operABS : public clonable_impl<operat, operABS>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt();
      if(result.iValue < 0)
        result.iValue = -result.iValue;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = fabs(v1.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operLESS : public clonable_impl<operat, operLESS>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() < v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = (v2.getReal() - v1.getReal()) > d_PRECISION;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operLESS_OR_EQU : public clonable_impl<operat, operLESS_OR_EQU>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() <= v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = (v2.getReal() - v1.getReal()) >= d_PRECISION;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operGREAT : public clonable_impl<operat, operGREAT>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() > v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = (v1.getReal() - v2.getReal()) > d_PRECISION;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operGREAT_OR_EQU : public clonable_impl<operat, operGREAT_OR_EQU>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() >= v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = (v1.getReal() - v2.getReal()) >= d_PRECISION;
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operNEG : public clonable_impl<operat, operNEG>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = -v1.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = -v1.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operNOTL : public clonable_impl<operat, operNOTL>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = !v1.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = !v1.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operNOTB : public clonable_impl<operat, operNOTB>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = ~v1.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      // il not a bit viene sempre fatto sul valore intero
      result.type = P_Expr::tvInt;
      result.iValue = ~v1.getInt();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operPOW : public clonable_impl<operat, operPOW>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = ROUND_REAL_TO_i64(pow(v1.getReal(), v2.getReal()));
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = pow(v1.getReal(), v2.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operSHL : public clonable_impl<operat, operSHL>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() << v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      // le operazioni sui bit vengono sempre fatte sul valore intero
      result.iValue = v1.getInt() << v2.getInt();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operSHR : public clonable_impl<operat, operSHR>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() >> v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      // le operazioni sui bit vengono sempre fatte sul valore intero
      result.iValue = v1.getInt() >> v2.getInt();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operMIN : public clonable_impl<operatMore, operMIN>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = min(v1.getInt(), v2.getInt());
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = min(v1.getReal(), v2.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operMAX : public clonable_impl<operatMore, operMAX>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = max(v1.getInt(), v2.getInt());
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = max(v1.getReal(), v2.getReal());
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operEQU : public clonable_impl<opMoreShortCircuit, operEQU>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() == v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = fabs(v2.getReal() - v1.getReal()) <= d_PRECISION;
      result.type = P_Expr::tvInt;
//      result.dValue = v1.getReal() == v2.getReal();
      return P_Expr::NO_ERR;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
class operSUM : public clonable_impl<operatMore, operSUM>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() + v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() + v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operAVR : public  clonable_impl<operSUM, operAVR>
{
  public:
  private:
    typedef operSUM baseClass;
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operYEAR : public clonable_impl<operat, operYEAR>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wYear;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operMONTH : public clonable_impl<operat, operMONTH>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wMonth;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operDAY : public clonable_impl<operat, operDAY>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wDay;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operHOUR : public clonable_impl<operat, operHOUR>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wHour;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operMINUTE : public clonable_impl<operat, operMINUTE>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wMinute;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operSEC : public clonable_impl<operat, operSEC>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      result.iValue = st.wSecond;
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operNOW : public clonable_impl<operat, operNOW>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operDATE : public clonable_impl<operNOW, operDATE>
{
  public:
  private:
    typedef operNOW baseClass;
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operTIME : public clonable_impl<operNOW, operTIME>
{
  public:
  private:
    typedef operNOW baseClass;
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      return calcI64(result, v1, v2);
    }
};
//------------------------------------------------------------------------
class operSWAB2 : public clonable_impl<operat, operSWAB2>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operSWAB4 : public clonable_impl<operat, operSWAB4>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operSWAB8 : public clonable_impl<operat, operSWAB8>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operTOBCD_LE : public clonable_impl<operat, operTOBCD_LE>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operFROMBCD_LE : public clonable_impl<operat, operTOBCD_LE>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operTOBCD_BE : public clonable_impl<operat, operTOBCD_BE>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operFROMBCD_BE : public clonable_impl<operat, operTOBCD_BE>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operRAND : public clonable_impl<operat, operRAND>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
};
//------------------------------------------------------------------------
class operRANDMINMAX : public clonable_impl<operat, operRANDMINMAX>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
};
//------------------------------------------------------------------------
class operBlock : public operatMore
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    virtual bool onShortCircuit(P_Expr::exprVar& result) { return false; }
  private:
    typedef operatMore baseClass;
};
//------------------------------------------------------------------------
class operBlockSum : public clonable_impl<operBlock, operBlockSum>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() + v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.dValue = v1.getReal() + v2.getReal();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operBlockORB : public clonable_impl<operBlock, operBlockORB>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() | v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
};
//------------------------------------------------------------------------
class operBlockANDB : public clonable_impl<operBlock, operBlockANDB>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() & v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result) { return result.getInt() == 0; }
};
//------------------------------------------------------------------------
class operBlockORL : public clonable_impl<operBlock, operBlockORL>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() || v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
class operBlockANDL : public clonable_impl<operBlock, operBlockANDL>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() && v2.getInt();
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);

};
//------------------------------------------------------------------------
class operBlockAVR : public clonable_impl<operBlockSum, operBlockAVR>
{
  public:
  private:
    typedef operBlockSum baseClass;
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operBlockDevStd : public clonable_impl<operBlockSum, operBlockDevStd>
{
  public:
  private:
    typedef operBlockSum baseClass;
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    // per altre operazioni che usano SD e AVR
    double Avr;
};
//------------------------------------------------------------------------
class operBlockCMK : public clonable_impl<operBlockDevStd, operBlockCMK>
{
  public:
  private:
    typedef operBlockDevStd baseClass;
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operBlockCPK : public clonable_impl<operBlockDevStd, operBlockCPK>
{
  public:
  private:
    typedef operBlockDevStd baseClass;
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
};
//------------------------------------------------------------------------
class operBlock2 : public operatMore
{
  public:
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender);
  protected:
    virtual bool onShortCircuit(P_Expr::exprVar& result) = 0;
  private:
    typedef operatMore baseClass;
};
//------------------------------------------------------------------------
class operBlockORL2 : public clonable_impl<operBlock2, operBlockORL2>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = !((v1.getInt() != 0) ^ (v2.getInt() != 0));
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = !((v1.getInt() != 0) ^ (v2.getInt() != 0));
      return P_Expr::NO_ERR;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
class operBlockANDL2 : public clonable_impl<operBlock2, operBlockANDL2>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = !((v1.getInt() != 0) ^ (v2.getInt() != 0));
      return P_Expr::NO_ERR;
    }
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.type = P_Expr::tvInt;
      result.iValue = !((v1.getInt() != 0) ^ (v2.getInt() != 0));
      return P_Expr::NO_ERR;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
};
//------------------------------------------------------------------------
class operBlockEQU_DIFF : public clonable_impl<operBlock2, operBlockEQU_DIFF>
{
  public:
  protected:
    virtual P_Expr::errCode calcDouble(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      return P_Expr::ILLEGAL_TYPE;
    }
    virtual bool onShortCircuit(P_Expr::exprVar& result);
    bool onShort;
  private:
    typedef operBlock2 baseClass;
};
//------------------------------------------------------------------------
class operBlockEQU2 : public clonable_impl<operBlockEQU_DIFF, operBlockEQU2>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() == v2.getInt();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
class operBlockDIFF2 : public clonable_impl<operBlockEQU_DIFF, operBlockDIFF2>
{
  public:
  protected:
    virtual P_Expr::errCode calcI64(P_Expr::exprVar& result, const P_Expr::exprVar& v1, const P_Expr::exprVar& v2)
    {
      result.iValue = v1.getInt() != v2.getInt();
      return P_Expr::NO_ERR;
    }
};
//------------------------------------------------------------------------
#endif
