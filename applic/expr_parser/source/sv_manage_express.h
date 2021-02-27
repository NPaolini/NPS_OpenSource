//-------- sv_manage_express.h -------------------------------------
#ifndef sv_manage_express_H_
#define sv_manage_express_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_vect.h"
#include "p_express.h"
#include "p_operation.h"
#include "p_tstack_simple.h"
#include "expr_parser_dll.h"
//------------------------------------------------------------------
#define ID_SHOW_ERROR 100
#define ID_INIT_SHOW_ERROR 101
#define ID_VAR_NOT_FOUND ID_INIT_SHOW_ERROR
#define ID_EXPR_ERROR 102
//------------------------------------------------------------------
#define SKIP_NAME _T("_SKIP")
#define ABORT_NAME _T("_ABORT")
//------------------------------------------------------------------
class svExpr;
class svFunctExpr;
//------------------------------------------------------------------
struct infoExpr
{
  int idResult;
  int typeResult;

  // viene usato anche come flag di variabile, cioè indica che l'offset
  // vero viene ricavato da una variabile
  int offset;
  int step;
  int cycle;
  svExpr* Expr;
};
//------------------------------------------------------------------
bool operator ==(const P_Expr::exprVar& v1, const P_Expr::exprVar& v2);
//------------------------------------------------------------------
enum ePartBlock { epVars, epFunct, epCalc, epInit, epbMax };
//------------------------------------------------------------------
struct infoBuff
{
  LPTSTR buff;
  uint dim;
  infoBuff() : buff(0), dim(0) {}
};
//------------------------------------------------------------------
class replaceVarName;
//------------------------------------------------------------------
class manExpressionVars
{
  public:
    manExpressionVars() : currRow(0), infoB(0), ReplaceVar(0) {}
    virtual ~manExpressionVars() {}
    virtual P_Expr::exprVar getVariable(int id, int offset, bool& exist) = 0;
    virtual void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName) {}

    // se viene implementata una cache per i dati da inviare occorre usare questa per inviare tutti i dati
    virtual void sendFinal() {}
    // se viene implementata una cache, anziché distruggerla e ricrearla ad ogni ciclo, si ricaricano i dati
    virtual void reloadCache() {}
    virtual void shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR functName = 0) {}
    virtual void manageInit(LPTSTR p) {}
    virtual void manageVar(LPTSTR p) {}
    virtual int getTypeResult(int id) { return 0; }
    virtual void reset() {}

    // prima inizializzazione, necessaria se nel file delle espressioni la zona di inizializzazione
    // non fosse inserita dopo la zona delle variabili e quindi prima di conoscere la posizione
    // effettiva
    virtual void performInit() {}

    uint getCurrRow() const { return currRow; }
    void setCurrRow(uint row) { currRow = row; }

    void setReplaceVar(replaceVarName* replVar) { ReplaceVar = replVar; }
    void setOrigBuff(const infoBuff* origBuff) { infoB = origBuff; }

  protected:
    replaceVarName* ReplaceVar;
    const infoBuff* infoB;

  private:
    uint currRow;
};
//------------------------------------------------------------------
class svManExpression;
class manExpressionFunct;
//------------------------------------------------------------------
class svCustFunct : public P_CustFunct
{
  private:
    typedef P_CustFunct baseClass;

  public:
    svCustFunct(manExpressionFunct* owner) : Owner(owner), ReplaceVar(0) {  }
    ~svCustFunct();
    virtual LPCTSTR getName() const;
    virtual bool parse(LPTSTR buff, uint len);

    P_Expr::errCode execute(P_Expression* owner, P_Expr::exprVar& result, PVect<P_Expr::exprVar>& v);
    virtual P_Expr::exprVar getVariable(int id, int offset, bool& exist);

    void setName(LPCTSTR name);
    virtual void moveFrom(const P_CustFunct* other);
  protected:
    manExpressionFunct* Owner;
    replaceVarName* ReplaceVar;
    PVect<P_Expr::exprVar> param;
    PVect<P_Expr::exprVar> local;
    PVect<infoExpr> pExpr;
    void shoMsg(int id, LPCTSTR var, int num2, int row);
    int manageNameParam(LPTSTR p);
    void manageCalc(LPTSTR p);
    void manageLine(LPTSTR p);
    void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName);
};
//------------------------------------------------------------------
class manExpressionFunct
{
  public:
    manExpressionFunct(svManExpression* owner) : Owner(owner) {}
    ~manExpressionFunct() { flushPV(Functs); }
    void parse(LPTSTR buff, uint dim);
    P_CustFunct* alloc(LPCTSTR functname);
    svManExpression* getMainOwner() { return Owner; }
    virtual void shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR functName = 0);
    LPCTSTR getRealName(uint id);
    manExpressionVars* getManVars();

  protected:
    svManExpression* Owner;
    PVect<P_CustFunct*> Functs;
};
//------------------------------------------------------------------
extern manExpressionVars* allocManVars(pInfoCallBack p_info_fz);
//------------------------------------------------------------------------
typedef P_TStackSimple<P_CustFunct*, 500> functStack;
//------------------------------------------------------------------
class svManExpression
{
  public:
    svManExpression(pInfoCallBack p_info_fz);
    ~svManExpression();

    bool makeExpression(LPCTSTR p);
    bool makeExpressionByBuff(LPCTSTR buff);
    bool makeExpressionByBuff(LPTSTR buff);
    bool performExpression();
    P_Expr::exprVar getVariable(int id, int offset, bool& exist);

    enum state { sUndef = -1, sInit, sVars = sInit, sCalc, sInitialize, sMaxState };

    P_CustFunct* getOwnerFunct(LPCTSTR functname) { return pManFunct ? pManFunct->alloc(functname) : 0; }
    P_CustFunct* getCurrFunct() { return FunctStack.getCurr(); }
    bool pushFunct(P_CustFunct* f) { return FunctStack.Push(f); }
    void popFunct() { FunctStack.Pop(); }
    void shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR functName = 0);
    LPCTSTR getRealName(uint id);
    manExpressionVars* getManVars() { return pManVars; }
  private:
    PVect<infoExpr> pExpr;

    manExpressionVars* pManVars;

    manExpressionFunct* pManFunct;
    functStack FunctStack;

    void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName);
//    P_Expr::exprVar loadVal(const infoVar& ivar, int offset);
//    bool getVariableAndData(int id, int offset, P_Expr::exprVar& var, infoVar& idata);

    int currState;
//    void manageMarker(LPTSTR p);
    void manageInit(LPTSTR p);
    void manageVar(LPTSTR p);
    int getTypeResult(int id);
    void manageCalc(LPTSTR p);
    void manageLine(LPTSTR p);

    replaceVarName* ReplaceVar;
    bool hasError() const { return Error; }
    void resetError() { Error = false; }
    bool Error;

    pInfoCallBack pInfoFz;

    infoBuff origBuff[epbMax];
    void resetOrigBuff();
};
//------------------------------------------------------------------
class svExpr : public P_Expression
{
  public:
    svExpr(svManExpression* owner, LPCTSTR expr = 0) :
        Owner(owner), P_Expression(expr) {}

    P_CustFunct* getOwnerFunct(LPCTSTR functname) { return Owner->getOwnerFunct(functname); }
    P_CustFunct* getCurrFunct() { return Owner->getCurrFunct(); }
    bool pushFunct(P_CustFunct* f) {  return Owner->pushFunct(f); }
    void popFunct() { Owner->popFunct(); }
  protected:
    virtual P_Expr::exprVar getVariable(int id, int offset, bool& exist)
      {
        return Owner->getVariable(id, offset, exist);
      }
  private:
    svManExpression* Owner;
};
//---------------------------------------------------------
extern LPTSTR getLine(LPTSTR buff, int len);
//---------------------------------------------------------
//extern svManExpression::state getState(LPCTSTR p, int old);
//---------------------------------------------------------
//----------------------------------------------------------
#endif
