//---------- expr_parser_Class.cpp -------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
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
#define NAME_DLL _T("expr_parser")
//------------------------------------------------------------------
pParserDll::pParserDll()
{
  if(!loadProc())
    MessageBox(0, _T("Unable to load [") NAME_DLL _T(".dll]"), _T("Error"), MB_OK | MB_ICONSTOP);
}
//------------------------------------------------------------------
pParserDll::~pParserDll()
{
  if(hDLL)
    FreeLibrary(hDLL);
}
//------------------------------------------------------------------
static LPCTSTR libName = NAME_DLL _T(".dll");
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
#ifdef USE_DUAL_VECT
//----------------------------------------------------------------------------
bool operator <(const ParserInfo& pi1, const ParserInfo& pi2)
{
  return _tcsicmp(pi1.varName, pi2.varName) < 0;
}
//----------------------------------------------------------------------------
bool operator <(const P_ExprMake::offsetValues& pi1, const P_ExprMake::offsetValues& pi2)
{
  return _tcsicmp(pi1.varName, pi2.varName) < 0;
}
//----------------------------------------------------------------------------
#else
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
#endif
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

  uint nElem = Vars.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete []Vars[i].varName;
  Vars.reset();
  flushPAV(Calc);
  nElem = Funct.getElem();
  for(uint i = 0; i < nElem; ++i) {
    delete []Funct[i].name_parameter;
    flushPAV(Funct[i].Calc);
    }
  Funct.reset();
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
  safeDeleteA(Prepared);
}
//----------------------------------------------------------------------------
bool P_ExprMake::addVar(const ParserInfo& v)
{
  clearPrepared();
  ParserInfo v2 = v;
  v2.varName = str_newdup(v.varName);
  if(!Vars.insert(v2)) {
    delete []v2.varName;
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------
bool P_ExprMake::remVar(LPCTSTR varname)
{
  clearPrepared();
  uint pos;
  ParserInfo pi;
  pi.varName = varname;
  if(Vars.find(pi, pos)) {
    pi = Vars.remove(pos);
    delete []pi.varName;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addCalcRow(LPCTSTR row)
{
  clearPrepared();
  Calc[Calc.getElem()] = str_newdup(row);
  return true;
}
//----------------------------------------------------------------------------
bool P_ExprMake::deleteRow(uint ix)
{
  clearPrepared();
  uint nElem = Calc.getElem();
  if(ix >= nElem)
    return false;
  delete []Calc.remove(ix);
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR P_ExprMake::getRow(uint ix)
{
  uint nElem = Calc.getElem();
  if(ix >= nElem)
    return 0;
  return Calc[ix];
}
//----------------------------------------------------------------------------
bool P_ExprMake::addFunctName(uint id, LPCTSTR name)
{
  clearPrepared();
  uint nElem = Funct.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(id == Funct[i].id || !_tcsicmp(name, Funct[i].name_parameter))
      return false;
    }
  Funct[nElem].name_parameter = str_newdup(name);
  Funct[nElem].id = id;
  return true;
}
//----------------------------------------------------------------------------
P_ExprMake::funct* P_ExprMake::getFunct(uint id)
{
  uint nElem = Funct.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(id == Funct[i].id)
      return &(Funct[i]);
  return 0;
}
//----------------------------------------------------------------------------
bool P_ExprMake::addCalcRowFunct(uint id, LPCTSTR row)
{
  clearPrepared();
  funct* f = getFunct(id);
  if(f) {
    PVect<LPCTSTR>& Calc = f->Calc;
    Calc[Calc.getElem()] = str_newdup(row);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool P_ExprMake::deleteRowFunct(uint id, uint ix)
{
  clearPrepared();
  funct* f = getFunct(id);
  if(!f)
    return false;
  PVect<LPCTSTR>& Calc = f->Calc;
  uint nElem = Calc.getElem();
  if(ix >= nElem)
    return false;
  delete []Calc[ix];
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR P_ExprMake::getRowFunct(uint id, uint ix)
{
  funct* f = getFunct(id);
  if(!f)
    return 0;
  PVect<LPCTSTR>& Calc = f->Calc;
  uint nElem = Calc.getElem();
  if(ix >= nElem)
    return 0;
  return Calc[ix];
}
//----------------------------------------------------------------------------
void P_ExprMake::writeVars(P_File& pf)
{
  uint nElem = Vars.getElem();
  pf.P_writeString(_T("<VARS>\r\n"));
  TCHAR t[4] = { 0, 0xd, 0xa, 0};
  for(uint i = 0; i < nElem; ++i) {
#ifndef USE_DUAL_VECT
    if(Vars[i].offset)
      continue;
#endif
    pf.P_writeString(_T("$"));
    pf.P_writeString(Vars[i].varName);
    pf.P_writeString(_T("="));
    t[0] = _T('0') + Vars[i].type;
    pf.P_writeString(t);
    }
  pf.P_writeString(_T("</VARS>\r\n"));
}
//----------------------------------------------------------------------------
void P_ExprMake::writeFunct(P_File& pf)
{
  uint nElem = Funct.getElem();
  pf.P_writeString(_T("<FUNCT>\r\n"));
  for(uint i = 0; i < nElem; ++i) {
    pf.P_writeString(Funct[i].name_parameter);
    pf.P_writeString(_T("\r\n{\r\n"));
    PVect<LPCTSTR>& calc = Funct[i].Calc;
    uint n = calc.getElem();
    for(uint j = 0; j < n; ++j) {
      pf.P_writeString(calc[j]);
      pf.P_writeString(_T("\r\n"));
      }
    pf.P_writeString(_T("}\r\n"));
    }
  pf.P_writeString(_T("</FUNCT>\r\n"));
}
//----------------------------------------------------------------------------
void P_ExprMake::writeCalc(P_File& pf)
{
  pf.P_writeString(_T("<CALC>\r\n"));
  uint n = Calc.getElem();
  for(uint j = 0; j < n; ++j) {
    pf.P_writeString(Calc[j]);
    pf.P_writeString(_T("\r\n"));
    }
  pf.P_writeString(_T("</CALC>\r\n"));
}
//----------------------------------------------------------------------------
bool P_ExprMake::make()
{
  if(Prepared)
    return true;
  if(!Vars.getElem() || !Calc.getElem())
    return false;
  TCHAR path[_MAX_PATH];
  if(!GetTempFileName(_T("."), _T("nps"), 0, path))
    return false;
  P_File pf(path, P_CREAT);
  if(!pf.P_open())
    return false;
  writeVars(pf);
  writeFunct(pf);
  writeCalc(pf);
  pf.P_close();
  return make(path, true);
}
//----------------------------------------------------------------------------
bool P_ExprMake::make(LPCTSTR filename, bool canDelete)
{
  // if it comes from an external file, better to eliminate all
  // instead if it comes from internal processing, is better to have already the variables
  if(!canDelete)
    flushAll();
  if(P_File::P_exist(filename)) {
    Prepared = str_newdup(filename);
    DimBuff = 0;
    canDeleteFile = canDelete;
    return true;
    }
  return false;
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
    safeDeleteA(Prepared);
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
  PVect<ParserInfo>& Vars = getVars();
  uint pos;
#ifdef USE_DUAL_VECT
  int offset = (int)pi->offset - 1;
  // if the variable does not exist
  if(!Vars.find(*pi, pos)) {
    pi->i = 0;
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    if(offset >= 0) {
      P_ExprMake::offsetValues ov(v.varName);
      ov.Values[offset] = 0;
      PVect<P_ExprMake::offsetValues>& Values = getValues();
      Values.insert(ov);
      }
    }
  else {
    // if there is an offset, the value is on the other vector
    if(offset >= 0) {
      PVect<P_ExprMake::offsetValues>& Values = getValues();
      // should not happen, if it exists in Vars must also exist here
      uint pos2;
      P_ExprMake::offsetValues ov(Vars[pos].varName);
      if(!Values.find(ov, pos2)) {
        ov.Values[offset] = pi->i = 0;
        Values.insert(ov);
        }
      else
        pi->i = Values[pos2].Values[offset];
      }
    else
      pi->i = Vars[pos].i;
    }
#else
  if(!Vars.find(*pi, pos)) {
    pi->i = 0;
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    }
  else
    pi->i = Vars[pos].i;
#endif
  return 0;
}
//----------------------------------------------------------------------------
LRESULT P_ExprManageVar::cbSetVar(const ParserInfo* pi)
{
  PVect<ParserInfo>& Vars = getVars();
  uint pos;
#ifdef USE_DUAL_VECT
  int offset = (int)pi->offset - 1;
  if(!Vars.find(*pi, pos)) {
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    if(offset >= 0) {
      P_ExprMake::offsetValues ov(v.varName);
      ov.Values[offset] = pi->i;
      PVect<P_ExprMake::offsetValues>& Values = getValues();
      Values.insert(ov);
      }
    }
  else {
    if(offset >= 0) {
      PVect<P_ExprMake::offsetValues>& Values = getValues();
      uint pos2;
      P_ExprMake::offsetValues ov(Vars[pos].varName);
      if(!Values.find(ov, pos2)) {
        ov.Values[offset] = pi->i;
        Values.insert(ov);
        }
      else
        Values[pos2].Values[offset] = pi->i;
      }
    else
      Vars[pos].i = pi->i;
    }
#else
  if(!Vars.find(*pi, pos)) {
    ParserInfo v = *pi;
    v.varName = str_newdup(pi->varName);
    Vars.insert(v);
    }
  else
    Vars[pos].i = pi->i;
#endif
  return 0;
}
//----------------------------------------------------------------------------
void P_ExprManageVar::makeRowOut(LPTSTR buff, size_t dim, const ParserInfo& pi)
{
  uint ix = pi.offset;
  if(ix) {
    if(itv_Int == pi.type)
      _stprintf_s(buff, dim, _T("%s[%d]=%I64i"), pi.varName, ix, pi.i);
    else {
      _stprintf_s(buff, dim, _T("%s[%d]=%lf"), pi.varName, ix, pi.d);
      zeroTrim(buff);
      }
    }
  else {
    if(itv_Int == pi.type)
      _stprintf_s(buff, dim, _T("%s=%I64i"), pi.varName, pi.i);
    else {
      _stprintf_s(buff, dim, _T("%s=%lf"), pi.varName, pi.d);
      zeroTrim(buff);
      }
    }
}
//----------------------------------------------------------------------------
void P_ExprManageVar::sendAllVars()
{
#ifdef USE_DUAL_VECT
  PVect<ParserInfo>& Vars = getVars();
  PVect<P_ExprMake::offsetValues>& Values = getValues();
  uint nElem = Vars.getElem();
  TCHAR t[1024];
  for(uint i = 0; i < nElem; ++i) {
    const ParserInfo& pi = Vars[i];
    makeRowOut(t, SIZE_A(t), pi);
    sendOut(t);
    uint pos;
    P_ExprMake::offsetValues ov(pi.varName);
    if(Values.find(ov, pos)) {
      const PVect<__int64>& values = Values[pos].Values;
      ParserInfo tpi = pi;
      uint n = values.getElem();
      for(uint j = 0; j < n; ++j) {
        tpi.i = values[j];
        tpi.offset = j + 1;
        makeRowOut(t, SIZE_A(t), tpi);
        sendOut(t);
        }
      }
    }
#else
  PVect<ParserInfo>& Vars = getVars();
  uint nElem = Vars.getElem();
  TCHAR t[1024];
  for(uint i = 0; i < nElem; ++i) {
    const ParserInfo& pi = Vars[i];
    makeRowOut(t, SIZE_A(t), pi);
    sendOut(t);
    }
#endif
}
//----------------------------------------------------------------------------
