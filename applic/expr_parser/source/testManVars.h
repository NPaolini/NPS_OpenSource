//----------- testManVars.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef testManVars_H_
#define testManVars_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"
#include "p_Util.h"
#include "p_file.h"
#include "sv_manage_express.h"
#include "expr_parser_dll.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class testManVars : public manExpressionVars
{
  private:
    typedef manExpressionVars baseClass;
  public:
    testManVars(pInfoCallBack p_info_fz) : pInfoFz(p_info_fz), functName(0) {}
    virtual ~testManVars() {}
    virtual P_Expr::exprVar getVariable(int id, int offset, bool& exist);
    virtual void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName);
    virtual void shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR funct_name = 0);
    virtual void manageVar(LPTSTR p);
    virtual int getTypeResult(int id);
    virtual void reset() { Vars.reset(); }

  protected:
    LPCTSTR functName;
  private:
    pInfoCallBack pInfoFz;
    struct infoVar
    {
      int id;
      int type; // 0 -> intero, 1 -> real
      infoVar() : id(0), type(0) {}
      infoVar(int id, int type) : id(id), type(type) {}
    };
    bool findId(int id);
    typedef PVect<infoVar> vInfoVar;
    vInfoVar Vars;

    P_Expr::exprVar loadVal(const infoVar& ivar, int offset);

    void setVal(const P_Expr::exprVar& var, const infoVar& ivar, int offset);
    void replaceVal(const P_Expr::exprVar& var, int id, int offset);
    int getTrueOffset(int offset, bool& exist);

    int parseVar(LPCTSTR buff, int pos, int& dimVar);

    void resolveRowCol(int& col, int& row);
    int getTrueRowFunct(int row);
    int isFalseRow(int& offs);
    int getTrueCol(int col, int row);

};
//----------------------------------------------------------------------------
#endif
