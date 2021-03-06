//----------- testManVars.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef testManVars_H_
#define testManVars_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <richedit.h>
#include "resource.h"
#include "pModDialog.h"
#include "p_Util.h"
#include "p_file.h"
#include "PBitmap.h"
#include "sv_manage_express.h"
#include "defgPerif.h"
//----------------------------------------------------------------------------
#if (MAX_DWORD_PERIF > SIZE_OTHER_DATA)
  #define DIM_DAT MAX_DWORD_PERIF
#else
  #define DIM_DAT SIZE_OTHER_DATA
#endif
//----------------------------------------------------------------------------
class testManVars : public manExpressionVars
{
  private:
    typedef manExpressionVars baseClass;
  public:
    testManVars(PWin* owner) : Owner(owner), functName(0) {}
    virtual ~testManVars() {}
    virtual P_Expr::exprVar getVariable(int id, int offset, int offsByStep, bool& exist);
    virtual void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName);
    virtual void shoMsg(int id, LPCTSTR var, int num2, int row, LPCTSTR funct_name = 0);
    virtual void manageInit(LPTSTR p);
    virtual void manageVar(LPTSTR p);
    virtual int getTypeResult(int id);
    virtual void reset() { Vars.reset(); }

    virtual void performInit();

  protected:
    LPCTSTR functName;
  private:
    PWin* Owner;
    struct infoVar
    {
      int id;
      int prf;
      int addr;
      int type;
      int nbit;
      int offs;
      int norm;
    };
    bool findId(int id);
    typedef PVect<infoVar> vInfoVar;
    vInfoVar Vars;

    typedef PVect<int> vInfoInit;
    vInfoInit Inits;

    struct infoDat
    {
      DWORD value[DIM_DAT];
      infoDat() { ZeroMemory(value, sizeof(value)); }
      DWORD& operator[](int offs) { return value[offs]; }
    };

    typedef PVect<infoDat> infoAllDat;

    infoAllDat Dat;
    P_Expr::exprVar loadVal(const infoVar& ivar, int offset, int offsByStep);

    void loadRowVals(LPCTSTR p);
    void loadEditField(HWND hed);

    void setVal(const P_Expr::exprVar& var, const infoVar& ivar, int offset);
    void replaceVal(const P_Expr::exprVar& var, int id, int offset);
    int getTrueOffset(int offset, bool& exist);

    void resolveRowCol(int& col, int& row);
    int getTrueRowFunct(int row);
    int isFalseRow(int row);
    int parseVar(LPTSTR buff, int pos, int& dimVar);
    int getTrueCol(int col, int row);
    void selectErr(int col, int row);
    void selectErr(LPCTSTR text, int row);
    void selectErr(int row);
};
//----------------------------------------------------------------------------
#endif
