//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "expr_parser_Class.h"
//----------------------------------------------------------------------------
//#define ONLY_AT_END
//----------------------------------------------------------------------------
class testExpr: public P_ExprManageVar
{
  private:
    typedef P_ExprManageVar baseClass;
  public:
    testExpr(P_ExprMake& emk) : baseClass(emk) {}
    void setValue(const ParserInfo& pi) { baseClass::cbSetVar(&pi); }
  protected:
    virtual LRESULT cbError(const ParserError* pe);

#ifdef ONLY_AT_END
    virtual void sendOut(LPCTSTR varValue)
    {
      _tprintf(varValue);
      _tprintf(_T("\r\n"));
    }
#else
    void outTxt(const ParserInfo* pi, LPCTSTR prefix)
    {
      TCHAR t[1024];
      makeRowOut(t, SIZE_A(t), *pi);
      _tprintf(prefix);
      _tprintf(t);
      _tprintf(_T("\r\n"));

    }
    virtual LRESULT cbSetVar(const ParserInfo* pi)
    {
      LRESULT res = baseClass::cbSetVar(pi);
      outTxt(pi, _T("Set -> "));
      return res;
    }
    virtual LRESULT cbGetVar(pParserInfo pi)
    {
      LRESULT res = baseClass::cbGetVar(pi);
      outTxt(pi, _T("Get <- "));
        return res;
    }
#endif
};
//----------------------------------------------------------------------------
LRESULT testExpr::cbError(const ParserError* pe)
{
  switch(pe->code) {
    case ERR_EXPR_VAR:
      _tprintf(_T("Row [%d], Error expression [%s] at position [%d]"), pe->row, pe->varName, pe->col);
      break;
    case ERR_EXPR_PARAM:
      _tprintf(_T("Row [%d], Error Parameter"), pe->row);
      break;
    case ERR_EXPR_VAR_NOT_FOUND:
      _tprintf(_T("Row [%d], Variable [%s] not found"), pe->row, pe->varName);
      break;
    case ERR_EXPR_DUP_LABEL:
      _tprintf(_T("Label [%s] duplicated"), pe->varName);
      break;
    default:
      return 0;
    }
  if(pe->functName)
    _tprintf(_T(" - Function %s"), pe->functName);
  _tprintf(_T("\n-------------------------\n"));
  return 0;
}
//----------------------------------------------------------------------------
int __cdecl main()
{
  P_ExprMake mk;
  mk.make(_T("test_funct.svx.txt"), false);
  testExpr test(mk);
  if(test.parse()) {
    test.perform();
#ifdef ONLY_AT_END
    test.sendAllVars();
#endif
    }
  _tprintf(_T("-------------------------\r\n"));
  mk.flushAll();
  ParserInfo pi;
  ZeroMemory(&pi, sizeof(pi));
  pi.type = itv_real;
  TCHAR varname[] = _T("var0");
  pi.varName = varname;
  for(uint i = 0; i < 4; ++i) {
    varname[3] = _T('1') + i;
    mk.addVar(pi);
    }

  LPCTSTR test_row[] = {
    _T("$var1=2.+$var4"),
    _T("$var2=3"),
    _T("$var3=$var2*$var1*1.2"),
    _T("$var1[1]=$var3/2."),
    _T("$_skip= if($var1[1] > 0, null, #end)"),
    _T("$var2 = $var1 / $var1[1]"),
    _T("#end"),
  };
  for(uint i = 0; i < SIZE_A(test_row); ++i)
    mk.addCalcRow(test_row[i]);
  if(test.parse()) {
    test.perform();
#ifdef ONLY_AT_END
    test.sendAllVars();
#endif
    pi.d=2.4;
    test.setValue(pi);
    _tprintf(_T("-------------------------\r\n"));
    test.perform();
#ifdef ONLY_AT_END
    test.sendAllVars();
#endif
  }
}
