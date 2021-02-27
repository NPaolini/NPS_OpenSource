//------ def_mainapp.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "expr_parser_Class.h"
#include <cstdio>
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
    virtual void sendOut(string_t varValue)
    {
      cout_t  << varValue << std::endl;
    }
#else
    void outTxt(const ParserInfo* pi, LPCTSTR prefix)
    {
      string_t t;
      makeRowOut(t, *pi);
      cout_t  << prefix << t << std::endl;

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
      cout_t << _T("Row [") << pe->row << _T("], Error expression [") << pe->varName << _T("] at position [")<< pe->col << _T("]");
      break;
    case ERR_EXPR_PARAM:
      cout_t << _T("Row [") << pe->row << _T("], Error Parameter");
      break;
    case ERR_EXPR_VAR_NOT_FOUND:
      cout_t << _T("Row [") << pe->row << _T("], Variable [") << pe->varName << _T("] not found");
      break;
    case ERR_EXPR_DUP_LABEL:
      cout_t << _T("Label [") << pe->varName << _T("], duplicated");
      break;
    default:
      return 0;
    }
  if(pe->functName)
    cout_t << _T(" - Function ")<< pe->functName;
  cout_t << std::endl << _T("-------------------------") << std::endl;
  return 0;
}
//----------------------------------------------------------------------------
#define SIZE_A(a) (sizeof(a) / sizeof(a[0]))
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
  cout_t  << _T("-------------------------") << std::endl;
  mk.flushAll();
  ParserInfo pi;
  ZeroMemory(&pi, sizeof(pi));
  pi.type = itv_real;
  wchar_t varname[] = L"var0";
  pi.varName = varname;
  for(uint i = 0; i < 4; ++i) {
    varname[3] = L'1' + i;
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
  for (uint i = 0; i < SIZE_A(test_row); ++i)
    mk.addCalcRow(test_row[i]);
  if(test.parse()) {
    test.perform();
#ifdef ONLY_AT_END
    test.sendAllVars();
#endif
    pi.d=2.4;
    test.setValue(pi);
    cout_t  << _T("-------------------------") << std::endl;
    test.perform();
#ifdef ONLY_AT_END
    test.sendAllVars();
#endif
  }
}
