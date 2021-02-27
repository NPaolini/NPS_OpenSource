//----------- replaceVarName.cpp --------------------------
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include "replaceVarName.h"
#include "p_avl.h"
#include "p_vect.h"
#include "sv_manage_express.h"
//---------------------------------------------------------
//---------------------------------------------------------
struct infoName : public genericSet
{
  LPCTSTR name;
  uint id;

  infoName() : name(0), id(0) {}
  infoName(LPCTSTR name) : name(name), id(0) {}
  infoName(LPCTSTR name, uint id) : name(name), id(id) {}
};
//---------------------------------------------------------
class setVarName : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setVarName() : P_Avl(true) { }
    virtual bool Search(LPCTSTR name) const;

  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
bool setVarName::Search(LPCTSTR name) const
{
  infoName in(name);
  return baseClass::Search(&in);
}
//----------------------------------------------------------------------------
#if 1
int setVarName::Cmp(const TreeData toCheck, const TreeData current) const
{
  infoName* check = (infoName*)toCheck;
  infoName* curr = (infoName*)current;
  return _tcsicmp(check->name, curr->name);
}
#else
//----------------------------------------------------------------------------
inline
bool operator >(const infoName& a, const infoName& b)
{
  return _tcscmp(a.name, b.name) > 0;
}
//----------------------------------------------------------------------------
inline
bool operator <(const infoName& a, const infoName& b)
{
  return _tcscmp(a.name, b.name) < 0;
}
//----------------------------------------------------------------------------
inline
bool operator ==(const infoName& a, const infoName& b)
{
  return !_tcscmp(a.name, b.name);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int setVarName::Cmp(const TreeData toCheck, const TreeData current) const
{
  infoName* check = (infoName*)toCheck;
  infoName* curr = (infoName*)current;
  if(*check < *curr)
    return -1;
  if(*check > *curr)
    return 1;
  return 0;
}
#endif
//---------------------------------------------------------
class setVarId : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setVarId() : P_Avl(false) { }
    virtual bool Search(uint id) const;

  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
bool setVarId::Search(uint id) const
{
  infoName in(0, id);
  return baseClass::Search(&in);
}
//----------------------------------------------------------------------------
int setVarId::Cmp(const TreeData toCheck, const TreeData current) const
{
  infoName* check = (infoName*)toCheck;
  infoName* curr = (infoName*)current;
  return (int)check->id - (int)curr->id;
}
//---------------------------------------------------------
bool isReturn(LPCTSTR p, int& pos)
{
  pos = 0;
  while(*p && *p <= _T(' ')) {
    ++p;
    ++pos;
    }
  if(!*p)
    return false;
  static TCHAR ret[] = _T("return ");
  for(uint i = 0; i < SIZE_A(ret) - 1; ++i, ++pos)
    if(_totlower(p[i]) != ret[i])
      return false;
  return true;
}
//---------------------------------------------------------
class iReplaceVarNameBase : public replaceVarName
{
  public:
    iReplaceVarNameBase() : gCount(1), newLen(0), firstVars(true), needExist(false) {}
    virtual ~iReplaceVarNameBase();

    LPTSTR run(LPTSTR buff, uint &len, bool& canDelete, int leftValue, bool first);
    LPCTSTR getName(uint id);
    uint getId(LPCTSTR name);
    virtual void setStartId(uint id) { gCount = id; }
    virtual void resetStartId() { gCount = 1; }
    virtual uint getIdAdd(LPCTSTR name) { return getIdByName(name); }

    virtual void flushWork() { flushPAV(SetRow); }

  protected:
    void finalReplace(LPTSTR buff, size_t lenBuff);
    virtual void replaceLine(LPTSTR p) = 0;
    void manageCalc(LPTSTR p, bool skipFirst, TCHAR match);

    setVarName SetVar;
    setVarId   SetId;
    PVect<LPTSTR> SetName;
    PVect<LPTSTR> SetRow;
    uint gCount;
    int newLen;

    LPTSTR extractVar(LPTSTR target, LPTSTR buff);
    uint getIdByName(LPCTSTR p, bool onlyNew = false);
    uint getNewIdByName(LPCTSTR p);
    void addRow(LPTSTR p);

    bool firstVars;

    bool needExist;
};
//---------------------------------------------------------
class iReplaceVarName : public iReplaceVarNameBase
{
  public:
    iReplaceVarName() {}
    ~iReplaceVarName() {}

  protected:
    void replaceLine(LPTSTR p);
    void manageOnlyFirstVar(LPTSTR p);
};
//---------------------------------------------------------
class iReplaceVarName4Funct : public iReplaceVarNameBase
{
  public:
    iReplaceVarName4Funct() : firstLine(true) {}
    ~iReplaceVarName4Funct() {}

  protected:
    void replaceLine(LPTSTR p);
    bool firstLine;
};
//---------------------------------------------------------
replaceVarName* allocReplaceVarName()
{
  return new iReplaceVarName;
}
//------------------------------------------------------------------
replaceVarName* allocReplaceVarName4Funct()
{
  return new iReplaceVarName4Funct;
}
//---------------------------------------------------------
iReplaceVarNameBase::~iReplaceVarNameBase()
{
  flushPAV(SetName);
  flushPAV(SetRow);
}
//---------------------------------------------------------
LPTSTR iReplaceVarNameBase::run(LPTSTR buff, uint &Len, bool& canDelete, int leftValue, bool first)
{
  LPTSTR p = buff;
  firstVars = first;
  flushPAV(SetRow);
  LeftValue = leftValue;
  int len = Len;
  newLen = 0;
  while(len && p) {
    LPTSTR p2 = getLine(p, len);
    int offs = p2 ? p2 - p : len;
    replaceLine(p);
    len -= offs;
    p = p2;
    }
  if(newLen > (int)Len) {
    if(canDelete)
      delete []buff;
    buff = new TCHAR[newLen + 2];
    canDelete = true;
    }
  finalReplace(buff, newLen);
  flushPAV(SetRow);
  Len = newLen;
  return buff;
}
//---------------------------------------------------------
LPCTSTR iReplaceVarNameBase::getName(uint id)
{
  if(SetId.Search(id)) {
    infoName* in = (infoName*)SetId.getCurr();
    return in->name;
    }
  return 0;
}
//---------------------------------------------------------
uint iReplaceVarNameBase::getId(LPCTSTR name)
{
  if(SetVar.Search(name)) {
    infoName* in = (infoName*)SetVar.getCurr();
    return in->id;
    }
  return 0;
}
//---------------------------------------------------------
void iReplaceVarNameBase::finalReplace(LPTSTR buff, size_t lenBuff)
{
  uint nElem = SetRow.getElem();
  if(!nElem || !buff)
    return;
  LPTSTR p = buff;
  uint totLen = lenBuff + 1;
  for(uint i = 0; i < nElem; ++i) {
    _tcscpy_s(p, totLen, SetRow[i]);
    uint len = _tcslen(p);
    p += len;
    totLen -= len;
    _tcscpy_s(p, totLen, _T("\r\n"));
    p += 2;
    totLen -= 2;
    }
  *p++ = 0;
  *p = 0;
}
//---------------------------------------------------------
inline
bool isValid(unsigned v)
{
  return _istalnum(v) || _T('_') == v;
}
//---------------------------------------------------------
#define MAX_VAR_LEN 64
//---------------------------------------------------------
LPTSTR iReplaceVarNameBase::extractVar(LPTSTR target, LPTSTR buff)
{
  if(!buff || !*buff)
    return 0;

  for(uint i = 0; i < MAX_VAR_LEN; ++i, ++buff) {
    target[i] = *buff;
    if(!isValid(target[i])) {
      target[i] = 0;
      break;
      }
    }
  return buff;
}
//---------------------------------------------------------
uint iReplaceVarNameBase::getIdByName(LPCTSTR p, bool onlyNew)
{
  if(!p)
    return 0;
  if(SetVar.Search(p)) {
    if(onlyNew)
      return (uint)-1;
    infoName* in = (infoName*)SetVar.getCurr();
    return in->id;
    }
  if(needExist)
    return 0;
  uint nElem = SetName.getElem();
  SetName[nElem] = str_newdup(p);
  infoName* in = new infoName(SetName[nElem], gCount++);
  SetVar.Add(in);
  SetId.Add(in);
  return gCount - 1;
}
//---------------------------------------------------------
void iReplaceVarNameBase::addRow(LPTSTR p)
{
  if(p && *p) {
    uint nElem = SetRow.getElem();
    SetRow[nElem] = str_newdup(p);
    newLen += _tcslen(p) + 2;
    }
}
//------------------------------------------------------------------
void iReplaceVarNameBase::manageCalc(LPTSTR p, bool skipFirst, TCHAR match)
{
  TCHAR result[5000] = _T("\0");
  if(skipFirst && *p) {
    LPTSTR pr = result;
    *pr++ = *p++;
    while(*p && *p != match)
      *pr++ = *p++;
    *pr = 0;
    if(*p)
      ++p;
    }
  needExist = VAR_FUNCT_CHAR != match;
  while(p && *p) {
    TCHAR v[MAX_VAR_LEN];
    p = extractVar(v, p);
    if(!p)
      break;
    uint id = getIdByName(v);
    if(!id) {
      MessageBox(0, v, _T("Variabile non esistente"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
      break;
      }
    TCHAR t[500];
    LPTSTR p2 = p;
    while(*p2) {
      if(match == *p2) {
        *p2 = 0;
        ++p2;
        break;
        }
      ++p2;
      }
    wsprintf(t, _T("%c%d%s"), match, id, p);
    _tcscat_s(result, SIZE_A(result), t);
    p = p2;
    }
  if(*result)
    addRow(result);
}
//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------
void iReplaceVarName::replaceLine(LPTSTR p)
{
  if(!p)
    return;
  int pos = 0;
  bool isRet = isReturn(p, pos);
  if(isRet) {
    manageCalc(p, true, VAR_CHAR);
    return;
    }
  while(*p && *p != VAR_CHAR && *p != _T(';') && *p != VAR_FUNCT_CHAR && *p != _T('#') && *p != _T('{') && *p != _T('}'))
    ++p;

  if(VAR_CHAR == *p) {
    if(LeftValue > 0) {
      if(firstVars) {
        firstVars = false;
        manageOnlyFirstVar(SKIP_NAME _T("=0,0,4"));
        manageOnlyFirstVar(ABORT_NAME _T("=0,0,4"));
        }
      manageOnlyFirstVar(p + 1);
      }
    else
      manageCalc(p + 1, false, VAR_CHAR);
    }
    // se siamo nel blocco funzioni, non deve gestire le variabili inizianti con VAR_FUNCT_CHAR(@)
  else {
    switch(*p) {
      case VAR_FUNCT_CHAR:
      case _T('{'):
      case _T('}'):
      case _T('#'):
        manageCalc(p, true, VAR_CHAR);
        break;
      }
    }
  if(isRet)
    delete []p;
}
//------------------------------------------------------------------
void iReplaceVarName::manageOnlyFirstVar(LPTSTR p)
{
  TCHAR v[MAX_VAR_LEN];
  p = extractVar(v, p);
  if(!p)
    return;

  needExist = false;
  uint id = getIdByName(v, 1 == LeftValue);
  if((uint)-1 == id) {
    id = 0;
    MessageBox(0, v, _T("Trovata variabile duplicata"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
    }
  if(!id)
    return;

  TCHAR t[500];
  wsprintf(t, _T("$%d%s"), id, p);
  addRow(t);
}
//---------------------------------------------------------
void iReplaceVarName4Funct::replaceLine(LPTSTR p)
{
  if(!p)
    return;
  int pos = 0;
  bool isRet = isReturn(p, pos);
  if(isRet) {
    LPCTSTR fname = getName(ID_FUNCT_NAME);
    int lenname = _tcslen(fname);
    int diff = pos - (lenname + 2) - 1;
    LPTSTR p1 = 0;
    if(diff <= 0) {
      int len = _tcslen(p);
      len += -diff + 2;
      p1 = new TCHAR[len + 1];
      wsprintf(p1, _T("%c%s="), VAR_FUNCT_CHAR, fname);
      _tcscat_s(p1, len, p + pos);
      }
    else {
      p1 = str_newdup(p);
      wsprintf(p1, _T("%c%s="), VAR_FUNCT_CHAR, fname);
      for(int i = _tcslen(p1); i < pos; ++i)
        p1[i] = _T(' ');
      }
    p = p1;
    }
  while(*p && *p != _T(';') && *p != VAR_FUNCT_CHAR && *p != _T('#') && *p != _T('{') && *p != _T('}'))
    ++p;

  needExist = false;
  if(VAR_FUNCT_CHAR == *p) {
    if(firstLine) {
      getIdByName(SKIP_NAME, true);
      getIdByName(ABORT_NAME, true);
      }
    manageCalc(p + 1, false, VAR_FUNCT_CHAR);
    if(firstLine) {
      firstLine = false;
      setStartId(MAX_PARAM_ID);
      }
    }
  else if(*p == _T('{') || *p == _T('}'))
    manageCalc(p, true, VAR_FUNCT_CHAR);
  if(isRet)
    delete []p;
}
//------------------------------------------------------------------
