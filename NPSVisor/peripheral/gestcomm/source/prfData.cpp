//------------ prfData.cpp ---------------------------------------------
//----------------------------------------------------------------------
// questo è una copia del file presente in svisor.
// è stata tolta la gestione del messaggio di errore per
// evitare di includere file che a loro volta avrebbero
// richiesto altri file ecc.
//----------------------------------------------------------------------
#include "precHeader.h"
#include "pwin.h"
#include "prfData.h"
//----------------------------------------------------------------------
prfData::tagErrBuff prfData::errBuff;
//----------------------------------------------------------------------
//----------------------------------------------------------------------
bool prfData::tagErrBuff::hasErr(uint perif, prfData::tResultData type)
{
  switch(type) {
    case prfData::invalidType:
      return toBool(Type & (1 << (perif - 1)));
    case prfData::invalidAddress:
      return toBool(Addr & (1 << (perif - 1)));
    case prfData::invalidPerif:
      return toBool(Perif & (1 << (perif - 1)));
    }
  return false;
}
//----------------------------------------------------------------------
void prfData::tagErrBuff::setErr(uint perif, tResultData type)
{
  switch(type) {
    case prfData::invalidType:
      Type |= (WORD)(1 << (perif - 1));
      break;
    case prfData::invalidAddress:
      Addr |= (WORD)(1 << (perif - 1));
      break;
    case prfData::invalidPerif:
      Perif |= (WORD)(1 << (perif - 1));
      break;
    }
}
//----------------------------------------------------------------------
bool prfData::showMessageError(PWin* /*win*/, uint /*prf*/, tResultData /*result*/) const
{
#if 1
  return false;
#else
  // se è già stato attivato, torna senza far nulla
  if(errBuff.hasErr(prf, result))
    return false;

  errBuff.setErr(prf, result);

  LPCTSTR format = getString(ID_FORMAT_MSG_ERROR_DATA);
  if(!format)
    format = _T("Error: %s\r\nperipheral %s, address %d, type %s");

  uint id = prfData::invalidType == result ? ID_ERROR_INVALID_TYPE :
            prfData::invalidAddress == result ? ID_ERROR_INVALID_ADDRESS :
            prfData::invalidPerif == result ? ID_ERROR_INVALID_PERIF :
            ID_ERROR_DATA_UNKNOW;

  LPCTSTR err = getStringOrId(id);
  DWORD type_var = typeVar & ~(tNegative | tArray);
  LPCTSTR type = getStringOrId(ID_INIT_TYPE_VAR + type_var);
  LPCTSTR namePrf = getStringOrId(ID_INIT_NAME_PERIF + prf);

  TCHAR buff[500];
  wsprintf(buff, format, err, namePrf, lAddr, type);
  MessageBox(*win, buff, _T("prfData Error"), MB_OK | MB_ICONSTOP);
  return true;
#endif
}
//----------------------------------------------------------------------
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
