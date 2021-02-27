//---------- expr_parser_Class.cpp -------------------------------------------
//----------------------------------------------------------------------------
//#include "precHeader.h"
//----------------------------------------------------------------------------
#include "expr_parser_Class.h"
//----------------------------------------------------------------------------
class pParserDll
{
  public:
    ~pParserDll();

    static prtParserProcDLL ParserProcDLL;
  private:
    pParserDll();
    static pParserDll dummy;
    static HMODULE hDLL;
    static bool loadProc();
    static LRESULT ConnProcDummy(uint codeMsg, LPVOID p1, HANDLE_PARSER pConn) { return 0; }
};
//------------------------------------------------------------------
pParserDll pParserDll::dummy;
HMODULE pParserDll::hDLL;
prtParserProcDLL pParserDll::ParserProcDLL;
//------------------------------------------------------------------
#define NAME_PARSER_DLL _T("expr_parser")
//------------------------------------------------------------------
pParserDll::pParserDll()
{
  if(!loadProc())
    MessageBox(0, _T("Unable to load [") NAME_PARSER_DLL _T(".dll]"), _T("Error"), MB_OK | MB_ICONSTOP);
}
//------------------------------------------------------------------
pParserDll::~pParserDll()
{
  if(hDLL)
    FreeLibrary(hDLL);
}
//------------------------------------------------------------------
static LPCTSTR libName = NAME_PARSER_DLL _T(".dll");
static LPCSTR procName = "_ParserProcDLL";
//------------------------------------------------------------------
bool pParserDll::loadProc()
{
  hDLL = LoadLibrary(libName);
  if(hDLL) {
    FARPROC f = GetProcAddress(hDLL, procName);
    if(!f)
      f = GetProcAddress(hDLL, procName + 1);
    if(f) {
      ParserProcDLL = (prtParserProcDLL)f;
      return true;
      }
    }
  ParserProcDLL = ConnProcDummy;
  return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t (*fz)(const T*)>
T* t_str_newdup(const T* str)
{
  if(!str) {
    T* tmp = new T[1];
    tmp[0] = 0;
    return tmp;
    }
  size_t len = fz(str) + 1;
  T* tmp = new T[len];
  for(size_t i = 0; i < len; ++i)
    tmp[i] = str[i];
  return tmp;
}
//----------------------------------------------------------------------------
LPSTR str_newdup(LPCSTR str)
{
  return t_str_newdup<char, strlen>(str);
}
//----------------------------------------------------------------------------
LPWSTR str_newdup(LPCWSTR str)
{
  return t_str_newdup<wchar_t, wcslen>(str);
}
//----------------------------------------------------------------------------
bool operator <(const ParserInfo& pi1, const ParserInfo& pi2)
{
  int diff = _wcsicmp(pi1.varName, pi2.varName);
  if(diff < 0)
    return true;
  if(!diff)
    return pi1.offset < pi2.offset;
  return false;
}
//----------------------------------------------------------------------------
bool operator <(const P_ExprMake::funct& f1, const P_ExprMake::funct& f2)
{
  return f1.id < f2.id;
}
//----------------------------------------------------------------------------
P_ExprMake::P_ExprMake() : Parser(0), Prepared(0), DimBuff(0), canDeleteFile(false) { ZeroMemory(&Icb, sizeof(Icb)); }
//----------------------------------------------------------------------------
P_ExprMake::~P_ExprMake()
{
  if(Parser)
    pParserDll::ParserProcDLL(cDestroy, 0, Parser);
  flushAll();
}
//----------------------------------------------------------------------------
void P_ExprMake::flushAll()
{
  clearPrepared();

  for(std::set<ParserInfo>::iterator it=Vars.begin(); it!=Vars.end(); ++it) {
    const ParserInfo& tmp = *it;
    ParserInfo& it2= const_cast<ParserInfo&>(tmp);
    delete []it2.varName;
    }

  Vars.clear();
  Calc.clear();
  Funct.clear();
}
//----------------------------------------------------------------------------
void P_ExprMake::clearPrepared()
{
  if(!Prepared)
    return;
  if(!DimBuff && canDeleteFile) {
    DeleteFile(Prepared);
    canDeleteFile = false;
    }
  delete []Prepared;
  Prepared = 0;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addVar(const ParserInfo& v)
{
  clearPrepared();

  ParserInfo v2 = v;
  v2.varName = str_newdup(v.varName);
  std::pair<std::set<ParserInfo>::iterator,bool> ret = Vars.insert(v2);
  if(!ret.second)
    delete []v2.varName;
  return ret.second;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addCalcRow(const string_t& row)
{
  clearPrepared();
  Calc.push_back(row);
  return true;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addFunctName(uint id, const string_t& name)
{
  clearPrepared();
  funct f(id, name);
  std::pair<std::set<funct>::iterator,bool> ret = Funct.insert(f);
  return ret.second;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addCalcRowFunct(uint id, const string_t& row)
{
  clearPrepared();
  funct f = { id, _T("") };
  std::set<funct>::iterator it = Funct.find(f);
  if(Funct.end() != it) {
    std::vector<string_t>& Calc = it->Calc;
    Calc.push_back(row);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void P_ExprMake::writeVars(ofstream_t& pf)
{
  pf << _T("<VARS>") << std::endl;
#ifdef Unicode
  for (std::set<ParserInfo>::iterator it = Vars.begin(); it != Vars.end(); ++it)
    if(!it->offset)
      pf << _T("$") << it->varName << _T("=") << it->type << std::endl;
#else
  for(std::set<ParserInfo>::iterator it=Vars.begin(); it!=Vars.end(); ++it) {
    if(!it->offset) {
      std::wstring t(it->varName);
      string_t t2(t.begin(), t.end());
      pf << _T("$") << t2 << _T("=") << it->type << std::endl;
      }
    }
#endif
  pf << _T("</VARS>") << std::endl;
}
//----------------------------------------------------------------------------
void P_ExprMake::writeFunct(ofstream_t& pf)
{
  pf << _T("<FUNCT>") << std::endl;
  for(std::set<funct>::iterator it=Funct.begin(); it!=Funct.end(); ++it) {
    pf << it->name_parameter << std::endl << _T("{") << std::endl;
    std::vector<string_t>& calc = it->Calc;
    for(std::vector<string_t>::iterator it2=calc.begin(); it2!=calc.end(); ++it2) {
      pf << *it2 << std::endl;
      }
    pf << _T("}") << std::endl;
    }
  pf << _T("</FUNCT>") << std::endl;
}
//----------------------------------------------------------------------------
void P_ExprMake::writeCalc(ofstream_t& pf)
{
  pf << _T("<CALC>") << std::endl;
  for(std::vector<string_t>::iterator it=Calc.begin(); it!=Calc.end(); ++it) {
    pf << *it << std::endl;
    }
  pf << _T("</CALC>") << std::endl;
}
//----------------------------------------------------------------------------
bool P_ExprMake::make()
{
  if(Prepared)
    return true;
  if(Vars.empty() || Calc.empty())
    return false;
  TCHAR path[_MAX_PATH];
  if(!GetTempFileName(_T("."), _T("nps"), 0, path))
    return false;
  ofstream_t pf(path);
  writeVars(pf);
  writeFunct(pf);
  writeCalc(pf);
  pf.close();
  return make(path, true);
}
//----------------------------------------------------------------------------
bool P_ExprMake::make(LPCTSTR filename, bool canDelete)
{
  if(!canDelete)
    flushAll();
  Prepared = str_newdup(filename);
  DimBuff = 0;
  canDeleteFile = canDelete;
  return true;
}
//----------------------------------------------------------------------------
bool P_ExprMake::parse()
{
  if(!Parser)
    Parser = (HANDLE_PARSER)pParserDll::ParserProcDLL(cMakeHandle, (LPDWORD)&Icb, (HANDLE_PARSER)1);
  if(!Parser)
    return false;
  if(!Prepared)
    if(!make())
      return false;
  BuffParser bp;
  bp.buff = Prepared;
  bp.len = DimBuff;
  bool success = toBool(pParserDll::ParserProcDLL(cParse, (LPDWORD)&bp, Parser));
  if(!DimBuff && canDeleteFile) {
    DeleteFile(Prepared);
    canDeleteFile = false;
    delete []Prepared;
    Prepared = 0;
    }
  return success;
}
//----------------------------------------------------------------------------
bool P_ExprMake::perform()
{
  if(!Parser)
    return false;
  return toBool(pParserDll::ParserProcDLL(cPerform, 0, Parser));
}
//----------------------------------------------------------------------------
static
LRESULT callBackBase(uint32_t msg, LPVOID param, LPVOID custom)
{
  if(!custom)
    return (LRESULT)-1;
  P_ExprManageBase* owner = (P_ExprManageBase*)custom;
  switch(msg) {
    case cb_MSG_GET_VAR:
      return owner->cbGetVar((pParserInfo)param);
    case cb_MSG_SET_VAR:
      return owner->cbSetVar((pParserInfo)param);
    case cb_MSG_ERROR:
      return owner->cbError((pParserError)param);
    case cb_MSG_DESTROY:
      return owner->cbDestroy();
    }
  return (LRESULT)-1;
}
//----------------------------------------------------------------------------
P_ExprManageBase::P_ExprManageBase(P_ExprMake& emk) : EMK(emk)
{
  Icb.fz_callBack = callBackBase;
  Icb.customData = 0;
}
//----------------------------------------------------------------------------
bool P_ExprManageBase::parse()
{
  Icb.customData = this;
  EMK.setCallBack(Icb);
  return EMK.parse();
}
//----------------------------------------------------------------------------
LRESULT P_ExprManageVar::cbGetVar(pParserInfo pi)
{
  std::set<ParserInfo>& Vars = getVars();
  std::set<ParserInfo>::iterator it = Vars.find(*pi);
  if(Vars.end() != it)
    pi->i = it->i;
  else {
    pi->i = 0;
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    }
  return 0;
}
//----------------------------------------------------------------------------
LRESULT P_ExprManageVar::cbSetVar(const ParserInfo* pi)
{
  std::set<ParserInfo>& Vars = getVars();
  std::set<ParserInfo>::iterator it = Vars.find(*pi);
  if(Vars.end() != it) {
    const ParserInfo& tmp = *it;
    ParserInfo& it2= const_cast<ParserInfo&>(tmp);
    it2.i = pi->i;
    }
  else {
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    }
  return 0;
}
//----------------------------------------------------------------------------
void P_ExprManageVar::makeRowOut(string_t& buff, const ParserInfo& pi)
{
  LPCWSTR_TO_STRING(buff, pi.varName);
  if(pi.offset) {
    buff += _T("[");
    buff += to_string_t(pi.offset);
    buff += _T("]");
    }
  buff += _T("=");
  if(itv_Int == pi.type)
    buff += to_string_t(pi.i);
  else
    buff += to_string_t(pi.d);
}
//----------------------------------------------------------------------------
void P_ExprManageVar::sendAllVars()
{
  std::set<ParserInfo>& Vars = getVars();
  for (std::set<ParserInfo>::iterator it = Vars.begin(); it != Vars.end(); ++it) {
    string_t t;
    makeRowOut(t, *it);
    sendOut(t);
    }
}
//----------------------------------------------------------------------------
