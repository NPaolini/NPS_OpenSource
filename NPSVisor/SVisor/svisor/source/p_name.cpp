//---------- p_name.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "pdef.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_name.h"
#include "def_dir.h"
#include "p_file.h"
#include "language.h"
#include "mainClient.h"
#include "p_avl.h"
//----------------------------------------------------------------------------
// offset per gruppo allarmi per periferica
// eliminato, ora l'allarme si  trova nei 16bit inferiori e la prph nei superiori
//#define MUL_STEP_PRF_ALARM  4000
//----------------------------------------------------------------------------
#define FILE_DAT_TXT_STD _T("#base_data") PAGE_EXT
#define FILE_DAT_GLOBAL_TXT _T("globalPageText") PAGE_EXT
#define FILE_DAT_ALARM _T("alarm")
//----------------------------------------------------------------------------
#define DEF_NAME_INFO_ALARM _T("infoAlarm")
//----------------------------------------------------------------------------
// ora sono 9, è stata aggiunta la prph_1
#define MAX_PERIF_ALARM 9
//----------------------------------------------------------------------------
static setOfString strMsg;
static setOfString strAlarm[MAX_PERIF_ALARM];
static setOfString strGlobalMsg;
//----------------------------------------------------------------------------
static bool useCodeFilter;
//----------------------------------------------------------------------------
class infoFileAlarmSet : public genericSet
{
  public:
    LPCTSTR file;
    setOfString str;
    infoFileAlarmSet() : file(0) {}
    infoFileAlarmSet(LPCTSTR file) : file(file)  { }
};
//----------------------------------------------------------------------------
class infoFileAlarmAvl : public P_Avl
{
  public:
    infoFileAlarmAvl(bool autodelete) : P_Avl(autodelete) { }
    infoFileAlarmSet* getCurrData() const { return reinterpret_cast<infoFileAlarmSet*>(getCurr()); }
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const
    {
      infoFileAlarmSet* ifa1 = (infoFileAlarmSet*)toCheck;
      if(!ifa1->file)
        return -1;
      infoFileAlarmSet* ifa2 = (infoFileAlarmSet*)current;
      if(!ifa2->file)
        return 1;
      return _tcsicmp(ifa1->file, ifa2->file);
    }
    virtual void freeData(Node* a)
    {
      infoFileAlarmSet* ifa = (infoFileAlarmSet*)a->D;
      delete []ifa->file;
    }
};
//----------------------------------------------------------------------------
static infoFileAlarmAvl iFileAlarmAvl(true);
//----------------------------------------------------------------------------
static setOfString& getSetFileAlarm(LPCTSTR file)
{
  infoFileAlarmSet ifa(file);
  if(iFileAlarmAvl.Search(&ifa)) {
    infoFileAlarmSet* pSet = iFileAlarmAvl.getCurrData();
    return pSet->str;
    }
  infoFileAlarmSet* pIfa = new infoFileAlarmSet(str_newdup(file));

  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, file);
  _tcscat_s(t, _T(".txa"));
//  _tcscat_s(t, _T(".txt"));
  checkSystemPathTxt(t, t);
  pIfa->str.add(t);
  iFileAlarmAvl.Add(pIfa);
  return pIfa->str;
}
//----------------------------------------------------------------------------
static LPCTSTR getStringByLangOrValue(const setOfString& sos, uint id)
{
  static TCHAR sBuff[64];
  smartPointerConstString res = getStringByLang(sos, id + 1);
  if((LPCTSTR)res)
    return res;
  wsprintf(sBuff, _T("%d"), id);
  return sBuff;
}
//----------------------------------------------------------------------------
void infoAssocAlarm::clone(const infoAssocAlarm& other)
{
  if(tSTR == type && tSTR == other.type && str == other.str)
    return;

  if(needDelete)
    delete []str;

  type = other.type;
  nDec = other.nDec;

  if(other.needDelete) {
    str = str_newdup(other.str);
    needDelete = true;
    type = tSTR;
    }
  else {
    str = other.str;
    needDelete = false;
    }
}
//----------------------------------------------------------------------------
void infoAssocAlarm::copy(const infoAssocAlarm& other)
{
  if(tSTR == type && tSTR == other.type && str == other.str)
    return;

  if(needDelete)
    delete []str;

  str = other.str;
  type = other.type;
  nDec = other.nDec;
  dw = other.dw;
  needDelete = false;
}
//----------------------------------------------------------------------------
void infoAssocAlarm::move(infoAssocAlarm& other)
{
  if(tSTR == type && tSTR == other.type && str == other.str)
    return;

  if(needDelete)
    delete []str;

  type = other.type;
  nDec = other.nDec;
  needDelete = other.needDelete;
  str = other.str;
  other.needDelete = false;
  other.str = 0;
  other.type = NO_ASSOC;
}
//----------------------------------------------------------------------------
#define FNZ(fnz, v)  if(sizeof(v) != pf.fnz(&v, sizeof(v))) return false
#define FNZ_WRT(v)  FNZ(P_write, v)
#define FNZ_READ(v)  FNZ(P_read, v)
//----------------------------------------------------------------------------
bool infoAssocAlarm::save(P_File& pf)
{
  FNZ_WRT(dw);
  FNZ_WRT(type);
  FNZ_WRT(nDec);
  uint dim = 0;
  if(needDelete)
    dim = (_tcslen(str) + 1) * sizeof(TCHAR);
  FNZ_WRT(dim);
  if(dim)
    if(dim != pf.P_write(str, dim))
      return false;
  return true;
}
//----------------------------------------------------------------------------
bool infoAssocAlarm::load(P_File& pf)
{
  FNZ_READ(dw);
  FNZ_READ(type);
  FNZ_READ(nDec);
  uint dim = 0;
  FNZ_READ(dim);
  if(dim) {
    needDelete = true;
    LPTSTR str2 = new TCHAR[dim / sizeof(TCHAR)];
    if(dim != pf.P_read(str2, dim)) {
      delete []str2;
      return false;
      }
    str = str2;
    }
  return true;
}
//----------------------------------------------------------------------------
void fullInfoAlarm::clone(const fullInfoAlarm& other)
{
  iaa.clone(other.iaa);
  descr = smartPointerConstString(str_newdup(other.descr), true);
  CLONE(alarm);
  CLONE(prph);
  CLONE(idGrp);
  CLONE(idFlt);
}
//----------------------------------------------------------------------------
void fullInfoAlarm::copy(const fullInfoAlarm& other)
{
  iaa.copy(other.iaa);
  descr = smartPointerConstString(other.descr, false);
  CLONE(alarm);
  CLONE(prph);
  CLONE(idGrp);
  CLONE(idFlt);
}
//----------------------------------------------------------------------------
void fullInfoAlarm::move(fullInfoAlarm& other)
{
  iaa.move(other.iaa);
  descr = other.descr;
  CLONE(alarm);
  CLONE(prph);
  CLONE(idGrp);
  CLONE(idFlt);
}
//----------------------------------------------------------------------------
long fullInfoAlarm::makeFullIdAlarm() const
{
  return makeIdAlarmByPrph(alarm, prph + FIRST_PRPH_ALARM);
}
//----------------------------------------------------------------------------
void fullInfoAlarm::splitId(long ix)
{
  splitIdAlarmByPrph(ix, alarm, prph);
}
//----------------------------------------------------------------------------
bool fullInfoAlarm::save(P_File& pf)
{
  FNZ_WRT(alarm);
  FNZ_WRT(prph);
  FNZ_WRT(idGrp);
  FNZ_WRT(idFlt);
  uint dim = 0;
  if(descr)
    dim = (_tcslen(descr) + 1) * sizeof(TCHAR);
  FNZ_WRT(dim);
  if(dim)
    if(dim != pf.P_write(&descr, dim))
      return false;
  return iaa.save(pf);
}
//----------------------------------------------------------------------------
bool fullInfoAlarm::load(P_File& pf)
{
  FNZ_READ(alarm);
  FNZ_READ(prph);
  FNZ_READ(idGrp);
  FNZ_READ(idFlt);
  uint dim = 0;
  FNZ_READ(dim);
  if(dim) {
    LPTSTR str = new TCHAR[dim / sizeof(TCHAR)];
    if(dim != pf.P_read(str, dim))
      return false;
    descr = smartPointerConstString(str, true);
    }
  return iaa.load(pf);
}
//----------------------------------------------------------------------------
bool is_digit(LPCTSTR p)
{
  if(!p || !*p)
    return false;
  bool hasDigit = false;
  while(true) {
    bool isDgt = toBool(_istdigit((unsigned)*p));
    hasDigit |= isDgt;
    if(!isDgt && _T(' ') != *p && _T('\t') != *p)
      break;
    ++p;
    }
  return hasDigit && _T(',') == *p;
}
//----------------------------------------------------------------------------
void checkSystemPathTxt(LPTSTR target, LPCTSTR name)
{
  TCHAR path[_MAX_PATH] = _T("system\\");
  _tcscat_s(path, SIZE_A(path), name);
  if(P_File::P_exist(path))
    _tcscpy_s(target, _MAX_PATH, path);
  else
    _tcscpy_s(target, _MAX_PATH, name);
}
//----------------------------------------------------------------------------
// funzioni visibili esternamente
void load_Txt()
{
  load_Txt(strMsg, FILE_DAT_TXT_STD);

  // per test, per verificare l'albero
//  strMsg.saveTree(path);
}
//----------------------------------------------------------------------------
void load_Global_Txt()
{
  extern void getFileStrCheckRemote(LPTSTR target, LPCTSTR file);

  TCHAR path[_MAX_PATH];
  getFileStrCheckRemote(path, FILE_DAT_GLOBAL_TXT);
  strGlobalMsg.add(path);
}
//----------------------------------------------------------------------------
void load_Txt(setOfString& set, LPCTSTR name)
{
  if(!name || !*name)
    return;
  TCHAR path[_MAX_PATH];
  checkSystemPathTxt(path, name);
  set.add(path);
  checkSystemPathTxt(path, name + 1);
  set.add(path);
}
//----------------------------------------------------------------------------
void resetTxt()
{
  strMsg.reset();
  strGlobalMsg.reset();
}
//----------------------------------------------------------------------------
void resetAlarmTxt()
{
  for(uint i = 0; i < SIZE_A(strAlarm); ++i)
    strAlarm[i].reset();
  iFileAlarmAvl.Flush();
}
//----------------------------------------------------------------------------
// il codice per gli allarmi di periferica uno sono stati messi in coda, quindi
// occorre aggiungere uno al nome del file e fare il modulo
//----------------------------------------------------------------------------
void load_Alarm()
{
  for(int i = 0; i < MAX_PERIF_ALARM; ++i) {
    LPCTSTR p = strMsg.getString(ID_MANAGE_ALARM_PLC + i);
    if(!p)
      continue;
    TCHAR name[50];
    int trueId = (i + FIRST_PRPH_ALARM) % MAX_PERIF_ALARM + 1;
    wsprintf(name, _T("#%s_%d") PAGE_EXT, FILE_DAT_ALARM, trueId);
    load_Txt(strAlarm[trueId - 1], name);
    }
}
//----------------------------------------------------------------------------
void allocInfoAlarm(PVect<setOfBlockString*>& target)
{
  int currLang = getCurrLang();

  TCHAR path[_MAX_PATH];
  for(int i = 0; i < MAX_PERIF_ALARM; ++i) {
    TCHAR name[64];
    if(currLang)
      wsprintf(name, _T("%s_%d_%d") PAGE_EXT, DEF_NAME_INFO_ALARM, i + FIRST_PRPH_ALARM, currLang);
    else
      wsprintf(name, _T("%s_%d") PAGE_EXT, DEF_NAME_INFO_ALARM, i + FIRST_PRPH_ALARM);
    checkSystemPathTxt(path, name);
    // se non esiste il file ed è impostata una lingua, prova senza codice di lingua
    if(!P_File::P_exist(path) && currLang) {
      wsprintf(name, _T("%s_%d") PAGE_EXT, DEF_NAME_INFO_ALARM, i + FIRST_PRPH_ALARM);
      checkSystemPathTxt(path, name);
      }
//    LPCTSTR p = strMsg.getString(ID_MANAGE_ALARM_PLC + i);
//    if(!p)
//      path[0] = 0;
    target[i] = new setOfBlockString(path);
    }
}
//----------------------------------------------------------------------------
void checkIfUseFilterCode()
{
  const DWORD offsAssoc = 100000;
  const DWORD offsText  = 200000;
  const DWORD firstIdAssocText = 300001;
  useCodeFilter = false;
  for(int i = 0; i < MAX_PERIF_ALARM; ++i) {
    if(strAlarm[i].setFirst()) {
      do {
        if(strAlarm[i].getCurrId() >= offsAssoc)
          break;
        LPCTSTR p = strAlarm[i].getCurrString();
        p = findNextParam(p, 1);
        if(!is_digit(p))
          return;
        } while(strAlarm[i].setNext());
      }
    }
  useCodeFilter = true;
}
//----------------------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key)
{
  TCHAR path[_MAX_PATH];
  getFileStrCheckRemote(path, FILE_DAT_GLOBAL_TXT);
  strGlobalMsg.add(path);

  return load_CriptedTxt(key, strMsg, FILE_DAT_TXT_STD);
}
//----------------------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name)
{
  infoFileCr result;
  result.header = (LPCBYTE)CRYPT_HEAD;
  result.lenHeader = DIM_HEAD;

  TCHAR path[_MAX_PATH];
  checkSystemPathTxt(path, name);

  if(decryptFile(path, key, DIM_KEY_CRYPT, CRYPT_STEP, result)) {
    uint dim = result.dim;
    LPTSTR res = autoConvert(result.buff, dim);
    set.add(dim, res, true);
//    if(!IsTextUnicode(result.buff, result.dim, 0))
//      set.add(result.dim, (LPSTR)result.buff, true);
//    else
//      set.add(result.dim / sizeof(wchar_t), (LPTSTR)result.buff, true);
    }
  checkSystemPathTxt(path, name + 1);
  set.add(path);
  return result.crc;
}
//----------------------------------------------------------------------------
DWORD load_CriptedAlarm(LPCBYTE key)
{
  for(int i = 0; i < MAX_PERIF_ALARM; ++i) {
    TCHAR name[50];
    int trueId = (i + FIRST_PRPH_ALARM) % MAX_PERIF_ALARM + 1;
    wsprintf(name, _T("#%s_%d") PAGE_EXT, FILE_DAT_ALARM, trueId);
    load_CriptedTxt(key, strAlarm[trueId - 1], name);
    }
  return 1;
}
//----------------------------------------------------------------------------
LPCTSTR getString(long id)
{
  return strMsg.getString(id);
}
//----------------------------------------------------------------------------
LPCTSTR getGlobalPageString(long id)
{
  return strGlobalMsg.getString(id);
}
//----------------------------------------------------------------------------
#define DEF_STR_MSG _T("msg_%d")
//----------------------------------------------------------------------------
LPCTSTR getStringOrId(long id)
{
  return strMsg.getStringOrId(id);
}
//----------------------------------------------------------------------------
void replaceString(long id, LPTSTR newString, bool addIfNotExist, bool deleteOld)
{
  strMsg.replaceString(id, newString, addIfNotExist, deleteOld);
}
//----------------------------------------------------------------------------
LPCTSTR getStringAlarm(long id)
{
  uint alarm;
  uint prph;
  splitIdAlarmByPrph(id, alarm, prph);
  LPCTSTR p = strAlarm[prph].getString(alarm);
  if(p && useCodeFilter)
    p = findNextParam(p, 1);
  return p;
}
//----------------------------------------------------------------------------
#define DEF_STR_ALARM _T("0,Alarm (or Event) n° %d not found")
//----------------------------------------------------------------------------
LPCTSTR getStringOrIdAlarm(long id)
{
  static TCHAR buff[SIZE_A(DEF_STR_ALARM) + 12];
  LPCTSTR ret = getStringAlarm(id);
  if(!ret) {
    wsprintf(buff, DEF_STR_ALARM, id);
    return buff;
    }
  return ret;
}
//----------------------------------------------------------------------------
long getAlarmFilterCode(long id)
{
  bool old = useCodeFilter;
  useCodeFilter = false;
  LPCTSTR p = getStringAlarm(id);
  useCodeFilter = old;
  if(p)
    return _ttoi(p);
  return -1;
}
//----------------------------------------------------------------------------
bool useMultipleAlarmFile()
{
  bool found = false;
  for(int i = 0; i < MAX_PERIF_ALARM; ++i) {
    if(strAlarm[i].getNumElem()) {
      if(found)
        return true;
      found = true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool useAlarmSection()
{
  return useCodeFilter;
}
//----------------------------------------------------------------------------
LPCTSTR getAlarmFullRow(long id)
{
  bool old = useCodeFilter;
  useCodeFilter = false;
  LPCTSTR p = getStringAlarm(id);
  useCodeFilter = old;
  return p;
}
//----------------------------------------------------------------------------
uint getPrphFromAlarm(uint id, uint& newId)
{
  newId = id;
  if(!id)
    return 0;
  uint idPrph;
  splitIdAlarmByPrph(id, newId, idPrph);
  return idPrph + FIRST_PRPH_ALARM;
}
//----------------------------------------------------------------------------
uint makeIdAlarmByPrph(uint id, uint prph)
{
  return id + ((prph - FIRST_PRPH_ALARM) << 16);
}
//------------------------------------------------------------------------
#define BIT_4_ACK_ALARM (1 << (14 + 16))
//------------------------------------------------------------------------
#define MASK_USED_BIT_4_ALARM ((1 << (8 + 16)) - 1)
//------------------------------------------------------------------------
bool isAckAlarm(long& id, bool clearBit)
{
  if(id & BIT_4_ACK_ALARM) {
    if(clearBit)
      id &= ~BIT_4_ACK_ALARM;
    return true;
    }
  return false;
}
//------------------------------------------------------------------------
void setAckAlarm(long& id)
{
  id |= BIT_4_ACK_ALARM;
}
//------------------------------------------------------------------------
void clearUnusedBitAlarm(long& id)
{
  id &= MASK_USED_BIT_4_ALARM;
}
//------------------------------------------------------------------------
template <typename T, typename D>
void splitIdAlarmByPrph_(long id, T& alarm, D& prph)
{
  if(id < 0)
    id = -id;
  alarm = id & 0xffff;
  prph = id >> 16;
  if(prph >= MAX_PERIF_ALARM)
    prph = 0;
}
//------------------------------------------------------------------------
void splitIdAlarmByPrph(long id, uint& alarm, uint& prph)
{
  splitIdAlarmByPrph_<uint, uint>(id, alarm, prph);
}
//------------------------------------------------------------------------
void splitIdAlarmByPrph(long id, int& alarm, int& prph)
{
  splitIdAlarmByPrph_<int, int>(id, alarm, prph);
}
//------------------------------------------------------------------------
LPCTSTR getInt(LPCTSTR p, DWORD& v)
{
  if(p) {
    v = _ttoi(p);
    p = findNextParamTrim(p);
    }
  return p;
}
//----------------------------------------------------------------------------
#define ID_ADD_ASSOC 100000
//----------------------------------------------------------------------------
void getNumDec4floatAlarm(fullInfoAlarm& fia)
{
  LPCTSTR p = strAlarm[fia.prph].getString(fia.alarm + ID_ADD_ASSOC);
  if(!p)
    return;

  DWORD prph = 0;
  DWORD addr = 0;
  DWORD type = 0;
  DWORD nBit = 0;

  p = getInt(p, prph);
  p = getInt(p, addr);
  p = getInt(p, type);
  p = getInt(p, nBit);
  fia.iaa.nDec = nBit;
}
//----------------------------------------------------------------------------
bool getTextAlarm(long id, fullInfoAlarm& fia, bool onlyValue)
{
  fia.splitId(id);
  LPCTSTR p = strAlarm[fia.prph].getString(fia.alarm);
  if(!p)
    return false;

  LPCTSTR pGrp = findNextParamTrim(p);
  LPCTSTR pIdDescr = findNextParamTrim(pGrp);

  if(!pIdDescr) { // vecchio modello senza filtro
    fia.idGrp = _ttoi(p);
    if(pGrp)
      fia.descr = getStringByLangSimple(pGrp);
    return true;
    }

  fia.idFlt = _ttoi(p);
  fia.idGrp = _ttoi(pGrp);

  uint idDescr = _ttoi(pIdDescr);
  if(!idDescr)
    fia.descr = getStringByLangSimple(pIdDescr);
  else
    fia.descr = getStringByLang(strAlarm[fia.prph], idDescr);

  p = strAlarm[fia.prph].getString(fia.alarm + ID_ADD_ASSOC);
  if(!p)
    return true;

  DWORD prph = 0;
  DWORD addr = 0;
  DWORD type = 0;
  DWORD nBit = 0;
  DWORD offset = 0;
  DWORD ixText = 0;

  p = getInt(p, prph);
  p = getInt(p, addr);
  p = getInt(p, type);
  p = getInt(p, nBit);
  p = getInt(p, offset);

  bool isNewMan = false;
  bool typeBit = false;
  if(p) {
    switch(p[0]) {
      case 'B':
      case 'b':
        typeBit = true;
      case 'N':
      case 'n':
//      case 'V': // in caso di valore equivale ad usare la versione precedente
//      case 'v':
        isNewMan = true;
        break;
      }
    }
  p = getInt(p, ixText);
  LPCTSTR fileNewMan = p;

  mainClient* mc = getMain();
  const genericPerif* pPrph = mc->getGenPerif(prph);
  if(!pPrph)
    return true;
  prfData data;
  data.lAddr = addr;
  data.typeVar = type;
  if(prfData::tBitData == type)
    data.U.dw = MAKELONG(nBit, offset);

  prfData::tResultData result = pPrph->get(data);

  if(prfData::failed >= result)
    return false;

  switch(type) {
    case prfData::tBitData:
    case prfData::tBData:
    case prfData::tWData:
    case prfData::tDWData:
      fia.iaa.type = infoAssocAlarm::tDW;
      break;
    case prfData::tFRData:
    case prfData::tRData:
      fia.iaa.type = infoAssocAlarm::tFLT;
      break;
    default:
      return false;
    }
  fia.iaa.nDec = nBit;
  if(onlyValue || !(ixText || isNewMan)) {
    fia.iaa.dw = data.U.dw;
    if(ixText) {
      fia.iaa.type = infoAssocAlarm::tDW4IX;
      fia.iaa.dw += ixText;
      }
    return true;
    }
  if(prfData::tBitData != type && fia.iaa.type != infoAssocAlarm::tFLT) {
    if(nBit || offset) {
      data.U.dw >>= offset;
      data.U.dw &= (1 << nBit) - 1;
      }
    }
  fia.iaa.type = infoAssocAlarm::tSTR;
  if(fia.iaa.str && fia.iaa.needDelete)
    delete []fia.iaa.str;
  if(isNewMan && fileNewMan) {
    uint ix = data.U.dw;
    bool foundMoreBit = false;
    if(typeBit) {
      if(!data.U.dw)
        ix = 0;
      else {
        DWORD dw = data.U.dw;
        for(ix = 1; ix <= 32; ++ix, dw >>= 1) {
          if(dw & 1) {
            if(dw & ~1) { // se c'è più di un bit attivo
              ix = data.U.dw;
              foundMoreBit = true;
              }
            break;
            }
          }
        }
      }
    if(foundMoreBit) {
      TCHAR t[64];
      wsprintf(t, _T("%d"), ix);
      fia.iaa.str = str_newdup(t);
      }
    else
      fia.iaa.str = str_newdup(getStringByLangOrValue(getSetFileAlarm(fileNewMan), ix));
    }
  else {
    ixText += data.U.dw;
    fia.iaa.str = str_newdup(getStringByLang(strAlarm[fia.prph], ixText));
    }
  fia.iaa.needDelete = true;
  return true;
}
//----------------------------------------------------------------------------
bool getTextReport(long id, fullInfoAlarm& fia, const infoAssocAlarm& info)
{
  fia.splitId(id);
  LPCTSTR p = strAlarm[fia.prph].getString(fia.alarm);
  if(!p)
    return false;

  LPCTSTR pGrp = findNextParamTrim(p);
  LPCTSTR pIdDescr = findNextParamTrim(pGrp);

  if(!pIdDescr) { // vecchio modello senza filtro
    fia.idGrp = _ttoi(p);
    if(pGrp)
      fia.descr = getStringByLangSimple(pGrp);
    return true;
    }

  fia.idFlt = _ttoi(p);
  fia.idGrp = _ttoi(pGrp);

  uint idDescr = _ttoi(pIdDescr);
  if(!idDescr)
    fia.descr = getStringByLangSimple(pIdDescr);
  else
    fia.descr = getStringByLang(strAlarm[fia.prph], idDescr);
  fia.iaa.clone(info);

  if(infoAssocAlarm::tFLT == fia.iaa.type)
    getNumDec4floatAlarm(fia);

  p = strAlarm[fia.prph].getString(fia.alarm + ID_ADD_ASSOC);
  if(p) {
    p = findNextParamTrim(p, 5);
    bool isNewMan = false;
    bool typeBit = false;
    if(p) {
      switch(p[0]) {
        case 'B':
        case 'b':
          typeBit = true;
        case 'N':
        case 'n':
  //      case 'V': // in caso di valore equivale ad usare la versione precedente
  //      case 'v':
          isNewMan = true;
          break;
        }
      }
    if(isNewMan) {
      LPCTSTR fileNewMan = findNextParamTrim(p);

      bool foundMoreBit = false;
      uint ix = info.dw;
      if(typeBit) {
        DWORD dw = info.dw;
        if(!dw)
          ix = 0;
        else {
          for(ix = 1; ix <= 32; ++ix, dw >>= 1) {
            if(dw & 1) {
              if(dw & ~1) { // se c'è più di un bit attivo
                ix = info.dw;
                foundMoreBit = true;
                }
              break;
              }
            }
          }
        }
      if(foundMoreBit) {
        TCHAR t[64];
        wsprintf(t, _T("%d"), ix);
        fia.iaa.str = str_newdup(t);
        }
      else
        fia.iaa.str = str_newdup(getStringByLangOrValue(getSetFileAlarm(fileNewMan), ix));
      fia.iaa.type = infoAssocAlarm::tSTR;
      fia.iaa.needDelete = true;
      return true;
      }
    }

  if(infoAssocAlarm::tDW4IX != info.type)
    return true;
  fia.iaa.str = str_newdup(getStringByLang(strAlarm[fia.prph], info.dw));
  fia.iaa.type = infoAssocAlarm::tSTR;
  fia.iaa.needDelete = true;
  return true;
}
//----------------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------------------------
static void clearBuff(LPTSTR target, LPCTSTR source, int sz)
{
  if(!source || !*source || sz < 0)
    return;
  int len = _tcslen(source);
  len = min(len, sz);
  for(int i = 0; i < len; ++i) {
    if(U_(source[i]) <= U_(_T('\r')))
      target[i] = _T(' ');
    else
      target[i] = source[i];
    }
  target[len] = 0;
}
//----------------------------------------------------------------------------
void makeAlarmDescr(LPTSTR descr, size_t lenBuff, const fullInfoAlarm& fia)
{
  *descr = 0;
  switch(fia.iaa.type) {
    case infoAssocAlarm::tDW:
      wsprintf(descr, _T("[%d] "), fia.iaa.dw);
      break;
    case infoAssocAlarm::tFLT:
      descr[0] = _T('[');
      do {
        uint nDec = fia.iaa.nDec;
        if(!nDec || nDec > 10)
          nDec = 5;
        _stprintf_s(descr + 1, lenBuff - 1, _T("%0.*f"), nDec, fia.iaa.flt);
        } while(false);
      zeroTrim(descr + 1);
      _tcscat_s(descr, lenBuff, _T("] "));
      break;
    case infoAssocAlarm::tSTR:
      descr[0] = _T('[');
      _tcscpy_s(descr + 1, lenBuff - 1, fia.iaa.str);
      _tcscat_s(descr, lenBuff, _T("] "));
      break;
    }
  int len = _tcslen(descr);
  clearBuff(descr + len, fia.descr, (int)lenBuff - len - 1);
}
