//----------- COMgPerif.CPP ------------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#include "file_lck.h"
#include "comgPerif.h"
#include "perifqueue.h"

#include "p_txt.h"
#include "prfData.h"

#include "hdrmsg.h"
#include "manageSetupPrph.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CustomQueue::~CustomQueue()
{
  while(firstFree) {
    pCustomData tmp = firstFree;
    firstFree = firstFree->U.next;
    delete tmp;
    }
}
//-----------------------------------------------------------------------------
customData *CustomQueue::getFree()
{
  pCustomData tmp;
  if(firstFree) {
    tmp = firstFree;
    firstFree = firstFree->U.next;
    }
  else
    tmp = new customData;
  return tmp;
}
//-----------------------------------------------------------------------------
void CustomQueue::freeData(pDataType d)
{
#ifdef USE_TEMPLATE
  #define dt d
#else
  pCustomData dt = dynamic_cast<pCustomData>(d);
#endif
  if(dt) {
    dt->U.next = firstFree;
    firstFree = dt;
    }
}
//-----------------------------------------------------------------------------
void CustomQueue::addToFree(pCustomData d)
{
  if(d) {
    d->U.next = firstFree;
    firstFree = d;
    }
}
//-----------------------------------------------------------------------------
static bool isCompatible(const addrToComm& d1, const addrToComm& d2)
{
  if(d1.type != d2.type)
    return false;
  if(d1.addr != d2.addr)
    return false;
  if(d1.db != d2.db)
    return false;
  if(d1.port != d2.port)
    return false;
  if(d1.ipAddr != d2.ipAddr)
    return false;
  return true;
}
//-----------------------------------------------------------------------------
bool CustomQueue::pushBits(pCustomData d)
{
#if 1
  return PushAlways(d);
#else
  List* t = First;
  const addrToComm& d1 = d->data;
  while(t) {
    pCustomData td = t->L;
    if(gestCommgPerif::tWrite == td->U.type) {
      if(isCompatible(d1, td->data)) {
        DWDATA* pdS = (DWDATA*)d1.buff;
        DWDATA* pdT = (DWDATA*)td->data.buff;
        DWDATA value = pdT[0]; // valore già presente
        DWDATA mask = pdS[1]; // nuova mask
        value &= ~mask; // annulla i valori corrispondenti al nuovo input
        pdT[0] = value | pdS[0]; // unisce valori
        pdT[1] |= mask; // unisce maschera
        addToFree(d); // elimina il pacchetto
        return true;
        }
      }
    t = t->Next;
    }
  // non esiste nella coda, lo aggiunge
  return atTop(d);
#endif
}
//-----------------------------------------------------------------------------
bool CustomQueue::isEqual(const pDataType newData, const pDataType listData)
{
#ifdef USE_TEMPLATE
  #define dt1 newData
  #define dt2 listData
#else
  pCustomData dt1 = dynamic_cast<pCustomData>(newData);
  pCustomData dt2 = dynamic_cast<pCustomData>(listData);
#endif
    // se è in scrittura accoda sempre, anche se duplicato (non possono
    // comunque esistere duplicati)
    if(gestCommgPerif::tWrite == dt1->U.type)
      return false;
    return dt1->data   == dt2->data &&
           dt1->U.type == dt2->U.type;
}
//-----------------------------------------------------------------------------
DWORD reverseIP(DWORD ip)
{
  ip = ((ip & 0xff) << 24) | ((ip & 0xff00) << 8) |
       ((ip & 0xff0000) >> 8) | (( ip & 0xff000000) >> 24);
  return ip;
}
//-----------------------------------------------------------------------------
#define ADDRFILE_EXT _T(".ad7")
//#define ID_DATATYPE 1000000
//#define ID_VER_ADR  1000001
//-----------------------------------------------------------------------------
void stringToAddr(long ix, long nElem, LPCTSTR p, alternAddress* Buff_Addr)
{
  if(ix > nElem)
    return;

  alternAddress& addr = Buff_Addr[ix - 1];

  // non mantiene più la compatibilità con le versioni vecchie dei file .adr
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%lf,%lf,%lf,%lf"),
      &addr.db, &addr.addr,
      &addr.dataType, &addr.action,
      &addr.ipAddr, &addr.port,
      &addr.vMin,
      &addr.vMax,
      &addr.vMinP,
      &addr.vMaxP
      );
}
//-----------------------------------------------------------------------------
//  enum verify { noAction, okRange, subRange, overRange };
//-----------------------------------------------------------------------------
template <class T>
//normalizer::verify normalizerInt<T>::getNormalized(T& data)
normVerify::verify normalizerInt<T>::getNormalized(T& data)
{
  if(vMin == vMax || vMinP == vMaxP)
    return normVerify::noAction;

  aaType t = (aaType)data;
  normVerify::verify result = normVerify::okRange;

  if(t < vMinP) {
    t = vMinP;
    result = normVerify::subRange;
    }
  if(vMaxP < t) {
    t = vMaxP;
    result = normVerify::overRange;
    }

  aaType norm = vMax - vMin;
  aaType normP = vMaxP - vMinP;

  t -= vMinP;
  t *= norm / normP;
  t += vMin;

  *(fREALDATA*)&data = (fREALDATA)t;
  return result;
}
//-----------------------------------------------------------------------------
template <class T>
normVerify::verify normalizerReal<T>::getNormalized(T& data)
{
  if(vMin == vMax || vMinP == vMaxP)
    return normVerify::noAction;

  aaType t = (aaType)data;

  normVerify::verify result = normVerify::okRange;

  if(t < vMinP) {
    t = vMinP;
    result = normVerify::subRange;
    }
  if(vMaxP < t) {
    t = vMaxP;
    result = normVerify::overRange;
    }

  aaType norm = vMax - vMin;
  aaType normP = vMaxP - vMinP;

  t -= vMinP;
  t *= norm / normP;
  t += vMin;

  data = (T)t;
  return result;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <class T>
normVerify::verify normalizerInt<T>::getDenormalized(T& data)
{
  if(vMin == vMax || vMinP == vMaxP)
    return normVerify::noAction;

  aaType t = (aaType)data;
  normVerify::verify result = normVerify::okRange;

  if(t < vMin) {
    t = vMin;
    result = normVerify::subRange;
    }
  if(vMax < t) {
    t = vMax;
    result = normVerify::overRange;
    }

  aaType norm = vMax - vMin;
  aaType normP = vMaxP - vMinP;

  t -= vMin;
  t *= normP / norm;
  t += vMinP;

  data = (T)(t + (t < 0 ? 0 : 1.0) - 0.5);
  return result;
}
//-----------------------------------------------------------------------------
template <class T>
normVerify::verify normalizerReal<T>::getDenormalized(T& data)
{
  if(vMin == vMax || vMinP == vMaxP)
    return normVerify::noAction;

  aaType t = (aaType)data;
  normVerify::verify result = normVerify::okRange;

  if(t < vMin) {
    t = vMin;
    result = normVerify::subRange;
    }
  if(vMax < t) {
    t = vMax;
    result = normVerify::overRange;
    }

  aaType norm = vMax - vMin;
  aaType normP = vMaxP - vMinP;

  t -= vMin;
  t *= normP / norm;
  t += vMinP;

  data = (T)t;
  return result;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
gestCommgPerif::gestCommgPerif(LPCTSTR file, WORD flagCommand) :
  baseClass(file), flagCommand(flagCommand),
  CustomDataRead(new CustomQueue), CustomDataWrite(new CustomQueue),
  lenFile(0), typeSize(DEF_TYPE_SIZE),
  CurrAction(noAction), Buff_Addr(0),
  BuffBlock(0), numBuffBlock(0),
  BuffBlockReq(0), numBuffBlockReq(0), useBlockReq(false),
  Buff_Perif(new BDATA[MAX_DIM_PERIF]), pMF(0), TimerTick(0), MSP(0),
  requestVarPresent(false)
{
  ZeroMemory(Buff_Lav, sizeof(Buff_Lav));
  ZeroMemory(Buff_Write, sizeof(Buff_Write));
  ZeroMemory(Buff_Perif, MAX_DIM_PERIF);
  ZeroMemory(Buff_Scratch, sizeof(Buff_Scratch));
}
//-----------------------------------------------------------------------------
gestCommgPerif::~gestCommgPerif()
{
  delete CustomDataRead;
  delete CustomDataWrite;
  delete []Buff_Addr;
  delete []BuffBlock;
  if(pMF)
    delete pMF;
  else
    delete []Buff_Perif;

 delete []BuffBlockReq;
 uint nElem = OneRead.getElem();
 for(uint j = 0; j < nElem; ++j)
   delete []OneRead[j].BlockReq;
 delete MSP;
}
//-----------------------------------------------------------------------------
uint gestCommgPerif::getTimerTick() const
{
  return TimerTick ? TimerTick : baseClass::getTimerTick();
}
//-----------------------------------------------------------------------------
bool gestCommgPerif::addBitsCommand(WORD p1, DWORD p2, DWORD id_req, bool on16)
{
  DWDATA offs;
  int resp;
  int addr = LOWORD(p2);
  DWDATA value;
  DWDATA mask;
  if(on16) {
    resp = OK_WRITE;
    offs = HIBYTE(p1);
    DWDATA nbit = LOBYTE(p1);
    value = HIWORD(p2);
    mask =  (1 << nbit) - 1;
    }
  else {
    offs = HIBYTE(p1);
    resp = LOBYTE(p1);
    value = LOBYTE(HIWORD(p2));
    mask =  HIBYTE(HIWORD(p2));
    }

  value &= mask;

  pCustomData data = CustomDataWrite->getFree();
  data->Clear();
  data->U.type = tWrite;
//  data->U.type = tWriteBits;
  data->responce = resp;
  data->idReq = id_req;

  data->data.ipAddr = Buff_Addr[addr].ipAddr;
  data->data.port = Buff_Addr[addr].port;
  data->data.addr = Buff_Addr[addr].addr;
  data->data.type = prfData::tBitData;
  data->data.db = Buff_Addr[addr].db;
  data->data.len = 1;

  value <<= offs;
  mask  <<= offs;
  *(DWDATA*)(data->data.buff) = value;
  *((DWDATA*)(data->data.buff) + 1) = mask;
  bool success = CustomDataWrite->pushBits(data);
//  bool success = CustomData->atTop(data);
  if(!success)
    CustomDataWrite->addToFree(data);
  return success;
}
//-----------------------------------------------------------------------------
#define MAX_COMMAND_QUEUE 50
//-----------------------------------------------------------------------------
bool gestCommgPerif::addCommand(gPerif_Command cmd, WORD p1, DWORD p2, DWORD id_req,
              actionFromFile action, LPBYTE buffer)
{
  switch(cmd) {
    case writeCustom:
    case writeBits:
    case writeBits16:
      if(MAX_COMMAND_QUEUE <= CustomDataWrite->getElem()) {
        deleteFile4Write(p2);
        return false;
        }
      break;
    case readCustom:
      if(MAX_COMMAND_QUEUE <= CustomDataRead->getElem())
        return false;
      break;
    default:
      return false;
    }

  if(writeBits == cmd)
    return addBitsCommand(p1, p2, id_req, false);
  if(writeBits16 == cmd)
    return addBitsCommand(p1, p2, id_req, true);
  int init = LOWORD(p2);
  int end = HIWORD(p2);

  static int nAlloc = 50;
  addrToComm* tmp;

  CustomQueue* CustomData = readCustom == cmd ? CustomDataRead : CustomDataWrite;
  bool success = false;

  do {
    tmp = new addrToComm[nAlloc];
    int allocated = nAlloc;
    bool result;
    if(writeCustom == cmd)
      result = LAddrToPAddrSend(tmp, allocated, buffer, p2);
    else
      result = LAddrToPAddrReceive(init, end, tmp, allocated, action);
    if(!result) {
      delete []tmp;
      tmp = 0;
      if(allocated) {
        nAlloc += 50;
        continue;
        }
      break;
      }
    bool onTop = false;
    int init = 0;
    int end = allocated;
    int step = 1;
    // se vanno in testa, rovescia l'ordine di inserimento, ma solo se comando di lettura
    if(NO != p1 && readCustom == cmd) {
      onTop = true;
      init = allocated - 1;
      end = -1;
      step = -1;
      }
    for(int i = init; i != end; i += step) {
      pCustomData data = CustomData->getFree();
      data->Clear();
      switch(cmd) {
        case writeCustom:
          data->U.type = tWrite;
          break;
        case readCustom:
          data->U.type = tRead;
          break;
        default:
          data->U.type = cmd;
          break;
        }
      data->data = tmp[i];
      data->idReq = id_req;
      if(i != init && i != (end - step)) {
        // se è un blocco di mezzo nessuna risposta o attesa
        if(onTop) {
          data->responce = WAIT_NEXT_BLOCK;
          success = CustomData->atTop(data);
          }
        else {
          data->responce = NO;
          if(readCustom == cmd)
            success = CustomData->Push(data);
          else
            success = CustomData->PushAlways(data);
          }
        }

        // primo o ultimo blocco
      else {
        if(onTop) {
          if(i != init || 1 == allocated)
            data->responce = p1;
          else
            data->responce = WAIT_NEXT_BLOCK;
          success = CustomData->atTop(data);
          }
        else {
          if(i == end - step)
            data->responce = p1;
          else
            data->responce = NO;
          if(readCustom == cmd)
            success = CustomData->Push(data);
          else
            success = CustomData->PushAlways(data);
          }
        }
      if(!success)
        CustomData->addToFree(data);

      }
    break;

    } while(true);

  if(!tmp)
    return false;

  delete []tmp;
  return success;
}
//-----------------------------------------------------------------------------
bool gestCommgPerif::addReadCustom(int init, int end, WORD param)
{
  return addCommand(readCustom, param, MAKELPARAM(init, end), 0);
}
//-----------------------------------------------------------------------------
void gestCommgPerif::defMakeAction(pCustomData data)
{
  CurrAction = MAKELPARAM(data->U.type, data->responce);
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::SendCommand()
{
  if(Logoff)
    return gestComm::NoErr;
//#define TEST__

#ifdef TEST__
  // per test

//  addReadCustom(0, 10, NO);
//  addReadCustom(10, 20, NO);
//  addReadCustom(20, 30, NO);
  addCommand(writeCustom, OK_WRITE, 0, 0);
#endif
  CustomQueue* CustomDataSet[] = { CustomDataWrite, CustomDataRead };

  for(uint i = 0; i < SIZE_A(CustomDataSet); ++i) {
    CustomQueue* CustomData = CustomDataSet[i];
    pCustomData data = CustomData->getCurrData();
    if(data) {
      if(tRead == data->U.type)
        CurrAction = MAKELPARAM(readCustom, data->responce);

      else if(tWrite  == data->U.type)
        CurrAction = MAKELPARAM(writeCustom, data->responce);

      else defMakeAction(data);

      idReq = data->idReq;
      }
    else
      CurrAction = 0;
    switch(LOWORD(CurrAction)) {
      case readCustom:
        return Send4ReadCustom();

      case writeCustom:
        return Send4WriteCustom();
      }
    }
  return defSendCommand();
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::DoData(gestComm::ComResult last)
{
  if(Logoff)
    return gestComm::NoErr;

  if(last < gestComm::NoErr)
    return last;

  ComResult res;
  switch(LOWORD(CurrAction)) {
    case readCustom:
      res = ReadCustom();
      break;

    case writeCustom:
      res = WriteCustom();
      break;

    default:
      res = defDoData(last);
      break;
    }
  return res;
}
//-----------------------------------------------------------------------------
//#define TRACE_SEND(msg) MessageBox(0, msg, "trace", MB_OK | MB_ICONINFORMATION);
#define TRACE_SEND(msg)
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::read(BDATA *buff, uint id)
{
  TRACE_SEND("On Read")
  TCHAR path[_MAX_PATH];
  makeFile4Write(path, id);
  ComResult result;
  lenFile = 0;
  do {
    P_File f(path, P_READ_ONLY);
    if(!f.P_open()) {
      result = gestComm::OpenFileErr;
      break;
      }
    lenFile = f.get_len();
    if(lenFile > MAX_DIM_FILE || !lenFile) {
      result = gestComm::ReadFileErr;
      break;
      }
    result = f.P_read(buff, (lUDimF)lenFile) == lenFile ?
              gestComm::NoErr : gestComm::ReadFileErr;
    } while(0);
  // deve essere cancellato, altrimenti il sv non invia altri dati
#ifndef TEST__
  DeleteFile(path);
#endif
  return result;
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::Send4NextCustom(Perif_TypeCommand type)
{
  CustomQueue* CustomData = tRead == type ? CustomDataRead : CustomDataWrite;

  pCustomData data = CustomData->getCurrData();
  if(data) {
    addrToComm& aData = data->data;
    needResponce = data->responce != NO;
    ComResult res = Send(&aData, type);
    // se si verifica un errore e si è in attesa di una risposta
    // si imposta il fallimento
    if(gestComm::NoErr > res) {
      // se si tratta di una serie di blocchi scarica i successivi fino a
      // quello contenente il codice di ritorno
      while(WAIT_NEXT_BLOCK == data->responce) {
        CustomData->Pop();
        data = CustomData->getCurrData();
        if(!data || NO == data->responce)
          return gestComm::ReadFileErr;
        }
      if(NO != data->responce) {
        sResponce = MAKE_SRESP(FAILED, tRead == type ? gestComm::RecErr : gestComm::SendErr);
        res = gestComm::doResponce;
        }
      // se fallisce occorre togliere dalla lista i dati corrispondenti
      // al comando, poiché la corrispondente read(write)custom() non verrà
      // richiamata
      DWORD resp = data->responce;

      CustomData->Pop();
      }
    return res;
    }
  // errore, per ogni dato immesso deve essere stato immesso anche un comando
  return gestComm::ReadFileErr;
}
//-----------------------------------------------------------------------------
static
void setErr(BDATA* buff, int offs)
{
  int addr = MAX_DIM_PERIF / DEF_TYPE_SIZE - 1;
  DWDATA* p = ((DWDATA*)buff) + addr;
  *p |= 1 << offs;
}
//-----------------------------------------------------------------------------
static
void clearErr(BDATA* buff, int offs)
{
  int addr = MAX_DIM_PERIF / DEF_TYPE_SIZE - 1;
  DWDATA* p = ((DWDATA*)buff) + addr;
  *p &= ~(1 << offs);
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::ReadCustom()
{
  CustomQueue* CustomData = CustomDataRead;

  customData *data = CustomData->getCurrData();

  // prepara per nessun errore
  ComResult result = gestComm::NoErr;
  customData *data2 = 0;
  sResponce = MAKE_NO_RESP;
  if(data) {
    needResponce = data->responce != NO;
    addrToComm& aData = data->data;
    if(tRead == data->U.type) {
      result = Receive(&aData, Buff_Scratch);

      if(Logoff)
        return gestComm::NoErr;

      ComResult trueResult = result;
      if(gestComm::NoErr <= result) {
        result = gestComm::doResponce;
        sResponce = MAKE_SRESP(data->responce, gestComm::NoErr);
        if(NO == data->responce || WAIT_NEXT_BLOCK == data->responce)
          // se tutto ok e non deve tornare un risultato annulla
          // l'assegnazione precedente per evitare l'uscita dal ciclo interno
          result = gestComm::NoErr;

        // scarica i dati dal buffer di scratch(fisico) in quello di lavoro(logico)
        // a meno che non sia stata effettuata alcuna azione di lettura e quindi lo
        // scratch non contiene dati validi
        if(gestComm::noAction != trueResult) {
          ScratchToLav(&data->data);
          if(gestComm::onErrButSave != trueResult)
            clearErr(Buff_Lav, 0);
          }
        if(gestComm::noActionButReload == trueResult) {
          data2 = CustomData->getFree();
          *data2 = *data;
          }
        }
      if(gestComm::NoErr > trueResult || gestComm::onErrButSave == trueResult) {
        setErr(Buff_Lav, 0);
        while(WAIT_NEXT_BLOCK == data->responce) {
          CustomData->Pop();
          data = CustomData->getCurrData();
          if(!data || NO == data->responce)
            return gestComm::ReadFileErr;
          }
        if(NO != data->responce)
          sResponce = MAKE_FAILED_REC;
        else
          sResponce = MAKE_SRESP(NO, gestComm::RecErr);
        result = gestComm::doResponce;
        }
      }
    CustomData->Pop();
    if(data2)
      CustomData->Push(data2);
    }
  return result;
}
//-----------------------------------------------------------------------------
static bool isSame(const alternAddress& d1, const addrToComm& d2)
{
   if(d1.addr != d2.addr)
    return false;
  if(d1.db != d2.db)
    return false;
  if(d1.port != d2.port)
    return false;
  if(d1.ipAddr != d2.ipAddr)
    return false;
  return true;
}
//-----------------------------------------------------------------------------
static DWORD getTypeBitVar(const alternAddress* b_Addr, addrToComm* pAddr)
{
  uint nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  for(uint i = 0; i < nElem; ++i) {
    if(isSame(b_Addr[i], *pAddr))
      return b_Addr[i].dataType;
    }
  return pAddr->type;
}
//-----------------------------------------------------------------------------
void gestCommgPerif::prepareSendBits(addrToComm* pAddr)
{
  BDATA buff[20];
  ZeroMemory(buff, sizeof(buff));
  addrToComm atc = *pAddr;

  atc.len = 1;
  DWORD type = getTypeBitVar(Buff_Addr, pAddr);
  atc.type = type;
  if(gestComm::NoErr == Receive(&atc, buff)) {
    DWDATA readed = *(DWDATA*)buff;
    DWDATA toSet = *(DWDATA*)pAddr->buff;
    DWDATA mask = *(DWDATA*)(pAddr->buff + sizeof(DWDATA));
    readed &= ~mask;
    toSet &= mask;
    toSet |= readed;
    *(DWDATA*)pAddr->buff = toSet;
    pAddr->type = type;
    }
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::WriteCustom()
{
  CustomQueue* CustomData = CustomDataWrite;

  customData *data = CustomData->getCurrData();
  // prepara per nessun errore
  ComResult result = gestComm::NoErr;
  sResponce = MAKE_NO_RESP;
  if(data) {
    idReq = data->idReq;
    if(tWrite == data->U.type) {
      needResponce = data->responce != NO;
      if(prfData::tBitData == data->data.type)
        prepareSendBits(&data->data);

      if(MSP && MSP->committed())
        MSP->refreshWrite(&data->data);
      result = SendData(&data->data);



      if(gestComm::NoErr <= result) {
        result = gestComm::doResponce;
        sResponce = MAKE_SRESP(data->responce, gestComm::NoErr);
        if(NO == data->responce || WAIT_NEXT_BLOCK == data->responce)
          // se tutto ok e non deve tornare un risultato annulla
          // l'assegnazione precedente
          result = gestComm::NoErr;
        clearErr(Buff_Lav, 1);
        }
      else {
        setErr(Buff_Lav, 1);
        while(WAIT_NEXT_BLOCK == data->responce) {
          CustomData->Pop();
          data = CustomData->getCurrData();
          if(!data || NO == data->responce)
            return gestComm::ReadFileErr;
          }
        if(NO != data->responce)
          sResponce = MAKE_FAILED_SEND;
        else
          sResponce = MAKE_SRESP(NO, gestComm::SendErr);
        result = gestComm::doResponce;
        }
      }
    CustomData->Pop();
    }
  return result;
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::write(gestComm::ComResult last)
{
  ComResult res = gestComm::NoErr;
  Dirty |= toBool(memcmp(Buff_Lav, Buff_Perif, MAX_DIM_PERIF));
  if(Dirty && !Logoff) {
    CopyMemory(Buff_Perif, Buff_Lav, MAX_DIM_PERIF);
    if(pMF) {
//      pMF->flush();
      res = NoErr;
      }
    else {
      P_File_Lock f(FileName);
      if(!f.isLocked())
        res = gestComm::OpenFileErr;
      else
        res = f.P_write(Buff_Lav, MAX_DIM_PERIF) ==
                        MAX_DIM_PERIF ?  NoErr : WriteFileErr;
       }
    }
  Dirty = false;
  if(gestComm::NoErr > last)
    return last;
  if(NoErr == res)
    res = last;
  return res;
}
//-----------------------------------------------------------------------------
uint gestCommgPerif::getMoreCycleCount() const
{
  uint count = CustomDataRead->getElem() + CustomDataWrite->getElem();
  if(!count) {
    gestCommgPerif* pt = const_cast<gestCommgPerif*>(this);
    pt->defSendCommand();
    count = CustomDataRead->getElem();
    }
  // uno in più così al termine ricarica, se ci sono meno dati di quelli normalmente in
  // lettura continua li somma, potrebbero esserci sono solo i dati a richiesta diretta
  // anche se è improbabile, visto che facendo un ciclo in più dovrebbe ricaricarli
  return count ?
    (int)count < numBuffBlockReq + numBuffBlock ? count + numBuffBlockReq + numBuffBlock + 1 : count + 1
    : baseClass::getMoreCycleCount();
}
//-----------------------------------------------------------------------------
bool load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name)
{
  infoFileCr result;
  result.header = (LPCBYTE)CRYPT_HEAD;
  result.lenHeader = DIM_HEAD;

  if(decryptFile(name, key, DIM_KEY_CRYPT, CRYPT_STEP, result)) {
#ifdef UNICODE
//  #error modificare ed adattare
#endif
  if (IsTextUnicode(result.buff, result.dim, 0))
    set.add(result.dim, (LPWSTR)result.buff, true);
  else
    set.add(result.dim, (LPSTR)result.buff, true);
    }
  else
    set.add(name + 1);
  return (LONG)result.crc >= 0;
}
//-----------------------------------------------------------------------------
void gestCommgPerif::setInitError()
{
  int addr = MAX_DIM_PERIF / DEF_TYPE_SIZE - 1;
  DWDATA* p = ((DWDATA*)Buff_Perif) + addr;
  *p = (DWORD)-1;
  p = ((DWDATA*)Buff_Lav) + addr;
  *p = (DWORD)-1;
  write(gestComm::OpenFileErr);
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::Init()
{
/*
// per bloccare il driver, per poter connettersi al processo e debuggare l'inizializzazione
  static bool inExec;
  if(!inExec) {
    inExec = true;
    MessageBox(0, _T("attesa"), _T(""), MB_OK);
    }
*/
  commIdentity *cI = getIdentity();
  openMapping(cI->useMappedFile());
  setInitError();
  TCHAR fileAddr[_MAX_PATH] = _T("#");
  _tcscpy_s(fileAddr + 1, SIZE_A(fileAddr) - 1, FileName);
  int len = _tcslen(fileAddr);
  for(int i = len - 1; i > 0; --i)
    if(_T('.') == fileAddr[i]) {
      fileAddr[i] = 0;
      _tcscat_s(fileAddr, ADDRFILE_EXT);
      break;
      }

  BYTE key[DIM_KEY_CRYPT + 2];
  wsprintfA((LPSTR)key, "p%s_%02d", "PRPH", UNFLAG_PERIF(flagCommand));

  setOfString set;
  load_CriptedTxt(key, set, fileAddr);
//  setOfString set(fileAddr);
  verifySet(set);

  LPCTSTR p = set.getString(ID_DATATYPE);
  if(!p) {
    MessageBox(0, fileAddr, _T("File AD7 mismatch"), MB_ICONSTOP | MB_OK);
    PostQuitMessage(0);
    return gestComm::NoErr;
    }
  p = set.getString(ID_VER_ADR);
  if(!p) {
    MessageBox(0, fileAddr, _T("Version Code not found"), MB_ICONSTOP | MB_OK);
    PostQuitMessage(0);
    return gestComm::NoErr;
    }
  if(CUR_VERSION < _ttoi(p)) {
    MessageBox(0, fileAddr, _T("Current version (") CUR_VERSION_TXT _T(") is older than that find"), MB_ICONSTOP | MB_OK);
    PostQuitMessage(0);
    return gestComm::NoErr;
    }
  p = set.getString(ID_TIMER_TICK);
  if(p)
    TimerTick = _ttoi(p);

  if(set.setFirst()) {
    int nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;//getTypeSize();

    Buff_Addr = new alternAddress[nElem];

    memset(Buff_Addr, 0, nElem * sizeof(*Buff_Addr));

    do {
      LPCTSTR p = set.getCurrString();
      int ix = set.getCurrId();
      if(ix > nElem) {
        if(ID_DATATYPE == ix)
          typeSize = _ttoi(p);
        }
      else
        stringToAddr(ix, nElem, p, Buff_Addr);
      } while(set.setNext());

    requestVarPresent = false;
    for(int i = 0; i < nElem; ++i) {
      if(Buff_Addr[i].action & REQ_ONLY_READ) {
        requestVarPresent = true;
        break;
        }
      }
    useBlockReq = checkUseReqRead();

    MSP = new manageSetupPrph(this, UNFLAG_PERIF(flagCommand));
    if(!MSP->loadSet(set)) {
      delete MSP;
      MSP = 0;
      }
//    else
//      MSP->loadFile();

    // lettura, nessun ritorno, inizio/fine, nessun id_req, flag prima lettura
    addCommand(readCustom, NO, MAKELPARAM(0, nElem - 1), 0, (actionFromFile)FIRST_READ);
    addCommand(readCustom, NO, MAKELPARAM(0, nElem - 1), 0, (actionFromFile)VAR_4_SETUP);

    int nAlloc = 100;
    addrToComm* tmp;
    do {
      tmp = new addrToComm[nAlloc];
      numBuffBlock = nAlloc;
      // se non riesce a caricare elimina il temporaneo e verifica se
      // è un problema di mancanza di buffer. In caso affermativo incrementa
      // la dimensione e riprova, altrimenti esce
      if(!LAddrToPAddrReceive(0, nElem - 1, tmp, numBuffBlock, ALWAYS_READ)) {
        delete []tmp;
        tmp = 0;
        if(numBuffBlock) {
          nAlloc += 50;
          continue;
          }
        break;
        }
      BuffBlock = new addrToComm[numBuffBlock];
      for(int i = 0; i < numBuffBlock; ++i)
        BuffBlock[i] = tmp[i];
      break;
      } while(true);

    if(tmp) {
      delete []tmp;
      performEndInit(set);
      return gestComm::NoErr;
      }

    return gestComm::OpenFileErr;
    }

  // nuova versione, deve esistere il file degli indirizzi
  return gestComm::OpenFileErr;
//  return gestComm::NoErr;
}
//-----------------------------------------------------------------
void gestCommgPerif::openMapping(bool useFileMapped)
{
  if(useFileMapped) {
    if(!pMF) {
      TCHAR full[_MAX_PATH];
      LPTSTR dummy;
      GetFullPathName(FileName, SIZE_A(full), full, &dummy);
      pMF = new p_MappedFile(full);
      if(!pMF->P_open(MAX_DIM_PERIF)) {
        delete pMF;
        pMF = 0;
        return;
        }
      delete []Buff_Perif;
      Buff_Perif = (BDATA*)pMF->getAddr();
      }
    // non dovrebbe verificarsi, se si chiude il driver viene chiuso anche il mapping
    else { // è necessario riaprirlo?
      Buff_Perif = 0;
      delete pMF;
      pMF = 0;
      openMapping(useFileMapped);
      }
    }
  else { // !useFileMapped
    if(pMF) {
      delete pMF;
      pMF = 0;
      Buff_Perif = new BDATA[MAX_DIM_PERIF];
      }
    // se non si usa il mapping e non era aperto il mapping non deve fare nulla
//    else {
//      }
    }
}
//-----------------------------------------------------------------------------
bool gestCommgPerif::orderSet(orderedPAddr& set, int init, int end, uint action)
{
  for(int i = init; i <= end; ++i) {
    if(NO == action || (Buff_Addr[i].action & action) == action) {
      if(Buff_Addr[i].dataType) {
        addrConv* p = new addrConv(Buff_Addr[i].ipAddr, Buff_Addr[i].port, Buff_Addr[i].db,
                      Buff_Addr[i].addr, i, Buff_Addr[i].dataType, toBool(Buff_Addr[i].action & 1));
        set.Add(p);
        }
      }
    }
  return set.setFirst();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool gestCommgPerif::isSimilar(uint type1, uint type2)
{
  // se uno è stringa e l'altro è bit è ok
  if(prfData::tStrData == type1 && prfData::tBitData == type2)
    return true;
  if(prfData::tStrData == type2 && prfData::tBitData == type1)
    return true;
  // impostiamo che che non sono uguali se hanno tipi diversi, se si impostano i min-max poi ci sono problemi
  if(type1 != type2)
    return false;
/*
  // se non hanno la stessa dimensione non è ok
  if(prfData::getNByte((prfData::tData)type1) != prfData::getNByte((prfData::tData)type2))
    return false;
*/
  // altrimenti sono compatibili
  return true;
}
//-----------------------------------------------------------------------------
bool gestCommgPerif::performLAddrToPAddr(orderedPAddr& set, addrToComm* LToP,
                int& maxVal, bool rec)
{
  int loaded = 0;
  int defTypeSz = DEF_TYPE_SIZE;//;
  bool ended = !set.setFirst();

  while(loaded < maxVal) {
    if(ended)
      break;
    addrConv* p = set.getCurrData();
    int typeSz = prfData::getNByte((prfData::tData)(p->type));
    if(prfData::tStrData == p->type || prfData::tBitData == p->type)
      typeSz = getTypeSize(); //defTypeSz;
    LToP[loaded].ipAddr = p->ipAddr;
    LToP[loaded].port = p->port;
    LToP[loaded].type = p->type;
    LToP[loaded].db = p->db;
    LToP[loaded].addr = p->pAddr;
    LToP[loaded].onCycleReading = p->onCycleReading;
    WORD* pw = (WORD*)LToP[loaded].buff;
    LPBYTE pb = LToP[loaded].buff;
    LToP[loaded].len = 1;
    // se ricezione carica gli indirizzi logici
    if(rec)
      *pw++ = (WORD)p->lAddr;
    // se trasmissione carica i dati da inviare denormalizzati
    else {
      int ix = p->lAddr;

      switch(p->type) {
        case prfData::tWData:
        case prfData::tDWData:
          do {
            DWORD t = *(DWDATA*)(Buff_Write + p->lAddr * defTypeSz);
            dwNormalizer n(Buff_Addr[ix]);
            n.getDenormalized(t);
            memcpy(pb, &t, typeSz);
            } while(false);
          break;

        case prfData::tWsData:
        case prfData::tDWsData:
          do {
            LONG t = *(DWsDATA*)(Buff_Write + p->lAddr * defTypeSz);
            dwsNormalizer n(Buff_Addr[ix]);
            n.getDenormalized(t);
            memcpy(pb, &t, typeSz);
            } while(false);
          break;

        case prfData::tFRData:
          do {
            fNormalizer n(Buff_Addr[ix]);
            fREALDATA t = *(fREALDATA*)(Buff_Write + p->lAddr * defTypeSz);
            n.getDenormalized(t);
            memcpy(pb, &t, typeSz);
            } while(false);
          break;

        case prfData::tRData:
          do {
            rNormalizer n(Buff_Addr[ix]);
            REALDATA t = *(REALDATA*)(Buff_Write + p->lAddr * defTypeSz);
            n.getDenormalized(t);
            memcpy(pb, &t, typeSz);
            } while(false);
          break;
        default:
          memcpy(pb, Buff_Write + p->lAddr * defTypeSz, typeSz);
          break;
        }

      pb += typeSz;
      }

    if(!set.setNext()) {
      ++loaded;
      break;
      }
    int step = getStep(p->type);
    do {
      ended = false;
      p = reinterpret_cast<addrConv*>(set.getCurr());

      // se uno è a lettura continua e l'altro no
      if(LToP[loaded].onCycleReading ^ p->onCycleReading)
        break;
      // se il successivo non è dello stesso tipo
      if(!isSimilar(p->type, LToP[loaded].type))
        break;
      // se non usa lo stesso ipAddr
      if(p->ipAddr != LToP[loaded].ipAddr)
        break;
      // se non usa la stessa porta
      if(p->port != LToP[loaded].port)
        break;
      // se non usa lo stesso db
      if(p->db != LToP[loaded].db)
        break;
      // se l'indirizzo non è immediatamente successivo
      if(p->pAddr > LToP[loaded].addr + step)
        break;

      // @@ Si è verificato un errore nell'immissione in un file .adr
      //    in cui venivano ripetuti alcuni indirizzi. Per evitare di
      //    andare oltre il buffer reale si verifica che l'indirizzo sia
      //    effettivamente successivo, altrimenti lo si ignora.

      // se l'indirizzo è immediatamente successivo.
      if(p->pAddr == LToP[loaded].addr + step) {
        ++LToP[loaded].len;
        step += getStep(p->type);

        if(rec)
          *pw++ = (WORD)p->lAddr;
        else {
          int ix = p->lAddr;

          switch(p->type) {
            case prfData::tWData:
            case prfData::tDWData:
              do {
                DWORD t = *(DWDATA*)(Buff_Write + p->lAddr * defTypeSz);
                dwNormalizer n(Buff_Addr[ix]);
                n.getDenormalized(t);
                memcpy(pb, &t, typeSz);
                } while(false);
              break;

            case prfData::tWsData:
            case prfData::tDWsData:
              do {
                LONG t = *(DWsDATA*)(Buff_Write + p->lAddr * defTypeSz);
                dwsNormalizer n(Buff_Addr[ix]);
                n.getDenormalized(t);
                memcpy(pb, &t, typeSz);
                } while(false);
              break;

            case prfData::tFRData:
              do {
                fNormalizer n(Buff_Addr[ix]);
                fREALDATA t = *(fREALDATA*)(Buff_Write + p->lAddr * defTypeSz);
                n.getDenormalized(t);
                memcpy(pb, &t, typeSz);
                } while(false);
              break;

            case prfData::tRData:
              do {
                rNormalizer n(Buff_Addr[ix]);
                REALDATA t = *(REALDATA*)(Buff_Write + p->lAddr * defTypeSz);
                n.getDenormalized(t);
                memcpy(pb, &t, typeSz);
                } while(false);
              break;
            default:
              memcpy(pb, Buff_Write + p->lAddr * defTypeSz, typeSz);
              break;
            }

          pb += typeSz;
          }
        int max_buff_t = MAX_BUFF_TRANSFER;
        if(max_buff_t <= (step / getStep(p->type)) * typeSz)
          break;
        }
      ended = true;
      } while(set.setNext());
    ++loaded;
    }

  if(loaded < maxVal) {
    maxVal = loaded;
    return true;
    }
  // se la lista non è terminata il buffer è troppo piccolo
  if(set.setNext())
    return false;

  return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool gestCommgPerif::LAddrToPAddrReceive(int init, int end, addrToComm* LToP,
                int& maxVal, uint action)
{
  if(!Buff_Addr) {
    maxVal = 0;
    return false;
    }

  orderedPAddr set(true);
  if(!orderSet(set, init, end, action)) {
    maxVal = 0;
    return false;
    }

  return performLAddrToPAddr(set, LToP, maxVal, true);
}
//-----------------------------------------------------------------------------
void gestCommgPerif::tranferToBuffWrite(int addr, LPBYTE pbuff, int len)
{
  int typeSz = prfData::getNByte((prfData::tData)(Buff_Addr[addr].dataType));
  addr *= DEF_TYPE_SIZE;
  const int nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  const int defTypeSize = max(DEF_TYPE_SIZE, typeSz);
  for(int i = 0; i < len && i < nElem; ++i, addr += defTypeSize, pbuff += typeSz) {
    *(LPDWORD)(Buff_Write + addr) = 0;
    CopyMemory(Buff_Write + addr, pbuff, typeSz);
    }
}
//-----------------------------------------------------------------------------
bool gestCommgPerif::LAddrToPAddrSend(addrToComm* LToP, int& maxVal, LPBYTE buffer, uint id)
{
  if(!Buff_Addr) {
    maxVal = 0;
    return false;
    }

  TRACE_SEND("init LAddrToPAddrSend")
  orderedPAddr set(true);
  sResponce = MAKE_NO_RESP;

  BDATA buff[MAX_DIM_FILE + 2];
  FillMemory(buff, sizeof(buff), -1);
  ComResult res = gestComm::NoErr;
  if(buffer && lenFile) {
    if(lenFile > MAX_DIM_FILE)
      return false;
    memcpy(buff, buffer, (size_t)lenFile);
    }
  else
    res = read(buff, id);
  if(gestComm::NoErr <= res && lenFile) {
    WDATA* pbuff = reinterpret_cast<WDATA*>(buff);
    if(flagCommand != *pbuff)  // flag non settato, dovrebbe essere un errore
      return false;

    TRACE_SEND("LAddrToPAddrSend p1")
    ++pbuff;
    lenFile -= 2;

    if(static_cast<int>(*pbuff) < 0)
      return false;

    TRACE_SEND("LAddrToPAddrSend p2")
    // in scrittura i dati sono sempre allineati alla dimensione del tipo
    // della periferica
    // nuove specifiche, il svisor usa sempre 32bit come base
    int typeSz = DEF_TYPE_SIZE; //getTypeSize();
    do {
      lenFile -= 4;
      if(long(lenFile) <= 0)  // non dovrebbe verificarsi
        break;

      TRACE_SEND("LAddrToPAddrSend p3")
      int init = *pbuff++;
      int end = *pbuff++;

      DWORD len = end - init + 1;
      tranferToBuffWrite(init, (LPBYTE)pbuff, len);
      len *= typeSz;
      pbuff = (WDATA*)((BDATA*)pbuff + len);
      lenFile -= len;

      if(!orderSet(set, init, end, NO)) {
        maxVal = 0;
        return false;
        }
      TRACE_SEND("LAddrToPAddrSend p4")
      } while(long(lenFile) >= 6); // due indirizzi più almeno un dato
    }

  bool success = performLAddrToPAddr(set, LToP, maxVal, false);
  return success;
}
//-----------------------------------------------------------------------------
gestComm::ComResult gestCommgPerif::defSendCommand()
{
  CustomQueue* CustomData = CustomDataRead;
  uint nElem = OneRead.getElem();
  for(uint j = 0; j < nElem; ++j) {
    const oneRead& or = OneRead[j];
    for(int i = 0; i < or.numBlockReq; ++i) {
      pCustomData data = CustomData->getFree();
      data->Clear();
      data->responce = NO;
      data->data = or.BlockReq[i];
      data->U.type = tRead;
      CustomData->Push(data);
      }
    delete []OneRead[j].BlockReq;
    // non serve perché il vettore viene resettato, ma per sicurezza
    OneRead[j].numBlockReq = 0;
    }
  OneRead.reset();

  if(BuffBlockReq && numBuffBlockReq) {
    for(int i = 0; i < numBuffBlockReq; ++i) {
      pCustomData data = CustomData->getFree();
      data->Clear();
      data->responce = NO;
      data->data = BuffBlockReq[i];
      data->U.type = tRead;
      CustomData->Push(data);
      }
    }

  if(BuffBlock && numBuffBlock) {
    for(int i = 0; i < numBuffBlock; ++i) {
      pCustomData data = CustomData->getFree();
      data->Clear();
      data->responce = NO;
      data->data = BuffBlock[i];
      data->U.type = tRead;
      CustomData->Push(data);
      }
    }
  return gestComm::noAction;
}
//-----------------------------------------------------------------------------
void gestCommgPerif::ScratchToLav(addrToComm* LToP)
{
  int len = LToP->len;
  int typeDefSz = DEF_TYPE_SIZE;
  int typeSz = prfData::getNByte((prfData::tData)(LToP->type));
  if(prfData::tStrData == LToP->type || prfData::tBitData == LToP->type)
    typeSz = getTypeSize();
  WDATA* pw = reinterpret_cast<WDATA*>(LToP->buff);
  BDATA* pb = Buff_Scratch;
  for(int i = 0; i < len; ++i, pb += typeSz) {
    int ix = pw[i];
    int addr = pw[i] * typeDefSz;

    switch(LToP->type) {
      case prfData::tWData:
        do {
          dwNormalizer n(Buff_Addr[ix]);
          DWDATA t = *(WDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeDefSz);
          } while(false);
        break;

      case prfData::tWsData:
        do {
          dwsNormalizer n(Buff_Addr[ix]);
          DWsDATA t = *(WsDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeDefSz);
          } while(false);
        break;

      case prfData::tDWData:
        do {
          dwNormalizer n(Buff_Addr[ix]);
          DWDATA t = *(DWDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeDefSz);
          } while(false);
        break;

      case prfData::tDWsData:
        do {
          dwsNormalizer n(Buff_Addr[ix]);
          DWsDATA t = *(DWsDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeDefSz);
          } while(false);
        break;

      case prfData::tFRData:
        do {
          fNormalizer n(Buff_Addr[ix]);
          fREALDATA t = *(fREALDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeDefSz);
          } while(false);
        break;

      case prfData::tRData:
        do {
          rNormalizer n(Buff_Addr[ix]);
          REALDATA t = *(REALDATA*)pb;
          n.getNormalized(t);
          CopyMemory(Buff_Lav + addr, &t, typeSz);
          } while(false);
        break;
      default:
        CopyMemory(Buff_Lav + addr, pb, typeSz);
        break;
      }
    if(MSP && !MSP->endedRead())
      MSP->checkReadedVar(Buff_Lav + addr, typeSz, LToP, i * getStep(LToP->type));
    }
  if(MSP && MSP->endedRead() && !MSP->committed()) {
    MSP->loadFile();
    PVect<transf4Send> SetSetup;
    MSP->getValToSend(SetSetup);
    uint nElem = SetSetup.getElem();
    if(nElem) {
      LPBYTE buff = new BYTE[MAX_DIM_FILE + 2];
      LPWORD p = (LPWORD)buff;
      *p++ = (WORD)flagCommand;
      const PVect<transf4Send>& set = SetSetup;
      for(uint i = 0; i < nElem; ++i) {
        *p++ = set[i].addr;
        *p++ = set[i].addr;
        *(LPDWORD)p = set[i].value;
        p += sizeof(DWORD) / sizeof(*p);
        if((LPBYTE)p - buff >= MAX_DIM_FILE - 8)
          break;
        }
      lenFile = (LPBYTE)p - buff;
      addCommand(writeCustom, NO, 0, 0, NO, buff);
      delete []buff;
      }
    nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
    addCommand(readCustom, NO, MAKELPARAM(0, nElem - 1), 0, VAR_4_SETUP);
    waitForReaded();
    }
}
//-----------------------------------------------------------------------------
void gestCommgPerif::addSingleWrite(DWORD data, WORD addr)
{
  BYTE buff[16];
  LPWORD p = (LPWORD)buff;
  *p++ = (WORD)flagCommand;
  *p++ = addr;
  *p++ = addr;
  *(LPDWORD)p = data;
  lenFile = sizeof(WORD) * 3 + sizeof(DWORD);
  addCommand(writeCustom, NO, 0, 0, NO, buff);
}
//-----------------------------------------------------
void gestCommgPerif::verifySet(setOfString& set)
{
  // ora il tipo di dato viene letto direttamente dal set
}
//-----------------------------------------------------------------
bool gestCommgPerif::checkUseReqRead()
{
  uint nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  for(uint i = 0; i < nElem; ++i)
    if((Buff_Addr[i].action & REQ_ONLY_READ) == REQ_ONLY_READ && Buff_Addr[i].dataType)
      return true;
  return false;
}
//-----------------------------------------------------------------
static void insertOnList(PVect<uint>& set, uint val)
{
  uint n = set.getElem();
  int pos = n;
  for(uint i = 0; i < n; ++i) {
    if(val == set[i])
      return;
    if(val < set[i]) {
      pos = i;
      break;
      }
    }
  if(pos == n)
    set[pos] = val;
  else
    set.insert(val, pos);
}
//-----------------------------------------------------------------
//#define TRACE_FILE
#ifdef TRACE_FILE
  #include "P_Date.h"
#endif
//-----------------------------------------------------------------
bool gestCommgPerif::addReadReq(WORD wP, DWORD lP)
{
  if(!useBlockReq)
    return false;

#ifdef TRACE_FILE
// /H137116 /O0 /I3 /C704609
  TCHAR buff[512];
  P_File* pf = new P_File(_T("trace.txt"));
  if(!pf->P_open()) {
    delete pf;
    pf = 0;
    }
  else
    pf->P_seek(0, SEEK_END_);

  if(pf) {
    wsprintf(buff, _T("entrata - %d - %p wp=%d, lp=%d\r\n"), numBuffBlockReq, BuffBlockReq, wP, lP);
    pf->P_writeString(buff);
    }

  addrToComm* pt = BuffBlockReq;
  BuffBlockReq = 0;
  numBuffBlockReq = 0;

  if(pf) {
    wsprintf(buff, _T("delete - %d - %p wp=%d, lp=%d\r\n"), numBuffBlockReq, pt, wP, lP);
    pf->P_writeString(buff);
    }

  if(!wP && !lP) {
    onReqRead.reset();
    if(pf)
      delete pf;
    delete []pt;
    return false;
    }

  TCHAR name[_MAX_PATH];
  MAKE_MAPPED_FILENAME(name, wP, lP);
  if(pf) {
    wsprintf(buff, _T("nameMapped - %s\r\n"), name);
    pf->P_writeString(buff);
    }
//  p_MappedFile mp(name, mP_READ_ONLY);
  p_MappedFile mp(name, mP_MEM_ONLY);
  if(!mp.P_open()) {
    if(pf) {
      wsprintf(buff, _T("failed open - %s\r\n"), name);
      pf->P_writeString(buff);
      delete pf;
      }
    delete []pt;
    return true;
    }
  tWrapMappedFile<DWORD> tmp(mp);
  DWORD nElem = tmp[0];

  if(pf) {
    pf->P_writeString(_T("request\r\n"));
    }
  for(uint j = 1; j <= nElem; ++j) {
    uint i = tmp[j];
    if(pf) {
      wsprintf(buff, _T("%d "), i);
      pf->P_writeString(buff);
      }
    if((Buff_Addr[i].action & REQ_ONLY_READ) == REQ_ONLY_READ) {
      if(Buff_Addr[i].dataType)
        insertOnList(onReqRead, i);
      }
    }
  if(pf) {
    pf->P_writeString(_T("\r\n"));
    }

  nElem = onReqRead.getElem();

  orderedPAddr set(true);

  if(pf) {
    SYSTEMTIME stm;
    GetLocalTime(&stm);
    set_format_data(buff, stm, European, _T(" - "));
    _tcscat(buff, _T("\r\n"));
    pf->P_writeString(buff);
    pf->P_writeString(name);
    pf->P_writeString(_T("\r\n"));
    }
  for(uint j = 0; j < nElem; ++j) {
    uint i = onReqRead[j];
    addrConv* p = new addrConv(Buff_Addr[i].ipAddr, Buff_Addr[i].port, Buff_Addr[i].db,
                      Buff_Addr[i].addr, i, Buff_Addr[i].dataType);
    set.Add(p);
    if(pf) {
      wsprintf(buff, _T("%d "), p->pAddr);
      pf->P_writeString(buff);
      }
    }
  if(pf) {
    pf->P_writeString(_T("\r\n"));
    delete pf;
    }
  delete []pt;

#else
  numBuffBlockReq = 0;
  delete []BuffBlockReq;
  BuffBlockReq = 0;

  if(!wP && !lP) {
    onReqRead.reset();
    return false;
    }

  TCHAR name[_MAX_PATH];
  MAKE_MAPPED_FILENAME(name, wP, lP);
  p_MappedFile mp(name, mP_READ_ONLY);
//  p_MappedFile mp(name, mP_MEM_ONLY);
  if(!mp.P_open())
    return true;

  tWrapMappedFile<DWORD> tmp(mp);
  DWORD nElem = tmp[0];

  for(uint j = 1; j <= nElem; ++j) {
    uint i = tmp[j];
    if((Buff_Addr[i].action & REQ_ONLY_READ) == REQ_ONLY_READ) {
      if(Buff_Addr[i].dataType)
        insertOnList(onReqRead, i);
      }
    }

  nElem = onReqRead.getElem();

  orderedPAddr set(true);
  for(uint j = 0; j < nElem; ++j) {
    uint i = onReqRead[j];
    addrConv* p = new addrConv(Buff_Addr[i].ipAddr, Buff_Addr[i].port, Buff_Addr[i].db,
                      Buff_Addr[i].addr, i, Buff_Addr[i].dataType);
    set.Add(p);
    }
#endif

  if(!set.setFirst())
    return true;
  nElem = set.getNumElem();

  BuffBlockReq = new addrToComm[nElem];
  numBuffBlockReq = nElem;

  performLAddrToPAddr(set, BuffBlockReq, numBuffBlockReq, true);
  return true;
}
//-----------------------------------------------------------------
bool gestCommgPerif::addOneReadReq(WORD wP, DWORD lP)
{
  if(!useBlockReq)
    return false;

  if(!wP && !lP)
    return false;

  TCHAR name[_MAX_PATH];
  MAKE_MAPPED_FILENAME(name, wP, lP);
  p_MappedFile mp(name, mP_READ_ONLY);
  if(!mp.P_open())
    return true;

  tWrapMappedFile<DWORD> tmp(mp);
  DWORD nElem = tmp[0];

  PVect<uint> onReqOneRead;

  for(uint j = 1; j <= nElem; ++j) {
    uint i = tmp[j];
    if((Buff_Addr[i].action & REQ_ONLY_READ) == REQ_ONLY_READ) {
      if(Buff_Addr[i].dataType)
        insertOnList(onReqOneRead, i);
      }
    }

  nElem = onReqOneRead.getElem();

  orderedPAddr set(true);
  for(uint j = 0; j < nElem; ++j) {
    uint i = onReqOneRead[j];
    addrConv* p = new addrConv(Buff_Addr[i].ipAddr, Buff_Addr[i].port, Buff_Addr[i].db,
                      Buff_Addr[i].addr, i, Buff_Addr[i].dataType);
    set.Add(p);
    }

  if(!set.setFirst())
    return true;
  nElem = set.getNumElem();

  uint ix = OneRead.getElem();

  OneRead[ix].BlockReq = new addrToComm[nElem];
  OneRead[ix].numBlockReq = nElem;

  performLAddrToPAddr(set, OneRead[ix].BlockReq, OneRead[ix].numBlockReq, true);
  return true;
}
//------------------------------------------------------------------
void gestCommgPerif::deleteFile4Write(uint id)
{
  TCHAR path[_MAX_PATH];
  MK_NAME_FILE_COMMAND(path, UNFLAG_PERIF(flagCommand), id);
  DeleteFile(path);
}
//------------------------------------------------------------------
void gestCommgPerif::makeFile4Write(LPTSTR path, uint id)
{
  MK_NAME_FILE_COMMAND(path, UNFLAG_PERIF(flagCommand), id);
}
