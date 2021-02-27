//-------- perif.cpp ---------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "perif.h"
#include "log_stat.h"
#include "file_lck.h"
#include "p_base.h"
#include "mainclient.h"
#include "p_util.h"
#include "def_dir.h"
#include "config.h"

#include "1.h"
#include "manageSplashScreen.h"

#define OFFS_WORD 3
//-------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack2.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 2)

#else
  #error Occorre definire l'allineamento a 2 byte per le strutture seguenti
#endif
//-------------------------------------------------------------
struct datasend {
   WDATA flag;
   WDATA init;
   WDATA end;
   WDATA val[4];
};
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//---------------------------------------------------------------------
//-------------------------------------------------------------
static uint idPerifData[] =
{
    ID_TYPE_ADDRESS_PLC,
    ID_TYPE_ADDRESS_PRF2,
    ID_TYPE_ADDRESS_PRF3,
    ID_TYPE_ADDRESS_PRF4,
    ID_TYPE_ADDRESS_PRF5,
    ID_TYPE_ADDRESS_PRF6,
    ID_TYPE_ADDRESS_PRF7,
    ID_TYPE_ADDRESS_PRF8,
};
//-------------------------------------------------------------
#define ID_PERIF_PLC 2
//-------------------------------------------------------------
static void showErrAndExit(LPCTSTR msg, uint prf)
{
  TCHAR buff[100];
  wsprintf(buff, msg, prf);
  My_Exit(buff);
}
//-----------------------------------------------------------------
static void MSG__(LPCTSTR msg, uint prf)
{
  TCHAR buff[100];
  wsprintf(buff, msg, prf);
//  PWin* main = getAppl()->getMainWindow();
  PWin* main = getCurrBody();
//  InvalidateRect(*main, 0, true);
  svMessageBox(main, buff, _T("Chiusura Periferica"), MB_OK);
  InvalidateRect(*main, 0, 0);
//  MessageBox(GetActiveWindow(), buff, _T("Closing Periph"), MB_OK);
}
//-----------------------------------------------------------------
#define OLD_DIM_DATA (4096 * 4)
#define TRUE_DIM_DWORD (trueDimBuff / sizeof(DWORD))
#define TRUE_DIM_BDATA trueDimBuff
#define TRUE_DIM_PERIF TRUE_DIM_BDATA
//-------------------------------------------------------------
perif::perif(mainClient* par, uint id) : gestPerif(id > 1000 ? id - 1000 : id, par),
    idMsgPerif(0), countRefresh(0), forceDirtyOnReceive(0), bitReset(-1),
    numWordAlarm(0), baseTypeAlarm(0), useSendBit(0),
    currBuff(new BDATA[MAX_BDATA_PERIF]), pMF(0), trueDimBuff(OLD_DIM_DATA)
{
  uint trueId = id > 1000 ? id - 1000 : id;

  RegSTR_WM_PRPH(idMsgPerif, trueId);
  ZeroMemory(oldBuff, sizeof(oldBuff));
  ZeroMemory(currBuff, MAX_BDATA_PERIF);

  if(id == trueId) {
    uint idV = id - ID_PERIF_PLC;
    if(idV >= SIZE_A(idPerifData)) {
      showErrAndExit(_T("L'Id della periferica è -> %d, (deve essere tra 2 e 9)"), id);
      return;
      }
    LPCTSTR p = getString(idPerifData[idV]);
    if(!p) {
      typeAddress = prfData::tDWData;
//      showErrAndExit(_T("Occorre definire un tipo di dato per la Periferica n° %d"), id);
      return;
      }

    uint type = _ttoi(p);

    p = findNextParamTrim(p);
    if(p && _ttoi(p))
      useSendBit = -1;

    switch(type) {
    // solo dati a 32bit
      case prfData::tFRData:    // 32 bit
      case prfData::tDWsData:   // 32 bit
        break;
      default:
      case prfData::tDWData:    // 32 bit
        type = prfData::tDWData;
        break;
      }
    typeAddress = type;

    if(isReadOnly()) {
      needRefresh = true;
      Ready = true;
      trueDimBuff = MAX_BDATA_PERIF;
      openMapping(false);
      }


    p = getString(ID_MANAGE_ALARM_PLC + id - WM_PLC);

    if(p) {
      DWDATA flag;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
            &addrAlarm, &numWordAlarm, &offsEvent, &numWordEvent, &flag, &wordStat,
            &maskAlarm, &maskEvent, &baseTypeAlarm);
      if(!baseTypeAlarm)
        baseTypeAlarm = typeAddress;

      useFlagAlarm = toBool(flag);
      maskAlarm = 1 << maskAlarm;
      maskEvent = 1 << maskEvent;

      p = getString(ID_MANAGE_RESET);
      for(uint i = 0; i < id - WM_PLC; ++i)
        p = findNextParam(p, 2);
      if(p)
        _stscanf_s(p, _T("%d,%d"), &wordResetAlarm, &bitReset);
      }
    }
}
//-----------------------------------------------------------------
perif::~perif()
{
  if(pMF)
    delete pMF;
  else
    delete []currBuff;
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
void perif::setErrPrph()
{
  (LPDWORD(currBuff))[TRUE_DIM_DWORD - 1] = DWORD(-1);
  if(!pMF) {
    TCHAR name[_MAX_PATH];
    MK_NAME_FILE_DATA(name, getId());
    getBasePath(name);
    P_File fl(name);
    if(fl.P_open())
      fl.P_write(currBuff, TRUE_DIM_PERIF);
    }
}
//-----------------------------------------------------------------
// numero delle volte che rilegge forzatamente il file dopo un messaggio
// di dati modificati
#define MAX_COUNT_REFRESH 0
//-----------------------------------------------------------------
void perif::refresh()
{
  baseClass::refresh();
  if(needRefresh) {
    if(!pMF) {
      TCHAR name[_MAX_PATH];
      MK_NAME_FILE_DATA(name, getId());
      getBasePath(name);
      do {
        P_File fl(name, P_READ_ONLY);
        if(fl.P_open() && fl.get_len() > 0)
          fl.P_read(currBuff, TRUE_DIM_PERIF);
        } while(false);
      }
    if(forceDirtyOnReceive < 0) {
      forceDirtyOnReceive = 1;
      for(int i = 0; i < (int)TRUE_DIM_PERIF; ++i)
        oldBuff[i] = ~currBuff[i];
      }
    setDirty(toBool(memcmp(currBuff, oldBuff, TRUE_DIM_PERIF)));
#if 1
    // viene annullato il needRefresh solo se la periferica deve
    // forzare la rilettura dei dati. In questo caso la periferica
    // invia il messaggio di dati cambiati.
    if(forceDirtyOnReceive && !isReadOnly()) {
      if(countRefresh-- <= 0) {
        countRefresh = 0;
        needRefresh = false;
        }
      }
#endif
    }
  if(!isReady() && !(LPDWORD(currBuff))[TRUE_DIM_DWORD - 1])
    setErrPrph();
}
//-----------------------------------------------------
void perif::commit()
{
  criticalLock cl(cSectSet);
  memcpy(oldBuff, currBuff, min(sizeof(oldBuff), TRUE_DIM_BDATA));
  setDirty(false);
}
//-----------------------------------------------------------------
#define GET_FORCE_DIRTY(a) (((int)lParam) & 1)
#define GET_USE_SEND_BIT(a) (((int)lParam) & 2)
#define GET_USE_FILE_MAPPED(a) (((int)lParam) & 4)
#define GET_USE_NEW_SIZE(a) (((int)lParam) & 8)
#define GET_USE_REQUEST_VAR(a) (((int)lParam) & 16)
#define GET_USE_SEND_BIT_16(a) (((int)lParam) & 32)
//-----------------------------------------------------------------
void perif::openMapping(bool useFileMapped)
{
  if(useFileMapped) {
    if(!pMF) {
      TCHAR name[_MAX_PATH];
      MK_NAME_FILE_DATA(name, getId());
      getBasePath(name);
      pMF = new p_MappedFile(name);
//      pMF = new p_MappedFile(name, mP_READ_ONLY);
      if(!pMF->P_open()) {
        delete pMF;
        pMF = 0;
        return;
        }
      delete []currBuff;
      currBuff = (BDATA*)pMF->getAddr();
      }
    // non dovrebbe verificarsi, se si chiude il driver viene chiuso anche il mapping
    else { // è necessario riaprirlo?
      currBuff = 0;
      delete pMF;
      pMF = 0;
      openMapping(useFileMapped);
      }
    }
  else { // !useFileMapped
    if(pMF) {
      delete pMF;
      pMF = 0;
      currBuff = new BDATA[TRUE_DIM_BDATA];
      needRefresh = true;
      refresh();
      }
    // se non si usa il mapping e non era aperto il mapping non deve fare nulla
//    else {
//      }
    }
}
//-----------------------------------------------------------------
bool perif::acceptMsg(uint msg, WPARAM wParam, LPARAM& lParam)
{
  if(msg != idMsgPerif)
    return false;

  switch(LOWORD(wParam)) {
    case MSG_HWND:
      if(!hWnd)
        hWnd = reinterpret_cast<HWND>(lParam);
      break;

    case MSG_IS_READY:
      if(HIWORD(wParam)) {
        if(!isReadOnly()) {
          forceDirtyOnReceive = GET_FORCE_DIRTY(lParam);
          if(useSendBit >= 0) {
            useSendBit = GET_USE_SEND_BIT(lParam) ? 1 : 0;
            if(GET_USE_SEND_BIT_16(lParam))
              useSendBit = 2;
            }
          }
        requestVarPresent = !GET_USE_REQUEST_VAR(lParam);
        bool useFileMapped = toBool(GET_USE_FILE_MAPPED(lParam));
        if(GET_USE_NEW_SIZE(lParam))
          trueDimBuff = MAX_BDATA_PERIF;
        openMapping(useFileMapped);
        if(!Ready) {
          Ready = true;
          needRefresh = true;
//          refresh();
          }
        cfg sCfg = config::getAll();
        int ix = getId() - ID_PERIF_PLC;
        TCHAR path[_MAX_PATH];
        copyStrZ(path, sCfg.pathPerif[ix]);
        sendMessageSplash(WMC_LOADED_PRF_0 + getId(), path);
        }
      else {
        openMapping(false);
        Ready = false;
        }
      break;

    case MSG_CHANGED_DATA:
      needRefresh = true;
      if(forceDirtyOnReceive)
        forceDirtyOnReceive = -1;

      countRefresh = MAX_COUNT_REFRESH;
      break;

    case MSG_CLOSING:
      if(hWnd != (HWND)lParam)
        break;
      if(MSG_OK_CLOSING != HIWORD(wParam)) {
        switch(HIWORD(wParam)) {
//          setErrPrph();
          case MSG_INIT_ERR:
            do {
              cfg sCfg = config::getAll();
              int ix = getId() - ID_PERIF_PLC;
              TCHAR path[_MAX_PATH];
              copyStrZ(path, sCfg.pathPerif[ix]);
              bool hasLogo = sendMessageSplash(WMC_FAILED_PRF_0 + getId(), path);
              if(!hasLogo)
                MSG__(_T("Errore in initializzazione della Periferica n° %d"), getId());
              } while(false);
            break;
          case MSG_I_AM_CLOSING:
            MSG__(_T("Chiusura della Periferica n° %d non comandata\r\ndall'applicazione principale"), getId());
            break;
          }
        hWnd = 0; // abbandona la comunicazione
        }
      break;

    case MSG_RESPONCE:
      Responce = HIWORD(wParam);
      Data = lParam;
      break;
    }
  return 1;
}
//---------------------------------------------------------------------
void perif::getBlockData(LPDWORD target, uint ndw, uint startAddr)
{
  if(startAddr > TRUE_DIM_DWORD)
    startAddr = TRUE_DIM_DWORD;
  ndw = min(ndw, TRUE_DIM_DWORD - startAddr);
  criticalLock cl(cSectSet);
  memcpy(target, ((LPDWORD)currBuff) + startAddr, ndw * sizeof(DWDATA));
}
//---------------------------------------------------------------------
void perif::copyBlockData(LPDWORD source, uint ndw, uint startAddr)
{
  if(startAddr > TRUE_DIM_DWORD)
    startAddr = TRUE_DIM_DWORD;
  ndw = min(ndw, TRUE_DIM_DWORD - startAddr);
  criticalLock cl(cSectSet);
  memcpy(((LPDWORD)currBuff) + startAddr, source, ndw * sizeof(DWDATA));
}
//---------------------------------------------------------------------
void perif::fillBlockData(DWORD value, uint ndw, uint startAddr)
{
  if(startAddr > TRUE_DIM_DWORD)
    startAddr = TRUE_DIM_DWORD;
  ndw = min(ndw, TRUE_DIM_DWORD - startAddr);
  LPDWORD target = ((LPDWORD)currBuff) + startAddr;
  criticalLock cl(cSectSet);
  for(uint i = 0; i < ndw; ++i)
    target[i] = value;
}
//---------------------------------------------------------------------
prfData::tResultData perif::get(prfData& target) const
{
  if(LtoP_End(target.lAddr) >= TRUE_DIM_BDATA)
    return prfData::invalidAddress;

  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidType;
  uint type = target.typeVar & ~prfData::tNegative;
  switch(type) {
    case prfData::tBitData:
      do {
        int nBit = LOWORD(target.U.dw);
        int offs = HIWORD(target.U.dw);
        DWDATA mask = 0xffffffff >> (32 - nBit);

        target.U.dw = getData(currBuff, target.lAddr, typeAddress);
        target.U.dw >>= offs;
        target.U.dw &= mask;

        DWDATA old = getData(oldBuff, target.lAddr, typeAddress);
        old  >>= offs;
        old &= mask;

        result = old == target.U.dw ? prfData::notModified : prfData::isChanged;
        if(target.typeVar & prfData::tNegative)
          target.U.dw = ~target.U.dw & mask;
        } while(false);
      break;

    case prfData::tBData:
      target.U.dw = 0;
      target.U.b = (BDATA)getData(currBuff, target.lAddr, type);
      result = target.U.b == (BDATA)(getData(oldBuff, target.lAddr, type) & 0xff) ?
                prfData::notModified : prfData::isChanged;
      break;
    case prfData::tBsData:
      target.U.dw = 0;
      target.U.sb = (BsDATA)getData(currBuff, target.lAddr, type);
      result = target.U.sb == (BsDATA)(getData(oldBuff, target.lAddr, type) & 0xff) ?
                prfData::notModified : prfData::isChanged;
      break;
    case prfData::tWData:
      target.U.dw = 0;
      target.U.w = (WDATA)getData(currBuff, target.lAddr, type);
      result = target.U.w == (WDATA)(getData(oldBuff, target.lAddr, type) & 0xffff) ?
                prfData::notModified : prfData::isChanged;
      break;
    case prfData::tWsData:
      target.U.dw = 0;
      target.U.sw = (WsDATA)getData(currBuff, target.lAddr, type);
      result = target.U.sw == (WsDATA)(getData(oldBuff, target.lAddr, type) & 0xffff) ?
                prfData::notModified : prfData::isChanged;
      break;

    case prfData::tFRData:
    case prfData::tDWsData:
    case prfData::tDWData:
      target.U.dw = getData(currBuff, target.lAddr, type);
      result = target.U.dw == getData(oldBuff, target.lAddr, type) ?
                prfData::notModified : prfData::isChanged;
      break;

    case prfData::tRData:
      do {
        const BDATA* pBuff = currBuff + LtoP_Init(target.lAddr);
        target.U.rw = *(REALDATA*)pBuff;
        pBuff = oldBuff + LtoP_Init(target.lAddr);
        result = target.U.rw == *(REALDATA*)pBuff ? prfData::notModified : prfData::isChanged;
      } while(false);
      break;

    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
      do {
        const BDATA* pBuff = currBuff + LtoP_Init(target.lAddr);
        target.U.li.QuadPart = *(const __int64*)pBuff;

        pBuff = oldBuff + LtoP_Init(target.lAddr);
        result = target.U.li.QuadPart == *(const __int64*)pBuff ?
               prfData::notModified : prfData::isChanged;
        } while(false);
      break;

    case prfData::tStrData:
      do {
        const BYTE* curr = currBuff + LtoP_Init(target.lAddr);
        memcpy(target.U.str.buff, curr, target.U.str.len);
        const BYTE* old = oldBuff + LtoP_Init(target.lAddr);

        int diff = memcmp(curr, old, target.U.str.len);
        result = diff ? prfData::isChanged : prfData::notModified;
        } while(false);
      break;

    default:
      break;
    }
  return /*isDirty() ? prfData::isChanged :*/ result;
}
//---------------------------------------------------------------------
LPBYTE allocStrToSend(const prfData& data, const datasend& dSend, int& len)
{
  len = sizeof(dSend)  - sizeof(dSend.val) + data.U.str.len;
  LPBYTE pData = new BYTE[len + sizeof(dSend.val)];
  datasend* pdSend = (datasend*)pData;
  memcpy(pData, &dSend, sizeof(dSend));

  LPBYTE target = pData + sizeof(dSend) - sizeof(dSend.val);
#ifndef ADDR_BY_64
  DWDATA pBuffer = dSend.val[0];
  pBuffer |= ((DWDATA)dSend.val[1]) << 16;
#else
  // nel caso si dovesse usare su sistemi a 64bit è già pronta
  __int64 pBuffer = dSend.val[0];
  pBuffer |= ((__int64)dSend.val[1]) << 16;
  pBuffer |= ((__int64)dSend.val[2]) << 32;
  pBuffer |= ((__int64)dSend.val[3]) << 48;
#endif
  LPBYTE source = (LPBYTE)(pBuffer);

  memcpy(target, source, data.U.str.len);
  return pData;
}
//---------------------------------------------------------------------
#define USE_SEND_BIT
#ifdef USE_SEND_BIT
prfData::tResultData perif::setBits(const prfData& data)
{
/*
  // operazione inversa effettuata nel driver
  int offs = HIBYTE(p1);
  int resp = LOBYTE(p1);
  int addr = LOWORD(p2);
  DWDATA value = LOBYTE(HIWORD(p2));
  DWDATA mask =  HIBYTE(HIWORD(p2));
*/
  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidAddress;
#if 1
//  howSend how = CHECK_ALL;
  howSend how = CHECK_READY;
#else
  howSend how = getAutoShow();
  if(NOT_DEFINED == how)
    how = ALL_AND_SHOW;
#endif
  WORD offAndBit = LOWORD(data.U.dw);
  DWORD val = HIWORD(data.U.dw);
#if 1
  if(data.typeVar & prfData::tNegative)
    val = ~val;
#endif
  BYTE nBit = LOBYTE(offAndBit);
  BYTE offs = HIBYTE(offAndBit);

  WORD hiP1 = MAKEWORD(OK_WRITE, offs);
  DWDATA mask = 0xffffffff >> (32 - nBit);
  DWORD p2 = MAKELPARAM(data.lAddr, MAKEWORD(val, mask));

  if(SendCustom(MSG_WRITE_BITS, how, hiP1, p2, OK_WRITE)) {
    result = prfData::okData;
    ReceiveHide(data.lAddr, data.lAddr);
    }
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData perif::setBits16(const prfData& data)
{
/*
  // operazione inversa effettuata nel driver
  int offs_and_bits = HIBYTE(p1);
  int offs = offs_and_bits & 0xf;
  int bits = (offs_and_bits >> 4) & 0xf;
  int resp = LOBYTE(p1);
  int addr = LOWORD(p2);
  DWDATA value = HIWORD(p2);
*/
  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidAddress;
  howSend how = CHECK_READY;

  WORD offAndBit = LOWORD(data.U.dw);
  DWORD val = HIWORD(data.U.dw);
  if(data.typeVar & prfData::tNegative)
    val = ~val;


#if 1
  WORD hiP1 = offAndBit;
#else
  BYTE nBit = LOBYTE(offAndBit);
  BYTE offs = HIBYTE(offAndBit);
  offAndBit = ((DWORD(nBit) << 4) & 0xf0) | (offs & 0xf);

  WORD hiP1 = MAKEWORD(OK_WRITE, offAndBit);
#endif
  DWORD p2 = MAKELPARAM(data.lAddr, val);

  if(SendCustom(MSG_WRITE_BITS_16, how, hiP1, p2, OK_WRITE)) {
    result = prfData::okData;
    ReceiveHide(data.lAddr, data.lAddr);
    }
  return result;
}
//---------------------------------------------------------------------
bool perif::canSendBits(prfData* data)
{
  WORD offAndBit = LOWORD(data->U.dw);

  BYTE nBit = LOBYTE(offAndBit);
  BYTE offs = HIBYTE(offAndBit);
  // non è possibile inviare il dato perché verrebbe troncato,
  // si carica la parte necessaria a rendere il dato pieno e poi
  // si usa il send normale
  if(nBit + offs > 8) {
    prfData data2 = *data;
    DWORD val = HIWORD(data->U.dw);
    bool negative = toBool(data->typeVar & prfData::tNegative);
    if(negative)
      val = ~val;
    data2.typeVar = prfData::tDWData;
    get(data2);

    val <<= offs;
    DWDATA mask = (1 << (DWDATA)nBit) - 1;
    data2.U.dw &= ~(mask << offs);
    data->U.dw = val | data2.U.dw;
    data->typeVar = prfData::tDWData;
    if(negative)
      data->typeVar |= prfData::tNegative;
    return false;
    }
  return true;
}
#endif
//---------------------------------------------------------------------
prfData::tResultData perif::getset(prfData& data, bool noCommit)
{
  criticalLock cl(cSectSet);
  prfData data2 = data;
  get(data);
  prfData::tResultData result = set(data2, noCommit);
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData perif::set(const prfData& data, bool /*noCommit*/)
{
  if(isReadOnly())
    return prfData::okData;
  criticalLock cl(cSectSet);
  uint type = data.typeVar & ~prfData::tNegative;
#ifdef USE_SEND_BIT
  if(useSendBit > 0 && prfData::tBitData == type) {
    if(useSendBit > 1)
      return setBits16(data);
    if(canSendBits((prfData*)&data))
      return setBits(data);
    }
#endif
  prfData::tResultData result = prfData::invalidType;
  datasend dSend;

  int step = fillToSend(data, dSend);
  if(step) {

    howSend how = getAutoShow();
    bool show = false;
    if(NOT_DEFINED == how || ALL_AND_SHOW == how)
      show = true;
    bool success;
    if(prfData::tStrData == type) {
      int len;
      LPBYTE pData = allocStrToSend(data, dSend, len);
      if(gestPerif::NO_CHECK == how)
        success = SendHide(pData, len);
      else
        success = Send(pData, len, show);
      delete []pData;
      }
    else {
      if(gestPerif::NO_CHECK == how)
        success = SendHide(&dSend, sizeof(dSend) - sizeof(dSend.val) + step);
      else
        success = Send(&dSend, sizeof(dSend) - sizeof(dSend.val) + step, show);
      }
    if(!success)
      result = prfData::failed;
    else {
      ReceiveHide(dSend.init,  dSend.end);
      result = prfData::okData;
      }
    }
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData perif::multiSet(const prfData* data, int num, bool show, sorted_multi_set sms)
{
  if(isReadOnly())
    return prfData::okData;

  criticalLock cl(cSectSet);
  int dimBuff = num * (sizeof(WDATA) * 2 + sizeof(DWDATA)) + sizeof(WDATA) + 256;
  for(int j = 0; j < num; ++j)
    if(prfData::tStrData == data[j].typeVar)
      dimBuff += data[j].U.str.len;

  BDATA *buff = new BDATA[dimBuff];
  *(WDATA*)buff = (WDATA)FLAG_PERIF(getId());

  BDATA* pBBuff = buff + sizeof(WDATA);

  prfData::tResultData result = prfData::invalidType;
  int i = 0;
  // se è un solo blocco già ordinato e non è una serie di stringhe
  if(sms_sortedOneBlock == sms && prfData::tStrData != data[0].typeVar) {
    datasend dSend;
    if(fillToSend(data[0], dSend)) {
      int len = sizeof(WDATA) * 2 + prfData::getNByte((prfData::tData)data[0].typeVar);
      memcpy(pBBuff, &dSend.init, len);
      pBBuff += len;

      __int64 target;
      for(i = 1; i < num; ++i) {
        len = getFromData(target, data[i]);
        memcpy(pBBuff, &target, len);
        pBBuff += len;
        }
      // punta all'indirizzo finale
      WDATA* pWBuff = (WDATA*)(buff + sizeof(WDATA) * 2);
      *pWBuff += num - 1;
      }
    }
  else {
    for(i = 0; i < num; ++i) {
      datasend dSend;
      if(fillToSend(data[i], dSend)) {
        if(prfData::tStrData == data[i].typeVar) {
          int len;
          LPBYTE pData = allocStrToSend(data[i], dSend, len);
          len -= sizeof(WDATA);
          memcpy(pBBuff, &dSend.init, len);
          pBBuff += len;
          delete []pData;
          }
        else {
          int minLen = sizeof(WDATA) * 2 + sizeof(DWDATA);
          int len = sizeof(WDATA) * 2 + prfData::getNByte((prfData::tData)data[i].typeVar);
          memcpy(pBBuff, &dSend.init, len);
          if(minLen > len) {
            memset(pBBuff + len, 0, minLen - len);
            pBBuff += minLen;
            }
          else
            pBBuff += len;
          }
        }
      else
        break;
      }
    }
  if(i == num) {
    howSend how = getAutoShow();
    show = false;
    if(NOT_DEFINED == how || ALL_AND_SHOW == how)
      show = true;
    bool success = false;
    if(gestPerif::NO_CHECK == how)
      success = SendHide(buff, pBBuff - buff);
    else
      success = Send(buff, pBBuff - buff, show);
    if(!success)
      result = prfData::failed;
    else {
      DWDATA init = data[0].lAddr;
      DWDATA end = init;
      int ixEnd = 0;
      for(i = 1; i < num; ++i) {
        if(data[i].lAddr < init)
          init = data[i].lAddr;
        else if(data[i].lAddr > end) {
          end = data[i].lAddr;
          ixEnd = i;
          }
        }
      if(data[ixEnd].typeVar != typeAddress)
        ++end;

      ReceiveHide(init, end);
      result = prfData::okData;
      }
    }
  delete []buff;
  return result;

}
//---------------------------------------------------------------------
DWDATA perif::getData(const BDATA *buff, int addr, uint type) const
{
  const BDATA* pBuff = buff + LtoP_Init(addr);

  switch(type) {
    case prfData::tBData:   // 8 bit
    case prfData::tBsData:   // 8 bit
      return *(BDATA*)pBuff;

    case prfData::tWData:   // 16 bit
    case prfData::tWsData:   // 16 bit
      return *(WDATA*)pBuff;

    case prfData::tBitData:
      do {
        DWDATA mask = (DWDATA) -1;
        int shift = LtoP_Init(1) * 8;
        if(shift < 32)
          mask = (1 << shift) - 1;

        return *(DWDATA*)pBuff & mask;
        } while(false);

    case prfData::tFRData:  // 32 bit
    case prfData::tDWData:  // 32 bit
    case prfData::tDWsData:  // 32 bit
      return *(DWDATA*)pBuff;

    default:
      showErrAndExit(_T("Tipo di dato errato per la Periferica n° %d"), getId());
    }
  return 0;
}
//-----------------------------------------------------
bool perif::makeDataToSend(const void *buff, int len, uint id)
{
  TCHAR command[_MAX_PATH];
  makefilenamecommand(command, id);

  P_File f(command, P_CREAT);
  if(f.P_open())
    return f.P_write(buff, len) == static_cast<UDimF>(len);
  return false;
}
//---------------------------------------------------------------------
void perif::unmakeDataToSend(bool failedWrite, uint id)
{
  if(failedWrite) {
    TCHAR command[_MAX_PATH];
    makefilenamecommand(command, id);
    DeleteFile(command);
    }
}
//-----------------------------------------------------
/*
#define MAX_REPEAT_EXIST 10
#define MAX_REPEAT_EXIST_1 4
//-----------------------------------------------------
bool perif::makeDataToSend(const void *buff, int len) const
{
  int i;
  TCHAR command[_MAX_PATH];
  _tcscpy_s(command, SIZE_A(command), getLocalBaseDir());
  int l = _tcslen(command);

  MK_NAME_FILE_COMMAND(command + l, getId());
//  getBasePath(command);

//  PAppl *app = getAppl();

  for(i = 0; i < MAX_REPEAT_EXIST; ++i) {
    WIN32_FIND_DATA fd;

    HANDLE hfff = FindFirstFile(command, &fd);
  // il file viene cancellato dal driver, se esiste
  // significa che il driver sta ancora elaborando i dati da inviare
    if(INVALID_HANDLE_VALUE != hfff)
      FindClose(hfff);
    else
      break;
    if(i == MAX_REPEAT_EXIST_1) {
      Data = 0;
      SendCustom(MSG_WRITE_CUSTOM, NO_CHECK, 0, Data, NO);
      }
    Sleep(100);
    }
  if(MAX_REPEAT_EXIST == i) {
  // se dopo MAX_REPEAT_EXIST * 100 non è ancora pronto ... cancelliamo direttamente il file, tanto c'è poco da fare
  // se il driver non l'ha ancora cancellato sarà mezzo bloccato
#if 0
#if 1
  if(IDNO == msgBoxByLangGlob(Par, ID_FORCE_DELETE_COMMAND, ID_FORCE_DELETE_COMMAND_TITLE, MB_ICONQUESTION | MB_YESNO))
#else
    if(svMessageBox(Par, getStringOrId(ID_FORCE_DELETE_COMMAND),
            getStringOrId(ID_FORCE_DELETE_COMMAND_TITLE), MB_ICONQUESTION | MB_YESNO) ==
            IDNO)
#endif
      return false;
#endif
    DeleteFile(command);
    }
  P_File_Lock f(command);
  if(f.isLocked())
    return f.P_write(buff, len) == static_cast<UDimF>(len);
  return false;
}
//---------------------------------------------------------------------
void perif::unmakeDataToSend(bool failedWrite) const
{
  if(failedWrite) {
    TCHAR command[_MAX_PATH];
    _tcscpy_s(command, SIZE_A(command), getLocalBaseDir());
    int len = _tcslen(command);

    MK_NAME_FILE_COMMAND(command + len, getId());
//    getBasePath(command);
    DeleteFile(command);
    }
}
*/
//---------------------------------------------------------------------
inline static DWDATA maskDataBits(DWDATA val, DWDATA bit)
{
  if(32 <= bit)
    return val;
  return val & ((1 << bit) - 1);
}
//---------------------------------------------------------------------
inline static bool getBit(DWDATA val, int bit)
{
  return toBool(val & (1 << bit));
}
//---------------------------------------------------------------------
void perif::log_Alarm(log_status &log, int which, bool force, int offs, int maxWord, int base, int step)
{
  BDATA *curr = currBuff + LtoP_Init(offs);
  BDATA *old =  oldBuff + LtoP_Init(offs);

  for(int word = 0; word < maxWord; ++word, base += step) {
    DWDATA val = getData(curr, word, typeAddress);
    val = maskDataBits(val, step);
    if(!force)
      // con lo xor si attivano solo i bit diversi
      val ^= maskDataBits(getData(old, word, typeAddress), step);
    if(val) {
      for(int j = 0; j < step; ++j) {
        if(getBit(val, j)) {
          val_gen v;
          v.id = makeIdAlarmByPrph(base + j, getId());
          v.val = which;
          if(getBit(getData(curr, word, typeAddress), j))
            log.setEvent(log_status::onAlarm, &v);
          else
            log.setEvent(log_status::endAlarm, &v);
          }
        }
      }
    }
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
int perif::getFromData(__int64& target, const prfData& data)
{
  uint type = data.typeVar & ~prfData::tNegative;

  int defStep = LtoP_Init(1);
  int step = 0;

  switch(type) {
    case prfData::tBitData:
      do {
        WORD offAndBit = LOWORD(data.U.dw);
        DWORD val = HIWORD(data.U.dw);
//        WORD val = HIWORD(data.U.dw);
        BYTE nBit = LOBYTE(offAndBit);
        BYTE offs = HIBYTE(offAndBit);

        DWDATA mask = 0xffffffff >> (32 - nBit);
        mask <<= offs;

        target = getData(currBuff, data.lAddr, type);
        if(data.typeVar & prfData::tNegative)
//          val = (WORD)~val;
          val = ~val;
        val <<= offs;
        val &= mask;
        mask = ~mask;
        target &= mask;
        target |= val;
        step = defStep;
        } while(false);
      break;

    case prfData::tBData:
      target = data.U.b;
      step = 1;
      break;
    case prfData::tBsData:
      target = data.U.sb;
      step = 1;
      break;

    case prfData::tWData:
      target = data.U.w;
      step = 2;
      break;
    case prfData::tWsData:
      target = data.U.sw;
      step = 2;
      break;

    case prfData::tFRData:
    case prfData::tDWData:
      target = data.U.dw;
      step = 4;
      break;
    case prfData::tDWsData:
      target = data.U.sdw;
      step = 4;
      break;

    case prfData::tStrData:
      target = (DWDATA)data.U.str.buff;
      step = data.U.str.len;// / defStep;
      break;

    case prfData::tRData:   // 64 bit float
    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
      target = data.U.li.QuadPart;
      step = 8;
      break;

    default:
      break;
    }

  return step;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
int perif::fillToSend(const prfData& data, datasend& result)
{
//  uint type = data.typeVar & ~prfData::tNegative;
//  bool success = false;

  int defStep = LtoP_Init(1);

  __int64 target;
  int step = getFromData(target, data);
  if(step) {
    result.flag = (WDATA)FLAG_PERIF(getId());
    result.init = (WDATA)data.lAddr;

    if(defStep >= step)
      result.end = (WDATA)data.lAddr;
    else {
      int addr = data.lAddr;
      addr += step / defStep - !(step / defStep);
      result.end = (WDATA)addr;
      }

    result.val[0] = (WDATA)(target & 0xffff);
    result.val[1] = (WDATA)(target >> 16);
    result.val[2] = (WDATA)(target >> 32);
    result.val[3] = (WDATA)(target >> 48);
    }

  return step;
}
//---------------------------------------------------------------------
//----------------------------------------------------------------------------
gestPerif::statAlarm perif::hasAlarm() const
{
  if(!numWordAlarm)
    return sNoAlarm;

  if(useFlagAlarm) {
  // gestione con bit di allarmi/eventi nel primo/ultimo bit della word
    gestPerif::statAlarm result = sNoAlarm;
    DWDATA word = getData(currBuff, wordStat, typeAddress);
    if(word & maskAlarm)
      result = sAlarm;
    else if(numWordEvent && (word & maskEvent))
      result = sEvent;
    if(sNoAlarm != result)
      ReceiveHide(addrAlarm, addrAlarm + numWordAlarm - 1);
    return result;
    }
  // gestione senza bit di allarmi con lettura continua di tutte le word
  for(DWDATA i = 0; i < numWordAlarm; ++i) {
    DWDATA word = getData(currBuff, addrAlarm + i, baseTypeAlarm);
    if(word) {
      if(!numWordEvent || (i < offsEvent || i > offsEvent + numWordEvent))
        return sAlarm;
      return sEvent;
      }
    }
  return sNoAlarm;
}
//---------------------------------------------------------------------
void perif::sendResetAlarm()
{
  if(bitReset < 0)
    return;

//-------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack2.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 2)

#else
  #error Occorre definire l'allineamento a 8 byte per le strutture seguenti
#endif
//-------------------------------------------------------------
  struct data_send {
    WDATA flag;
    WDATA init;
    WDATA end;
    DWDATA val;
    } sendData = {
      (WDATA)FLAG_PERIF(getId()),
      (WDATA)wordResetAlarm,
      (WDATA)wordResetAlarm,
      0
      };
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//---------------------------------------------------------------------

    prfData data;
    data.lAddr = wordResetAlarm;
    data.typeVar = typeAddress;

    prfData::tResultData result = get(data);

    if(prfData::failed >= result) {
      getCurrBody()->ShowErrorData(WM_REAL_PLC, data, result);
//      data.showMessageError(Par, 2, result);
      return;
      }

    sendData.val = data.U.dw | (1 << bitReset);

    if(Send(&sendData, sizeof(sendData)))
      Receive(wordResetAlarm, wordResetAlarm);

}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#define NBYTE_WORD prfData::getNByte((prfData::tData)typeAddress)
#define NBIT_WORD  prfData::getNBit((prfData::tData)typeAddress)
//---------------------------------------------------------------------
#define NBYTE_WORD_ALARM prfData::getNByte((prfData::tData)baseTypeAlarm)
#define NBIT_WORD_ALARM  prfData::getNBit((prfData::tData)baseTypeAlarm)
//---------------------------------------------------------------------
void perif::logAlarm(log_status &log, int which, bool force)
{
  if(numWordAlarm)
    perif::log_Alarm(log, which, force, addrAlarm, numWordAlarm, 0, NBIT_WORD_ALARM);
}
//---------------------------------------------------------------------
int perif::getNBitAlarm() const
{
  return NBIT_WORD_ALARM;
}
//---------------------------------------------------------------------
bool perif::sendErrBit(eSetting /*set*/, int /*bit*/)
{
  return false;
/*
  WDATA Err;
  get(IO_SET_ALARM, &Err);
  if(sSet == set)
    Err |= WDATA(1 << bit);
  else if(sReset == set)
    Err &= WDATA(~(1 << bit));
  else
    return false;

  WDATA buff[3 + 1];
  buff[0] = FLAG_SEND_PLC;
  buff[1] = PLC_ADDR(IO_SET_ALARM);
  buff[2] = PLC_ADDR_END(IO_SET_ALARM);
  buff[3] = Err;
  return SendHide(buff, sizeof(buff));
*/
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
prfData::tResultData perif::getErr(prfData& target) const
{
  if(numWordAlarm) {
    target.lAddr += addrAlarm;
    return get(target);
    }
  return prfData::invalidPerif;
}
//-----------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
perifPLC::perifPLC(mainClient* par) : perif(par, WM_REAL_PLC),
    useRunning(alwaysTrue), maskRunning(0), offsRunning(0)
{
  #define MAX_VAL_RUNNING (ID_MANAGE_RESET - ID_MANAGE_RUNNING - 1)

  LPCTSTR p = getString(ID_MANAGE_RUNNING);
  if(p) {
    _stscanf_s(p, _T("%d,%d,%d,%d"),
            &useRunning, &wordRunning, &maskRunning, &offsRunning);
    if(useVal == useRunning) {
#if 1
      // maskRunning contiene il numero di valori che seguono
      uint i;
      for(i = 0; i < maskRunning; ++i) {
        LPCTSTR pV = getString(ID_MANAGE_RUNNING + 1 + i);
        if(!pV)
          break;
        pVal[i] = _ttoi(pV);
        }
      maskRunning = i;
#else
      LPCTSTR pV = findNextParam(p, 4);
      int i;
      for(i = 0; i < MAX_VAL_STATUS; ++i) {
        if(!pV)
          break;
        pVal[i] = _ttoi(pV);
        pV = findNextParam(pV, 1);
        }
#endif
      if(!i)
        useRunning = alwaysFalse;
      for(; i < MAX_VAL_STATUS; ++i)
        pVal[i] = 0;
      }
    }
  // per attivare lo stato di onRun al primo avvio
  if(alwaysTrue == useRunning)
    maskRunning = 1;
}
//-------------------------------------------------------------
perifPLC::~perifPLC()
{
}
//-----------------------------------------------------------------
void perifPLC::commit()
{
  if(alwaysTrue == useRunning)
    maskRunning = 0;
  baseClass::commit();
}
//-------------------------------------------------------------
#define statLine(buff)  getData(buff, wordRunning, typeAddress)
//-----------------------------------------------------------------
gestPerif::statMachine perifPLC::isRunning() const
{
  switch(useRunning) {
    case alwaysFalse:
      return sDejaStop;
    case alwaysTrue:
      return maskRunning ? sOnRun : sDejaRun;
    }

  DWDATA stat = statLine(currBuff);
  DWDATA oldstat = statLine(oldBuff);

  bool running;
  bool oldrunning;

  stat >>= offsRunning;
  oldstat >>= offsRunning;

  switch(useRunning) {
    case useBits:
      running = toBool(stat & maskRunning);
      oldrunning = toBool(oldstat & maskRunning);
      break;
    case useVal:
      running = false;
      oldrunning = false;
#if 1
      for(uint i = 0; i < maskRunning; ++i) {
        if(stat == pVal[i]) {
          running = true;
          break;
          }
        }
      for(uint i = 0; i < maskRunning; ++i) {
        if(oldstat == pVal[i]) {
          oldrunning = true;
          break;
          }
        }
#else
      stat &= (1 << maskRunning) - 1;
      for(uint i = 0; i < MAX_VAL_STATUS && pVal[i]; ++i) {
        if(stat == pVal[i]) {
          running = true;
          break;
          }
        }
      oldstat &= (1 << maskRunning) - 1;
      for(uint i = 0; i < MAX_VAL_STATUS && pVal[i]; ++i) {
        if(oldstat == pVal[i]) {
          oldrunning = true;
          break;
          }
        }
#endif
    }

  if(running) {
    if(!oldrunning)
      return sOnRun;
    return sDejaRun;
    }
  if(oldrunning)
    return sOnStop;
  return sDejaStop;
}
//-----------------------------------------------------------------
bool perifPLC::isOnEmerg() const
{
  LPCTSTR p = getString(ID_MANAGE_EMERG);
  if(!p)
    return false;

  DWDATA maskOrBit = _ttoi(p);

  DWDATA stat = statLine(currBuff);
  stat >>= offsRunning;

  switch(useRunning) {
    case useBits:
      stat &= maskOrBit;
      break;
    case useVal:
      stat &= (1 << maskOrBit) - 1;
      break;
    }
  return toBool(stat);
}
//----------------------------------------------------------------------------
