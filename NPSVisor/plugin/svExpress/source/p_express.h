//----------------- p_express.h ------------------------------------------
#ifndef P_EXPRESS_H_
#define P_EXPRESS_H_
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#define VAR_CHAR _T('$')
#define VAR_FUNCT_CHAR _T('@')
//------------------------------------------------------------------------
#define MAX_LEN_NAME_VARS_FUNCT 64
//------------------------------------------------------------------
//#define ADD_OFFS_VAR 25000
#define MAX_VARS 0x4000
#define ADD_OFFS_VAR (MAX_VARS * 2)
//------------------------------------------------------------------
inline int makeOffsVar(int ofs) {  return ofs + ADD_OFFS_VAR; }
inline int unmakeOffsVar(int ofs) {  return ofs - ADD_OFFS_VAR; }
inline bool isOffsVar(int ofs) {  return ofs >= MAX_VARS; }
//------------------------------------------------------------------------
class operatBase;
class P_Expression;
//------------------------------------------------------------------------
extern uint getNextCountOnMem();
extern void resetCountOnMem();
//------------------------------------------------------------------------
namespace P_Expr
{
//------------------------------------------------------------------------
LPCTSTR str_newdupStripSpace(LPCTSTR p);
//------------------------------------------------------------------------
  enum errCode {
    ONE_HAS_NO_ACTION = -2,
    NO_ACTION,
    NO_ERR,
    DIVISION_BY_0,
    ILLEGAL_OPERATION,
    UNDEFINED_VARIABLE,
    INVALID_DOMAIN,
    INVALID_EXPRESSION,
    ILLEGAL_TYPE,
    TO_MANY_CALL,
    ERR_ON_FUNCT
    };
//------------------------------------------------------------------------
#define ROUND_REAL_TO_i64(a) (__int64)((a) - ((a) < 0) + 0.5)
//------------------------------------------------------------------------
enum typeVal { tvReal, tvInt };
//------------------------------------------------------------------------
union step_offs_u
{
  struct {
    int step : 15;
    int offs : 17;
    } S;
  DWORD dw;
};
//------------------------------------------------------------------------
inline DWORD getAggr(int step, int offs)
{
  step_offs_u sou;
  sou.S.offs = offs;
  sou.S.step = step;
  return sou.dw;
}
//------------------------------------------------------------------------
inline int getUnaggrStep(DWORD dw)
{
  step_offs_u sou;
  sou.dw = dw;
  return sou.S.step;
}
//------------------------------------------------------------------------
inline int getUnaggrOffs(DWORD dw)
{
  step_offs_u sou;
  sou.dw = dw;
  return sou.S.offs;
}
//------------------------------------------------------------------------
#define myAGGREG(step, offs) getAggr(step, offs)
#define myUNAGGREG_STEP(v) getUnaggrStep(v)
#define myUNAGGREG_OFFS(v) getUnaggrOffs(v)
//------------------------------------------------------------------------
  struct exprVar
  {
    union
    {
      double dValue;
      __int64 iValue;
    };
    int type; // 0 -> double, 1 -> __int64

    bool nullVal; // il valore non va assegnato

    explicit exprVar(int type = tvInt) : type(type) , nullVal(false) { dValue = 0; }
    explicit exprVar(__int64 val) : type(tvInt), nullVal(false) { iValue = val; }
    explicit exprVar(double val) : type(tvReal), nullVal(false) { dValue = val; }

    double getReal() const { if(tvInt == type) return (double)iValue; return dValue; }
    __int64 getInt() const { if(tvInt == type) return iValue; return ROUND_REAL_TO_i64(dValue); }

    void set_variable(int id, int offset, int step) {
      type = tvInt;
      LARGE_INTEGER li;
      li.LowPart = id;
      li.HighPart = myAGGREG(step, offset);
      iValue = li.QuadPart;
      }

    void get_variable(int& id, int& offset, int& step) {
      LARGE_INTEGER li;
      li.QuadPart = iValue;
      id = li.LowPart;
      step = myUNAGGREG_STEP(li.HighPart);
      offset = myUNAGGREG_OFFS(li.HighPart);
      }
  };
//------------------------------------------------------------------------
  class node
  {
    public:
      node(operatBase* oper = 0, node* left = 0, node* right = 0) :
          left(left), right(right), oper(oper), more(0) { }
      ~node() { free(); }

      node(const node& other);
      const node& operator=(const node& other);

      P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result);

      node* left;
      node* right;
      // more è usato per funzioni con più di due argomenti, i primi due
      // occupano left e right, gli altri sono concatenati in more
      node* more;

      operatBase* oper;
      exprVar Val;
    private:
      void free();

  };
//------------------------------------------------------------------------
};
//------------------------------------------------------------------------
class operatBase
{
  public:
    typedef P_Expr::node opNode;
    typedef P_Expr::node* pTree;
    operatBase() {}
    virtual ~operatBase() {}
    virtual P_Expr::errCode calc(P_Expression* owner, P_Expr::exprVar& result, opNode* sender) = 0;
    virtual operatBase* makeObj() = 0;
};
//------------------------------------------------------------------------
class P_CustFunct
{
  public:
    P_CustFunct() : nArg(0) {}
    virtual ~P_CustFunct() {}

    uint getNumArg() const { return nArg; }
    virtual LPCTSTR getName() const = 0;
    virtual void setName(LPCTSTR name) = 0;
    virtual bool parse(LPTSTR buff, uint len) = 0;
    virtual P_Expr::errCode execute(P_Expression* owner, P_Expr::exprVar& result, PVect<P_Expr::exprVar>& v) = 0;
    virtual P_Expr::exprVar getVariable(int id, int offset, int offsByStep, bool& exist) = 0;
    virtual void moveFrom(const P_CustFunct* other) = 0;
  protected:
    int nArg;
};
//------------------------------------------------------------------------
class P_Expression
{
  public:
    P_Expression(LPCTSTR expr = 0);
    virtual ~P_Expression();

    const P_Expression& operator=(const P_Expression& expr);
    P_Expression(const P_Expression& expr);

    typedef P_Expr::node* pTree;
    typedef P_Expr::node  Tree;

    // currCycle è usato in abbinamento a variabili in cui è stato impostato lo step
    P_Expr::errCode getValue(P_Expr::exprVar& val, int currCycle = 0);

    int UpdateTree();
    int ChangeExpression(LPCTSTR expr);

    // per poter usare la classe in assenza di variabili
    virtual P_Expr::exprVar getVariable(int id, int offset, int offsByStep, bool& exist)
    { exist = false; P_Expr::exprVar val; return val; }
    int getCycle() const { return Cycle; }

    virtual P_CustFunct* getOwnerFunct(LPCTSTR functname) { return 0; }
    virtual P_CustFunct* getCurrFunct() { return 0; }
    virtual bool pushFunct(P_CustFunct* f) { return false; }
    virtual void popFunct() {  }

  protected:
    pTree CloneTree() const;
    pTree GetRadix();
  private:
    LPCTSTR Expr;
    int currPos;
    pTree Radix;

    // il tipo desiderato di ritorno viene preso dalla variabile passata
    int desiredType;
    int Cycle;
    P_Expr::exprVar vexp(pTree a);

    pTree makeLevel0_0(); // livelli di priorità 0 == più basso
    pTree makeLevel0_1();
    pTree makeLevel0_2();
    pTree makeLevel1_0();
    pTree makeLevel1_1();
    pTree makeLevel1_2();
    pTree makeLevel2();
    pTree makeLevel3();
    pTree makeLevel4();
    pTree makeLevel5();
    pTree makeLevel6();
    pTree makeLevelHigh(); // penultimo livello, segno '-' e parentesi
    pTree makeLevelLast(); // ultimo livello, costanti, variabili e funzioni

    pTree makeConstValue();
    pTree makeVarValue(uint code);
    pTree makeFunctValue();

    pTree clone(pTree a) const;
    P_Expr::errCode Result;

    void getOffsStep(int& offs, int& step);
    int getEndFunct();
};
#endif
