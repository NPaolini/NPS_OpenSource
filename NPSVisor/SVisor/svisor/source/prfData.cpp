//------------ prfData.cpp ---------------------------------------------
//----------------------------------------------------------------------
#include "precHeader.h"
#include "mainClient.h"
#include "prfData.h"
#include "p_name.h"
#include "1.h"
//----------------------------------------------------------------------
prfData::tagErrBuff prfData::errBuff;
//----------------------------------------------------------------------
prfData::prfData(uData u, uint p_addr, tData type, uint l_addr) :
  U(u), pAddr(p_addr), typeVar(type), lAddr(l_addr)
{
  if(tStrData == type) {
    if(U.str.len) {
      LPBYTE p = U.str.buff;
      U.str.buff = new BYTE[U.str.len + 1];
      memset(U.str.buff, 0, U.str.len + 1);
      memcpy(U.str.buff, p, U.str.len);
      }
    else
      U.str.buff = 0;
    }
}
//----------------------------------------------------------------------
prfData::prfData(const prfData& other) : typeVar(tNoData), lAddr(0)
{
  MK_I64(U.ft) = 0;
  *this = other;
}
//----------------------------------------------------------------------
prfData::prfData(uint lenBuff, LPBYTE Buff) : lAddr(0), pAddr(0), typeVar(tStrData)
{
  U.str.len = lenBuff;
  U.str.buff = new BYTE[lenBuff + 1];
  memset(U.str.buff, 0, lenBuff + 1);
  if(Buff)
    memcpy(U.str.buff, Buff, lenBuff);
}
//----------------------------------------------------------------------
prfData::prfData(LPBYTE Buff) : lAddr(0), pAddr(0), typeVar(tStrData)
{
  U.str.len = strlen((LPSTR)Buff);
  U.str.buff = new BYTE[U.str.len + 1];
  memcpy(U.str.buff, Buff, U.str.len + 1);
}
//----------------------------------------------------------------------
prfData::prfData(uint lenBuff, LPCTSTR Buff) : lAddr(0), pAddr(0), typeVar(tStrData)
{
  U.str.len = lenBuff;
  U.str.buff = new BYTE[lenBuff + 1];
  memset(U.str.buff, 0, lenBuff + 1);
  if(Buff)
    copyStr(U.str.buff, Buff, lenBuff);
}
//----------------------------------------------------------------------
prfData::prfData(LPCTSTR Buff) : lAddr(0), pAddr(0), typeVar(tStrData)
{
  U.str.len = _tcslen(Buff);
  U.str.buff = new BYTE[U.str.len + 1];
  copyStr(U.str.buff, Buff, U.str.len + 1);
}
//----------------------------------------------------------------------
prfData::~prfData()
{
  if(tStrData == typeVar)
    delete []U.str.buff;
}
//----------------------------------------------------------------------
const prfData& prfData::operator =(const prfData& other)
{
  // se non è lo stesso oggetto
  if(&other != this) {
    // se l'altro è di tipo stringa
    if(tStrData == other.typeVar) {
      // se anche questo è stringa
      if(tStrData == typeVar) {
        // se punta allo stesso buffer ???
        if(U.str.buff == other.U.str.buff)
          return *this;
        if(U.str.len < other.U.str.len) {
          delete []U.str.buff;
          U.str.len = other.U.str.len;
          U.str.buff = new BYTE[U.str.len + 1];
          memset(U.str.buff, 0, U.str.len + 1);
          }
        }
      else
        U.str.len = other.U.str.len;

      memcpy(U.str.buff, other.U.str.buff, other.U.str.len + 1);
      }
    else
      U = other.U;
    lAddr = other.lAddr;
    pAddr = other.pAddr;
    typeVar = other.typeVar;
    }
  return *this;
}
//----------------------------------------------------------------------
void prfData::setStr(uint len, LPCTSTR newBuff)
{
  if(newBuff) {
    uint l = _tcslen(newBuff);
    LPBYTE p = new BYTE[l + 1];
    copyStr(p, newBuff, l + 1);
    setStr(len, p);
    delete []p;
    }
  else
    setStr(len);
}
//----------------------------------------------------------------------
void prfData::setStr(uint len, LPBYTE newBuff)
{
  LPBYTE p = 0;
  // se è già di tipo stringa
  if(tStrData == typeVar) {
    // se punta allo stesso buffer e non deve incrementare le dimensioni
    if(U.str.buff == newBuff)
      if(len <= U.str.len)
        return;
    // se la nuova dimensione è maggiore di quella già allocata
    // occorre riallocare, allora si salva il puntatore
    if(len > U.str.len)
      p = U.str.buff;
    }
  else {
    // non era di tipo stringa, la assegna ed azzera la dimensione
    U.str.len = 0;
    typeVar = tStrData;
    }

  // se è necessario riallocare o prima allocazione
  if(p || !U.str.len)
    U.str.buff = new BYTE[len + 1];

  int maxLen = max(len, U.str.len);
  memset(U.str.buff, 0, maxLen + 1);

  if(!newBuff)
    newBuff = p;

  if(newBuff) {
//    uint nLen = _tcslen((LPSTR)newBuff);
//    len = min(len, nLen);
    memcpy(U.str.buff, newBuff, len + 1);
    }
  U.str.len = maxLen;
  delete []p;
}
//----------------------------------------------------------------------
bool prfData::tagErrBuff::hasErr(uint perif, prfData::tResultData type)
{
//  if(!perif)
//    return true;
  switch(type) {
    case prfData::invalidType:
      return toBool(Type & (1 << perif));
    case prfData::invalidAddress:
      return toBool(Addr & (1 << perif));
    case prfData::invalidPerif:
      return toBool(Perif & (1 << perif));
    }
  return false;
}
//----------------------------------------------------------------------
void prfData::tagErrBuff::setErr(uint perif, tResultData type)
{
  switch(type) {
    case prfData::invalidType:
      Type |= (WORD)(1 << perif);
      break;
    case prfData::invalidAddress:
      Addr |= (WORD)(1 << perif);
      break;
    case prfData::invalidPerif:
      Perif |= (WORD)(1 << perif);
      break;
    }
}
//----------------------------------------------------------------------
UINT svMessageBox(PWin* parent, LPCTSTR msg, LPCTSTR title, UINT flag)
{
  static bool inExec;
  if(inExec)
    return 0;
  inExec = true;
  mainClient* mc = getMain();
  if(mc)
    mc->disableFunctionKey();

  HWND focus = GetFocus();
  HWND hw = (parent != 0) ? parent->getHandle() : 0;
  UINT result = MessageBox(hw, msg, title, flag);
  if(mc)
    mc->enableFunctionKey();
  if(focus && IsWindow(focus))
    SetFocus(focus);
  inExec = false;
  return result;
}
//----------------------------------------------------------------------
#define LAST_PRF_DEFINED 9
//----------------------------------------------------------------------
bool prfData::showMessageError(PWin* win, uint prf, tResultData result) const
{
  // se è già stato attivato, torna senza far nulla
  if(errBuff.hasErr(prf, result))
    return false;

  errBuff.setErr(prf, result);

#if 1
  bool needDelete;
  LPCTSTR format = getStringByLangGlob(ID_FORMAT_MSG_ERROR_DATA, needDelete);
  smartPointerConstString sPS(format, needDelete);
#else
  LPCTSTR format = getString(ID_FORMAT_MSG_ERROR_DATA);
#endif
  if(!format)
    format = _T("Error: %s\r\nperipheral %s, address %d, type %s");

  uint id = prfData::invalidType == result ? ID_ERROR_INVALID_TYPE :
            prfData::invalidAddress == result ? ID_ERROR_INVALID_ADDRESS :
            prfData::invalidPerif == result ? ID_ERROR_INVALID_PERIF :
            ID_ERROR_DATA_UNKNOW;

#if 1
  LPCTSTR err = getStringByLangGlob(id, needDelete);
  smartPointerConstString sPSerr(err, needDelete);

  DWORD type_var = typeVar & ~(tNegative);// | tArray);
  LPCTSTR type = getStringOrIdByLangGlob(ID_INIT_TYPE_VAR + type_var);
  smartPointerConstString sPStype(type, needDelete);
  LPCTSTR namePrf;
  needDelete = false;
  if(prf <= LAST_PRF_DEFINED)
    namePrf = getStringOrIdByLangGlob(ID_INIT_NAME_PERIF + prf);
  else {
    static TCHAR prfName[4] = _T("10");
    wsprintf(prfName, _T("%d"), prf);
    namePrf = prfName;
    }
  smartPointerConstString sPSprph(namePrf, needDelete);
#else
  LPCTSTR err = getStringOrId(id);
  DWORD type_var = typeVar & ~(tNegative);// | tArray);
  LPCTSTR type = getStringOrId(ID_INIT_TYPE_VAR + type_var);
  LPCTSTR namePrf;
  if(prf <= LAST_PRF_DEFINED)
    namePrf = getStringOrId(ID_INIT_NAME_PERIF + prf);
  else {
    static TCHAR prfName[4] = _T("10");
    wsprintf(prfName, _T("%d"), prf);
    namePrf = prfName;
    }
#endif

  TCHAR buff[500];
  wsprintf(buff, format, err, namePrf, lAddr, type);
  svMessageBox(win, buff, _T("prfData Error"), MB_OK | MB_ICONSTOP);
  return true;
}
//----------------------------------------------------------------------
/**/
int prfData::getNBit(tData type)
{
  switch(type) {
    case tBData:
    case tBsData:
    case tStrData:
      return 8;
    case tWData:
    case tWsData:
      return 16;
    case tDWData:
    case tDWsData:
    case tFRData:
      return 32;
    case ti64Data:
    case tRData:
    case tDateHour:
    case tDate:
    case tHour:
      return 64;
    case tBitData:
    case tNoData:
      return 0;
    }
  return 0;
}
//----------------------------------------------------------------------
int prfData::getNByte(tData type)
{
  switch(type) {
    case tBData:
    case tBsData:
    case tStrData:
      return 1;
    case tWData:
    case tWsData:
      return 2;
    case tDWData:
    case tDWsData:
    case tFRData:
      return 4;
    case ti64Data:
    case tRData:
    case tDateHour:
    case tDate:
    case tHour:
      return 8;
    case tBitData:
    case tNoData:
      return 0;
    }
  return 0;
}
//----------------------------------------------------------------------
#if 0
//---------------------------------------------------------------------
template <class T>
inline
REALDATA getNormalized(T val, REALDATA nrm)
{
  return (REALDATA)(val * nrm);
}
//---------------------------------------------------------------------
template <class T>
inline
void getDenormalizedInt(T& target, REALDATA val, REALDATA nrm)
{
  if(0.0 == nrm || 1.0 == nrm)
    target = (T)val;
//  target = (T)(val / nrm);
  else {
    val /= nrm;
    target = (T)(val + (val > 0 ? 1.0 : 0) - 0.5);
    }
}
//---------------------------------------------------------------------
template <class T>
inline
void getDenormalizedReal(T& target, REALDATA val, REALDATA nrm)
{
  if(0.0 == nrm || 1.0 == nrm)
    target = (T)val;
//  target = (T)(val / nrm);
  else
    target = (T)(val / nrm);
}
#endif
//----------------------------------------------------------------------------
bool getNormalizedData(REALDATA& val, const prfData& data, REALDATA norm)
{
  switch(data.typeVar) {
    case prfData::tBData:
      val = getNormalized(data.U.b, norm);
      break;
    case prfData::tBsData:
      val = getNormalized(data.U.sb, norm);
      break;

    case prfData::tWData:
      val = getNormalized(data.U.w, norm);
      break;
    case prfData::tWsData:
      val = getNormalized(data.U.sw, norm);
      break;

    case prfData::tDWData:
      val = getNormalized(data.U.dw, norm);
      break;
    case prfData::tDWsData:
      val = getNormalized(data.U.sdw, norm);
      break;

    case prfData::tBitData: // il campo di bit viene trattato come dword
      val = getNormalized(data.U.dw, norm);
      break;

    case prfData::tFRData:
      val = getNormalized(data.U.fw, norm);
      break;

    case prfData::ti64Data: // 64 bit
      val = getNormalized(data.U.li.QuadPart, norm);
      break;

    case prfData::tRData:   // 64 bit float
      val = getNormalized(data.U.rw, norm);
      break;
/*
    case prfData::tDateHour:// 64 bit per data + ora
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else
        set_format_data(buffer, data.U.ft, whichData(), _T(" - "));
      break;

    case prfData::tDate:    // 64 bit per data
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else {
        set_format_data(buffer, data.U.ft, whichData(), _T(" "));
        buffer[10] = 0;
        }
      break;

    case prfData::tHour:    // 64 bit per ora
      set_format_time(buffer, data.U.ft);
      break;

    case prfData::tStrData:
      memcpy(buffer, data.U.str.buff, data.U.str.len + 1);
      break;
*/
    default:
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool getDenormalizedData(prfData& data, REALDATA val, REALDATA norm)
{
  data.U.li.QuadPart = 0;

  switch(data.typeVar) {
    case prfData::tBData:
      getDenormalizedInt(data.U.b, val, norm);
      break;
    case prfData::tBsData:
      getDenormalizedInt(data.U.sb, val, norm);
      break;

    case prfData::tWData:
      getDenormalizedInt(data.U.w, val, norm);
      break;
    case prfData::tWsData:
      getDenormalizedInt(data.U.sw, val, norm);
      break;

    case prfData::tDWData:
      getDenormalizedInt(data.U.dw, val, norm);
      break;
    case prfData::tDWsData:
      getDenormalizedInt(data.U.sdw, val, norm);
      break;

    case prfData::tBitData: // il campo di bit viene trattato come dword
      getDenormalizedInt(data.U.dw, val, norm);
      break;

    case prfData::tFRData:
      getDenormalizedReal(data.U.fw, val, norm);
      break;

    case prfData::ti64Data: // 64 bit
      getDenormalizedInt(data.U.li.QuadPart, val, norm);
      break;

    case prfData::tRData:   // 64 bit float
      getDenormalizedReal(data.U.rw, val, norm);
      break;
/*
    case prfData::tDateHour:// 64 bit per data + ora
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else
        set_format_data(buffer, data.U.ft, whichData(), _T(" - "));
      break;

    case prfData::tDate:    // 64 bit per data
      if(!data.U.li.QuadPart)
        buffer[0] = 0;
      else {
        set_format_data(buffer, data.U.ft, whichData(), _T(" "));
        buffer[10] = 0;
        }
      break;

    case prfData::tHour:    // 64 bit per ora
      set_format_time(buffer, data.U.ft);
      break;

    case prfData::tStrData:
      memcpy(buffer, data.U.str.buff, data.U.str.len + 1);
      break;
*/
    default:
      return false;
    }
  return true;
}
/**/
