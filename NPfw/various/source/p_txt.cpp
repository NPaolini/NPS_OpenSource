//---------- p_txt.h ----------------------------------------------
//-----------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include "pDef.h"
//-----------------------------------------------------------------
#include "p_txt.h"
#include "p_avl.h"
#include "p_file.h"
#include "p_Util.h"
#include "PCrt_lck.h"
//-----------------------------------------------------------------
// prototipi interne
static LPTSTR get_msg(LPTSTR p);
static LPTSTR get_line(LPTSTR buff, size_t len);
static void load_Set_gen(LPCTSTR file, class Set4Msg& Avl);
//-----------------------------------------------------------------
//----------------------------------------------------------
#define BOM_UTF_16_LE 0xfeff
#define BOM_UTF_16_BE 0xfffe
#define BOM_UTF_32_LE 0x0000feff
#define BOM_UTF_32_BE 0xfffe0000
//----------------------------------------------------------------------------
LPWSTR makeWCHAR(LPWSTR target, uint lenT, LPCSTR source, uint lenS, uint cp)
{
  *target = 0;
  if(!lenS)
    return target;
  MultiByteToWideChar(cp, 0, source, lenS, target, lenT);
  target[lenT] = 0;
  return target;
}
//----------------------------------------------------------------------------
LPSTR unmakeWCHAR(LPSTR target, uint lenT, LPCWSTR source, uint lenS, uint cp)
{
  *target = 0;
  if(!lenS)
    return target;
  if(BOM_UTF_16_LE == *(LPWORD)source || BOM_UTF_16_BE == *(LPWORD)source) {
    ++source;
    }
  else if(BOM_UTF_32_LE == *(LPDWORD)source || BOM_UTF_32_BE == *(LPDWORD)source) {
    source += 2;
    if(lenS > 1)
      lenS -= 2;
    }
  WideCharToMultiByte(cp, 0, source, lenS, target, lenT, 0, 0);
  return target;
}
//----------------------------------------------------------------------------
LPWSTR makeWCHAR(uint& len, LPCSTR buff, uint cp)
{
  if(!len)
    len = -1;
  uint len2 = MultiByteToWideChar(cp, 0, buff, len, 0, 0);
  LPWSTR p = new wchar_t[len2 + 1];
  MultiByteToWideChar(cp, 0, buff, len, p, len2);
  p[len2] = 0;
  len = len2;
  return p;
}
//----------------------------------------------------------------------------
LPSTR unmakeWCHAR(uint& len, LPCWSTR buff, uint cp)
{
  if(!len)
    len = -1;
  if(BOM_UTF_16_LE == *(LPWORD)buff || BOM_UTF_16_BE == *(LPWORD)buff) {
    ++buff;
    if(len > 0)
      --len;
    }
  else if(BOM_UTF_32_LE == *(LPDWORD)buff || BOM_UTF_32_BE == *(LPDWORD)buff) {
    buff += 2;
    if(len > 0)
      len -= 2;
    }
  int len2 = WideCharToMultiByte(cp, 0, buff, len, 0, 0, 0, 0);
  LPSTR p = new CHAR[len2];
  WideCharToMultiByte(cp, 0, buff, len, p, len2, 0, 0);
  len = len2;
  return p;
}
//----------------------------------------------------------------------------
LPSTR autoConvertA(LPVOID buff, uint& len, uint cp)
{
  LPSTR target = 0;
  if(IsTextUnicode(buff, len, 0)) {
    target = unmakeWCHAR(len, (LPCWSTR)buff, cp);
    if(!target)
      return 0;
    delete [](LPCWSTR)buff;
    }
  else
    target = (LPSTR)buff;
  return target;
}
//----------------------------------------------------------------------------
LPWSTR autoConvertW(LPVOID buff, uint& len, uint cp)
{
  LPWSTR target = 0;
  if(!IsTextUnicode(buff, len, 0)) {
    target = makeWCHAR(len, (LPCSTR)buff, cp);
    if(!target)
      return 0;
    delete [](LPCSTR)buff;
    }
  else {
    len /= sizeof(*target);
    target = (LPWSTR)buff;
    if(BOM_UTF_16_LE == *(LPWORD)buff || BOM_UTF_16_BE == *(LPWORD)buff) {
      memmove(buff, (LPWORD)buff + 1, len * sizeof(*target));
      --len;
      }
    else if(BOM_UTF_32_LE == *(LPDWORD)buff || BOM_UTF_32_BE == *(LPDWORD)buff) {
      memmove(buff, (LPDWORD)buff + 1, len * sizeof(*target));
      len -= 2;
      }
    }

  return target;
}
//----------------------------------------------------------------------------
manageRowOfBuff::manageRowOfBuff(LPCTSTR filename) : Buff(0), Len(0), autoDelete(true)
{
  Buff = openFile(filename, Len);
}
//----------------------------------------------------------------------------
#ifdef _UNICODE
//----------------------------------------------------------------------------
manageRowOfBuff::manageRowOfBuff(uint len, LPSTR buffer, bool autodelete) :
    Buff(0), Len(len), autoDelete(autodelete)
{
  Buff = makeWCHAR(Len, buffer);
  if(autodelete)
    delete []buffer;
  autoDelete = true;
}
//----------------------------------------------------------------------------
manageRowOfBuff::manageRowOfBuff(uint len, LPWSTR buffer, bool autodelete) :
    Buff(buffer), Len(len), autoDelete(autodelete)
{}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
manageRowOfBuff::manageRowOfBuff(uint len, LPWSTR buffer, bool autodelete) :
    Buff(0), Len(len), autoDelete(autodelete)
{
  Buff = unmakeWCHAR(Len, buffer);
  if(autodelete)
    delete []buffer;
  autoDelete = true;
}
//----------------------------------------------------------------------------
manageRowOfBuff::manageRowOfBuff(uint len, LPSTR buffer, bool autodelete) :
    Buff(buffer), Len(len), autoDelete(autodelete)
{}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
manageRowOfBuff::~manageRowOfBuff()
{
  if(autoDelete)
    delete []Buff;
}
//----------------------------------------------------------------------------
bool manageRowOfBuff::run()
{
  if(!Len || !Buff)
    return false;

  size_t offs = 0;
  size_t dim = Len;
  LPTSTR p = Buff;
  do {
    LPTSTR next = get_line(p, dim - offs);
    if(!manageLine(p))
      break;
    size_t len = next ? next - p : dim - offs;
    offs += len;
    p = next;
    } while(p);
  return true;
}
//-----------------------------------------------------------------
class setString : public genericSet
{
  public:
    setString() : id(0), msg(0) { }
    setString(long id, LPTSTR msg) : id(id), msg(msg) { }
//    ~setString() { delete []msg; }

    long id;
    LPTSTR msg;
};
//-----------------------------------------------------------------
typedef setString* pAString;
typedef const setString* pcAString;
//-----------------------------------------------------------------
bool operator >(const setString& a, const setString& b)
{
  return a.id > b.id;
}
//-----------------------------------------------------------------
bool operator <(const setString& a, const setString& b)
{
  return a.id < b.id;
}
//-----------------------------------------------------------------
bool operator ==(const setString& a, const setString& b)
{
  return a.id == b.id;
}
//-----------------------------------------------------------------
class Set4Msg : public P_Avl
{
  public:
    Set4Msg() : P_Avl(true) { }
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
    virtual void freeData(Node* a) {
      pAString p = reinterpret_cast<pAString>(a->D);
      delete []p->msg;
      delete p;
      }
};
//-----------------------------------------------------------------
int Set4Msg::Cmp(const TreeData toCheck, const TreeData current) const
{
  // si dovrebbe usare dynamic_cast per avere un maggior controllo,
  // ma poiché avviene tutto in questo file possiamo essere sicuri
  // che la classe può essere solo quella giusta
  pcAString check = reinterpret_cast<pcAString>(toCheck);
  pcAString curr = reinterpret_cast<pcAString>(current);
  if(*check < *curr)
    return -1;
  if(*check > *curr)
    return 1;
  return 0;
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// funzioni private
LPTSTR get_msg(LPTSTR p)
{
  while(_istdigit((unsigned)*p))
    ++p;
  if(*p)
    ++p;
  return p;
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
LPTSTR get_line(LPTSTR buff, size_t len)
{
  if((int)len < 0)
    return 0;
  size_t i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//------------------------------------------------------------------
LPTSTR translateToCRNL(LPTSTR dest, LPCTSTR srg)
{
  LPTSTR result = dest;
  for(; *srg; ++srg, ++dest) {
    if(*srg == _T('\\')) {
      ++srg;
      if(*srg == _T('n')) {
        *dest++ = _T('\r');
        *dest = _T('\n');
        }
      else if(*srg == _T('t'))
        *dest = _T('\t');
      else if(!*srg)
        break;
      else
        *dest = *srg;
      }
    else
      *dest = *srg;
    }
  *dest = 0;
  return result;
}
//----------------------------------------------------------
LPTSTR translateFromCRNL(LPTSTR dest, LPCTSTR srg)
{
  LPTSTR result = dest;
  bool found = false;
  for(; *srg; ++srg) {
    if(*srg == _T('\t')) {
      *dest++ = _T('\\');
      *dest++ = _T('t');
      }
    else if(*srg == _T('\r') || *srg == _T('\n')) {
      if(!found) {
        found = true;
        *dest++ = _T('\\');
        *dest++ = _T('n');
        }
      continue;
      }
    else if(*srg == _T('\\')) {
      *dest++ = _T('\\');
      *dest++ = _T('\\');
      }
    else
      *dest++ = *srg;
    found = false;
    }
  *dest = 0;
  return result;
}
//----------------------------------------------------------
LPSTR openFileA(LPCTSTR file, uint& dim)
{
  dim = 0;
  if(!file)
    return 0;
  LPSTR buff = 0;
  uint len = 0;
  do {
    P_File f(file, P_READ_ONLY);
    if(f.P_open()) {
      len = (uint)f.get_len();
      if(!len)
        return 0;
      dim = len;
      buff = new char[dim + 1];
      buff[dim - 1] = 0;
      buff[dim] = 0;
      f.P_read(buff, len);
      }
    } while(0);
  if(IsTextUnicode(buff, dim, 0)) {
    dim /= sizeof(wchar_t);
    LPSTR t = unmakeWCHAR(dim, (LPCWSTR)buff);
    delete []buff;
    buff = t;
    }
  return buff;
}
//----------------------------------------------------------
LPWSTR openFileW(LPCTSTR file, uint& dim)
{
  dim = 0;
  if(!file)
    return 0;
  LPWSTR buff = 0;
  uint len = 0;
  do {
    P_File f(file, P_READ_ONLY);
    if(f.P_open()) {
      len = (uint)f.get_len();
      dim = len / sizeof(*buff) + toBool(len % sizeof(*buff));
      if(!dim)
        return 0;
      buff = new wchar_t[dim + 1];
      buff[dim - 1] = 0;
      buff[dim] = 0;
      f.P_read(buff, len);
      }
    } while(0);
  if(!IsTextUnicode(buff, len, 0)) {
    dim *= sizeof(*buff);
    LPWSTR t = makeWCHAR(dim, (LPSTR)buff);
    delete []buff;
    buff = t;
    }
  else {
    if(BOM_UTF_16_LE == *(LPWORD)buff || BOM_UTF_16_BE == *(LPWORD)buff) {
      memmove(buff, (LPWORD)buff + 1, dim * sizeof(*buff));
      --dim;
      }
    else if(BOM_UTF_32_LE == *(LPDWORD)buff || BOM_UTF_32_BE == *(LPDWORD)buff) {
      memmove(buff, (LPDWORD)buff + 1, dim * sizeof(*buff));
      dim -= 2;
      }
    }
  return buff;
}
//----------------------------------------------------------
static void load_Set_gen(LPTSTR buff, size_t dim, Set4Msg& Set)
{
  size_t offs = 0;
  LPTSTR p = buff;
  do {
    LPTSTR next = get_line(p, dim - offs);
    long id = _ttol(p);
    size_t len = next ? next - p : dim - offs;
    offs += len;
    bool isValid = false;
    if(!id) {
      if(!cmpStr(p, _T("0000,"), 5))
        isValid = true;
      }
    if(id || isValid) {
      p = get_msg(p);
      if(!p) {
        static TCHAR tmp[50];
        wsprintf(tmp, _T("%d,???"), id);
        p = tmp;
        }
      size_t l = _tcslen(p);
      LPTSTR pt = translateToCRNL(new TCHAR[l + 1], p);
      setString set(id, 0);
      if(Set.Search(&set)) {
        setString *p = reinterpret_cast<setString*>(Set.getCurr());
        delete []p->msg;
        p->msg = pt;
        }
      else {
        setString *p = new setString(id, pt);
        Set.Add(p);
        }
      }
    p = next;
    } while(p);
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
class impl_set
{
  public:
    impl_set(LPCTSTR filename)
      {
        add(filename);
      }

    impl_set(uint len, LPTSTR buffer, bool autodelete)
      {
        add(len, buffer, autodelete);
      }
    ~impl_set() { reset(); }

    void reset() {
      criticalLock lck(CS);
      Data_Set.Flush();
      }

    LPCTSTR getString(long id)  {
      criticalLock lck(CS);
      setString set(id, 0);
      if(Data_Set.Search(&set)) {
        setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
        return p->msg;
        }
      return 0;
      }

    void add(LPCTSTR filename)  {
      criticalLock lck(CS);
      uint dim;
      LPTSTR buff = openFile(filename, dim);
      if(buff) {
        load_Set_gen(buff, dim, Data_Set);
        delete []buff;
        }
      }
    void add(uint len, LPTSTR buffer, bool autodelete)  {
      criticalLock lck(CS);

      LPTSTR b2 = buffer;
      if(!autodelete) {
        b2 = new TCHAR[len + 1];
        copyStr(b2, buffer, len);
        b2[len] = 0;
        }
      load_Set_gen(b2, len, Data_Set);
      delete []b2;
      }

    uint getNumElem() { return Data_Set.getNumElem(); }
    void saveTree(LPCTSTR filename);

    void replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld);

    bool setFirst() {  return Data_Set.setFirst();  }
    bool setLast()  {  return Data_Set.setLast();   }
    bool setNext()  {  return Data_Set.setNext();   }
    bool setPrev()  {  return Data_Set.setPrev();   }

    long getCurrId() const  {
      criticalLock lck(CS);
      setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
      if(p)
        return p->id;
      return 0;
      }

    LPCTSTR getCurrString() const {
      criticalLock lck(CS);
      setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
      if(p)
        return p->msg;
      return 0;
      }
    bool writeCurrent(P_File& pf, bool unicode = false) {
      LPTSTR buff;
      LPCTSTR curr = getCurrString();
      size_t len = curr ? _tcslen(curr) : 4;
      buff = new TCHAR[len + 20];
      wsprintf(buff, _T("%d,"), getCurrId());
      LPTSTR p = buff + _tcslen(buff);
      if(curr)
        translateFromCRNL(p, curr);
      p += _tcslen(p);
      _tcscpy_s(p, len + 20 - (p - buff), _T("\r\n"));
      bool success;
      if(unicode)
        success = toBool(pf.P_writeToUnicode(buff));
      else
        success = toBool(pf.P_writeToAnsi(buff));
      delete []buff;
      return success;
      }

  private:
    Set4Msg Data_Set;
    mutable criticalSect CS;
};
//-----------------------------------------------------------------
void impl_set::replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld)
{
  criticalLock lck(CS);
  setString set(id, 0);
  if(Data_Set.Search(&set)) {
    setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
    if(!newString) {
      if(deleteOld)
        delete [] p->msg;
      Data_Set.Rem();
      }
    else if(p->msg != newString) {
      if(deleteOld)
        delete [] p->msg;
      p->msg = newString;
      }
    }
  else if(addIfNotExist) {
    setString *set = new setString(id, newString);
    Data_Set.Add(set);
    }
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
static void save_tree(P_File& file, P_Tree::Node* node, int level, int offset)
{
  char buff[2000];
  setString *data = reinterpret_cast<setString*>(node->D);
  wsprintfA(buff, "Id=%5d - this=%6X - Par=%6X - Sx=%6X - Dx=%6X - level=%3d - offset=%3d - string=%s\r\n",
    data->id, node, node->Parent, node->Sx, node->Dx, level, offset, data->msg);
  file.P_write(buff, (lUDimF)strlen(buff));
}
//-----------------------------------------------------------------
void impl_set::saveTree(LPCTSTR filename)
{
  LPTSTR nfile = str_newdup(filename);
  nfile[_tcslen(nfile) - 1] = '_';
  P_File file(nfile, P_CREAT);
  if(file.P_open())
    Data_Set.exportTree(file, save_tree);
  delete []nfile;

}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
setOfString::setOfString(LPCTSTR filename) :
      ImplSet(new impl_set(filename)) {   }
//-----------------------------------------------------------------
#ifdef _UNICODE
setOfString::setOfString(uint len, LPWSTR buffer, bool autodelete) :
      ImplSet(new impl_set(len, buffer, autodelete)) {   }
//-----------------------------------------------------------------
setOfString::setOfString(uint len, LPSTR buffer, bool autodelete) :
      ImplSet(0)
{
  LPTSTR p = makeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
  ImplSet = new impl_set(len, p, autodelete);
}
#else
setOfString::setOfString(uint len, LPSTR buffer, bool autodelete) :
      ImplSet(new impl_set(len, buffer, autodelete)) {   }
//-----------------------------------------------------------------
setOfString::setOfString(uint len, LPWSTR buffer, bool autodelete) :
      ImplSet(0)
{
  LPSTR p = unmakeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
  ImplSet = new impl_set(len, p, autodelete);
}
#endif
//-----------------------------------------------------------------
setOfString::~setOfString()
{
  delete ImplSet;
}
//-----------------------------------------------------------------
LPCTSTR setOfString::getString(long id) const
{
  return ImplSet->getString(id);
}
//-----------------------------------------------------------------
void setOfString::add(LPCTSTR filename)  { ImplSet->add(filename); }
//-----------------------------------------------------------------
#ifdef _UNICODE
//-----------------------------------------------------------------
void setOfString::add(uint len, LPWSTR buffer, bool autodelete)
{ ImplSet->add(len, buffer, autodelete); }
//-----------------------------------------------------------------
void setOfString::add(uint len, LPSTR buffer, bool autodelete)
{
  LPWSTR p = makeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;

  ImplSet->add(len, p, autodelete);
}
//-----------------------------------------------------------------
#else
//-----------------------------------------------------------------
void setOfString::add(uint len, LPSTR buffer, bool autodelete)
{ ImplSet->add(len, buffer, autodelete); }
//-----------------------------------------------------------------
void setOfString::add(uint len, LPWSTR buffer, bool autodelete)
{
  LPSTR p = unmakeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;

  ImplSet->add(len, p, autodelete);
}
//-----------------------------------------------------------------
#endif
//-----------------------------------------------------------------
//-----------------------------------------------------------------
uint setOfString::getNumElem()
{
  return ImplSet->getNumElem();
}
//-----------------------------------------------------------------
#define DEF_RESULT _T("Msg%d")
//-----------------------------------------------------------------
LPCTSTR setOfString::getStringOrId(long id) const
{
  static TCHAR def[SIZE_A_c(DEF_RESULT) + 12];
  LPCTSTR result = getString(id);
  if(!result) {
    wsprintf(def, DEF_RESULT, id);
    return def;
    }
  return result;
}
//-----------------------------------------------------------------
void setOfString::replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld)
{
  ImplSet->replaceString(id, newString, addIfNotExist, deleteOld);
}
//-----------------------------------------------------------------
void setOfString::saveTree(LPCTSTR filename)
{
  ImplSet->saveTree(filename);
}
//-----------------------------------------------------------------
bool setOfString::setFirst()
{
  return ImplSet->setFirst();
}
//-----------------------------------------------------------------
bool setOfString::setLast()
{
  return ImplSet->setLast();
}
//-----------------------------------------------------------------
bool setOfString::setNext()
{
  return ImplSet->setNext();
}
//-----------------------------------------------------------------
bool setOfString::setPrev()
{
  return ImplSet->setPrev();
}
//-----------------------------------------------------------------
long setOfString::getCurrId() const
{
  return ImplSet->getCurrId();
}
//-----------------------------------------------------------------
LPCTSTR setOfString::getCurrString() const
{
  return ImplSet->getCurrString();
}
//-----------------------------------------------------------------
void setOfString::reset()
{
  ImplSet->reset();
}
//-----------------------------------------------------------------
bool setOfString::writeCurrent(P_File& pf, bool unicode)
{
  return ImplSet->writeCurrent(pf, unicode);
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
static bool isClosed(LPCTSTR p)
{
  while(*p > _T('\n')) {
    if(_T(']') == *p)
      return true;
    ++p;
    }
  return false;
}
//----------------------------------------------------------
static void load_SetInfo_gen(LPTSTR buff, uint dim, Set4Msg& Set)
{
  size_t offs = 0;
  LPTSTR p = buff;
  TCHAR tmp[5000];
  do {
    LPTSTR next = get_line(p, dim - offs);
    size_t len = next ? next - p : dim - offs;
    offs += len;
    if(_T('[') == *p) {
      long id = _ttol(p + 1);
      if(id && isClosed(p)) {
        size_t ix = 0;
        while(next) {
          if(_T('[') == *next)
            break;
          p = next;
          next = get_line(p, dim - offs);
          len = next ? next - p : dim - offs;
          offs += len;
          if(_T('#') != *p) {
            size_t l = _tcslen(p);
            if(ix + l > SIZE_A(tmp))
              break;
            copyStr(tmp + ix, p, l);
            ix += l;
            if(ix + 2 > SIZE_A(tmp))
              break;
            copyStr(tmp + ix, _T("\r\n"), 2);
            ix += 2;
            if(ix >= SIZE_A(tmp))
              break;
            }
          }
        TCHAR* pt = new TCHAR[ix + 1];
        copyStr(pt, tmp, ix + 1);
        pt[ix] = 0;
        setString set(id, 0);
        if(Set.Search(&set)) {
          setString *p = reinterpret_cast<setString*>(Set.getCurr());
          delete []p->msg;
          p->msg = pt;
          }
        else {
          setString *p = new setString(id, pt);
          Set.Add(p);
          }
        }
      }
    p = next;
    } while(p);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
class impl_blockSet
{
  public:
    impl_blockSet(LPCTSTR filename)
      {
        add(filename);
      }

    impl_blockSet(uint len, LPTSTR buffer, bool autodelete)
      {
        add(len, buffer, autodelete);
      }

    void reset() {
      criticalLock lck(CS);
      Data_Set.Flush();
      }

    LPCTSTR getString(long id)  {
      criticalLock lck(CS);
      setString set(id, 0);
      if(Data_Set.Search(&set)) {
        setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
        return p->msg;
        }
      return 0;
      }

    void add(LPCTSTR filename)  {
      criticalLock lck(CS);
      uint dim;
      LPTSTR buff = openFile(filename, dim);
      if(buff) {
        load_SetInfo_gen(buff, dim, Data_Set);
        delete []buff;
        }
      }
    void add(uint len, LPTSTR buffer, bool autodelete)  {
      criticalLock lck(CS);

      LPTSTR b2 = buffer;
      if(!autodelete) {
        b2 = new TCHAR[len + 1];
        copyStr(b2, buffer, len);
        b2[len] = 0;
        }
      load_SetInfo_gen(b2, len, Data_Set);
      delete []b2;
      }

    uint getNumElem() { return Data_Set.getNumElem(); }

    void replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld) {
      criticalLock lck(CS);
      setString set(id, 0);
      if(Data_Set.Search(&set)) {
        setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
        if(!newString) {
          if(deleteOld)
            delete [] p->msg;
          Data_Set.Rem();
          }
        else if(p->msg != newString) {
          if(deleteOld)
            delete [] p->msg;
          p->msg = newString;
          }
        }
      else if(addIfNotExist) {
        setString *set = new setString(id, newString);
        Data_Set.Add(set);
        }
      }

    bool setFirst() {  return Data_Set.setFirst();  }
    bool setLast()  {  return Data_Set.setLast();   }
    bool setNext()  {  return Data_Set.setNext();   }
    bool setPrev()  {  return Data_Set.setPrev();   }

    long getCurrId() const  {
      criticalLock lck(CS);
      setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
      if(p)
        return p->id;
      return 0;
      }

    LPCTSTR getCurrString() const {
      criticalLock lck(CS);
      setString *p = reinterpret_cast<setString*>(Data_Set.getCurr());
      if(p)
        return p->msg;
      return 0;
      }
    bool writeCurrent(P_File& pf, bool unicode = false) {
      TCHAR buff[50];
      wsprintf(buff, _T("[%d]\r\n"), getCurrId());
      if(unicode) {
        if(!pf.P_writeToUnicode(buff))
          return false;
        LPCTSTR curr = getCurrString();
        if(!curr)
          return toBool(pf.P_writeToUnicode(_T("\r\n")));
        if(pf.P_writeToUnicode(curr))
          return toBool(pf.P_writeToUnicode(_T("\r\n")));
        }
      else {
        if(!pf.P_writeToAnsi(buff))
          return false;
        LPCTSTR curr = getCurrString();
        if(!curr)
          return toBool(pf.P_writeToAnsi(_T("\r\n")));
        if(pf.P_writeToAnsi(curr))
          return toBool(pf.P_writeToAnsi(_T("\r\n")));
        }
      return false;
      }

  private:
    Set4Msg Data_Set;
    mutable criticalSect CS;
};
//-----------------------------------------------------------------
//-----------------------------------------------------------------
setOfBlockString::setOfBlockString(LPCTSTR filename) :
      ImplSet(new impl_blockSet(filename)) {   }
//-----------------------------------------------------------------
#ifdef _UNICODE
setOfBlockString::setOfBlockString(uint len, LPWSTR buffer, bool autodelete) :
      ImplSet(new impl_blockSet(len, buffer, autodelete)) {   }
//-----------------------------------------------------------------
setOfBlockString::setOfBlockString(uint len, LPSTR buffer, bool autodelete) :
      ImplSet(0)
{
  LPWSTR p = makeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
  ImplSet = new impl_blockSet(len, p, autodelete);
}
#else
setOfBlockString::setOfBlockString(uint len, LPSTR buffer, bool autodelete) :
      ImplSet(new impl_blockSet(len, buffer, autodelete)) {   }
//-----------------------------------------------------------------
setOfBlockString::setOfBlockString(uint len, LPWSTR buffer, bool autodelete) :
      ImplSet(0)
{
  LPSTR p = unmakeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
  ImplSet = new impl_blockSet(len, p, autodelete);
}
#endif
//-----------------------------------------------------------------
setOfBlockString::~setOfBlockString()
{
  delete ImplSet;
}
//-----------------------------------------------------------------
LPCTSTR setOfBlockString::getString(long id) const
{
  return ImplSet->getString(id);
}
//-----------------------------------------------------------------
void setOfBlockString::add(LPCTSTR filename)  { ImplSet->add(filename); }
//-----------------------------------------------------------------
#ifdef _UNICODE
//-----------------------------------------------------------------
void setOfBlockString::add(uint len, LPWSTR buffer, bool autodelete)
{ ImplSet->add(len, buffer, autodelete); }
//-----------------------------------------------------------------
void setOfBlockString::add(uint len, LPSTR buffer, bool autodelete)
{
  LPTSTR p = makeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
}
//-----------------------------------------------------------------
#else
//-----------------------------------------------------------------
void setOfBlockString::add(uint len, LPSTR buffer, bool autodelete)
{ ImplSet->add(len, buffer, autodelete); }
//-----------------------------------------------------------------
void setOfBlockString::add(uint len, LPWSTR buffer, bool autodelete)
{
  LPTSTR p = unmakeWCHAR(len, buffer);
  if(autodelete)
    delete []buffer;
  autodelete = true;
}
//-----------------------------------------------------------------
#endif
//-----------------------------------------------------------------
uint setOfBlockString::getNumElem()
{
  return ImplSet->getNumElem();
}
//-----------------------------------------------------------------
#define DEF_RESULT _T("Msg%d")
//-----------------------------------------------------------------
LPCTSTR setOfBlockString::getStringOrId(long id) const
{
  static TCHAR def[SIZE_A_c(DEF_RESULT) + 12];
  LPCTSTR result = getString(id);
  if(!result) {
    wsprintf(def, DEF_RESULT, id);
    return def;
    }
  return result;
}
//-----------------------------------------------------------------
void setOfBlockString::replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld)
{
  ImplSet->replaceString(id, newString, addIfNotExist, deleteOld);
}
//-----------------------------------------------------------------
bool setOfBlockString::setFirst()
{
  return ImplSet->setFirst();
}
//-----------------------------------------------------------------
bool setOfBlockString::setLast()
{
  return ImplSet->setLast();
}
//-----------------------------------------------------------------
bool setOfBlockString::setNext()
{
  return ImplSet->setNext();
}
//-----------------------------------------------------------------
bool setOfBlockString::setPrev()
{
  return ImplSet->setPrev();
}
//-----------------------------------------------------------------
long setOfBlockString::getCurrId() const
{
  return ImplSet->getCurrId();
}
//-----------------------------------------------------------------
LPCTSTR setOfBlockString::getCurrString() const
{
  return ImplSet->getCurrString();
}
//-----------------------------------------------------------------
void setOfBlockString::reset()
{
  ImplSet->reset();
}
//-----------------------------------------------------------------
bool setOfBlockString::writeCurrent(P_File& pf, bool unicode)
{
  return ImplSet->writeCurrent(pf, unicode);
}
//-----------------------------------------------------------------
