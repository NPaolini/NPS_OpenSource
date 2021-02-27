//--------------- prph_modbus_Exclude.cpp -------------------
//----------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>

#include "prph_modbus.h"
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"
#include "prfData.h"
#include "modBus_ser.h"
#include "modBus_lan.h"

#include "p_txt.h"
//---------------------------------------------------------
extern bool isValidAddr(DWORD addr);
//---------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("prph_modbus [%d]"), idPrf);
    w->setCaption(buff);
    }
  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new prph_modbus(name, FLAG_PERIF(idPrf));
}
//-----------------------------------------------------
#define MAX_LEN 2
//-----------------------------------------------------
prph_modbus::prph_modbus(LPCTSTR file, WORD flagCommand) :
    gestCommgPerif(file, flagCommand), maxLenData(MAX_LEN) { }
//-----------------------------------------------------
prph_modbus::~prph_modbus()
{
  uint nElem = modBusSet.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete modBusSet[i].mBus;
}
//-----------------------------------------------------
//----------------------------------------------------------
void prph_modbus::addToData(const alternAddress& tAddr)
{
  if((!tAddr.ipAddr || !tAddr.port) && !tAddr.db)
    return;
  uint nElem = modBusSet.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(tAddr.ipAddr == modBusSet[i].IP && tAddr.port == modBusSet[i].Port)
      return;
    if(tAddr.db && modBusSet[i].IP && !modBusSet[i].Port)
      return;
    }
  if(tAddr.ipAddr && tAddr.port)
    modBusSet[nElem] = modBusInfo(tAddr.ipAddr, tAddr.port);
  else
    modBusSet[nElem] = modBusInfo(tAddr.db, 0);
}
//----------------------------------------------------------
void prph_modbus::verifySet(setOfString& set)
{
  baseClass::verifySet(set);

  if(!set.setFirst())
    return;

  alternAddress tmpAddr;

  int nElem = MAX_DIM_PERIF / getTypeSize();

  do {
    if(set.getCurrId() >= nElem)
      continue;
    LPCTSTR p = set.getCurrString();
    stringToAddr(1, 1, p, &tmpAddr);

    addToData(tmpAddr);

    } while(set.setNext());

  set.setFirst();
}
//-----------------------------------------------------
gestComm::ComResult prph_modbus::Init()
{
  gestComm::ComResult result = baseClass::Init();
  uint nElem = modBusSet.getElem();

  // non c'è nessuna informazione, né sulla lan, né sui device
  if(!nElem)
    return gestComm::OpenComErr;

  commIdentity *cI = getIdentity();
  maxLenData = cI->getData();
  bool fullSwap = true;
  if(long(maxLenData) < 0) {
    maxLenData = -long(maxLenData);
    fullSwap = false;
    }
  if(!maxLenData)
    maxLenData = MAX_LEN;

  for(uint i = 0; i < nElem; ++i) {
    DWORD ip = reverseIP(modBusSet[i].IP);
    DWORD port = modBusSet[i].Port;
    if(port)
      modBusSet[i].mBus = new gModBusLan(ip, port);
    else {
      commIdentity *cI = getIdentity();
      DWORD ipc = cI->getPCom();
      modBusSet[i].mBus = new gModBusSer(paramConn(ipc));
      }
    modBusSet[i].mBus->setFullSwap(fullSwap);

    if(!modBusSet[i].mBus->init())
      return gestComm::OpenComErr;
    }
  return result;
}
//-------------------------------------------------------
gModBus* prph_modbus::getModBus(const addrToComm* pAddr)
{
  uint nElem = modBusSet.getElem();
  for(uint i = 0; i < nElem; ++i) {
    const DWORD ip = modBusSet[i].IP;
    const DWORD port = modBusSet[i].Port;
    if(port) {
      if(pAddr->ipAddr == ip && pAddr->port == port)
        return modBusSet[i].mBus;
      }
    else if(pAddr->db && ip)
      return modBusSet[i].mBus;
    }
  return 0;
}
//-------------------------------------------------------
gestComm::ComResult prph_modbus::Send(const addrToComm*, Perif_TypeCommand)
{
  return gestComm::NoErr;
}
//-------------------------------------------------------
#define NBIT_4_VAR 32
#define MAX_VAR_4_ERR 2
#define MAX_DB (NBIT_4_VAR * MAX_VAR_4_ERR)
//-------------------------------------------------------
#define DEF_ADDR_4_STD_ERR (MAX_DIM_PERIF / DEF_TYPE_SIZE - 1)
//-------------------------------------------------------
gestComm::ComResult prph_modbus::ReceiveError(const addrToComm* pAddr, bool set)
{
  uint db = pAddr->db - 1;
  if(db >= MAX_DB)
    return set ? gestComm::RecErr : gestComm::noAction;

  int addr = DEF_ADDR_4_STD_ERR - MAX_VAR_4_ERR;
  while(db >= NBIT_4_VAR) {
    ++addr;
    db -= NBIT_4_VAR;
    }
  DWDATA* p = ((DWDATA*)Buff_Lav) + addr;
  if(set)
    *p |= 1 << db;
  else
    *p &= ~(1 << db);
  return  set ? gestComm::RecErr : gestComm::noAction;
}
//-------------------------------------------------------
gestComm::ComResult prph_modbus::Receive(const addrToComm* pAddr, BDATA* readBuff)
{
  if(Logoff)
    return gestComm::NoErr;

  if(!isValidAddr(pAddr->db))
    return ReceiveError(pAddr, false);

  gModBus* mBus = getModBus(pAddr);
  if(!mBus)
    return ReceiveError(pAddr, false);

  uint size = prfData::getNByte((prfData::tData)pAddr->type);
  if(!size || prfData::tStrData == pAddr->type)
    size = getTypeSize();

  sModBus smb;
  smb.addr = pAddr->addr / 2;
  DWORD maxData = pAddr->len * size / getTypeSize();
  smb.device = pAddr->db & 0xff;
  smb.dimType = size;

  BDATA* pData = readBuff;
  DWORD offset = 0;
  while(true) {
    smb.len = (WORD)min(maxData, maxLenData);
    maxData -= smb.len;

    if(!mBus->read(smb, pData + offset))
      return ReceiveError(pAddr, true);

    if(!maxData)
      break;
    offset += smb.len * getTypeSize();
    smb.addr += smb.len;
    }
  ReceiveError(pAddr, false);
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult prph_modbus::SendData(addrToComm* pAddr)
{
  if(!isValidAddr(pAddr->db))
    return gestComm::NoErr;

  gModBus* mBus = getModBus(pAddr);
  if(!mBus)
    return gestComm::SendErr;

  int init = pAddr->addr;
  uint size = prfData::getNByte((prfData::tData)pAddr->type);
  if(!size || prfData::tStrData == pAddr->type)
    size = getTypeSize();

  sModBus smb;
  smb.addr = pAddr->addr / 2;
  DWORD maxData = pAddr->len * size / getTypeSize();
  smb.device = pAddr->db & 0xff;
  smb.dimType = size;
  smb.writeSingle = pAddr->db >= 256;

  BDATA* pData = pAddr->buff;
  DWORD offset = 0;
  while(true) {
    smb.len = (WORD)min(maxData, maxLenData);
    maxData -= smb.len;

    if(!mBus->write(smb, pData + offset))
      return gestComm::SendErr;

    if(!maxData)
      break;
    offset += smb.len * getTypeSize();
    smb.addr += smb.len;
    }
  return gestComm::NoErr;
}//-------------------------------------------------------
//-----------------------------------------------------

