//--------------- prf_simulation_v7.CPP -------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#include "pwin.h"

#include "hdrmsg.h"
#include "prfdata.h"
#include "prph_simulation_v7.h"
#include "file_lck.h"
#include "commidt.h"
#include "p_txt.h"
#include "p_util.h"
//---------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("Simulation Device [%d] V7"), idPrf);
    w->setCaption(buff);
    }
  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new simulation_Driver(name, FLAG_PERIF(idPrf), idPrf);
}
//-----------------------------------------------------
//-----------------------------------------------------
simulation_Driver::simulation_Driver(LPCTSTR file, WORD flagCommand, uint idPrf) :
    baseClass(file, flagCommand), idPrf(idPrf),
    hThread(0), hEventClose(0), onWrite(0), simulErr(false) {   }
//-----------------------------------------------------
simulation_Driver::~simulation_Driver()
{
  if(hEventClose) {
    SetEvent(hEventClose);
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    CloseHandle(hEventClose);
    }
}
//-----------------------------------------------------
gestComm::ComResult simulation_Driver::Init()
{
  commIdentity* ci = getIdentity();
  ci->forceMappedFile(false);
  if(12345 == ci->getData())
    simulErr = true;
  hEventClose = CreateEvent(0, TRUE, 0, 0);
  if(hEventClose) {
    DWORD idThread;
    hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderProc, this, 0, &idThread);
    if(!hThread) {
      CloseHandle(hEventClose);
      hEventClose = 0;
      }
    }
  P_File pfl(FileName, P_READ_ONLY);
  if(pfl.P_open()) {
    pfl.P_read(Buff_Lav, MAX_DIM_PERIF);
    memcpy(Buff_Perif, Buff_Lav, MAX_DIM_PERIF);
    memcpy(Buff_Write, Buff_Lav, MAX_DIM_PERIF);
    }
  gestComm::ComResult result = baseClass::Init();

  return result;
}
#define RET_CMP(a) \
  diff = d2.a - d1.a;  \
  if(diff) \
    return diff
//-------------------------------------------------------
int dualCmp(const dual& d1, const dual& d2)
{
  int diff;
  RET_CMP(db);
  RET_CMP(ipAddr);
  RET_CMP(port);
  RET_CMP(addr);
  return 0;
}
//-------------------------------------------------------
void simulation_Driver::makeDual(setOfString& set)
{
  const uint nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  set.setFirst();
  uint nDual = 0;
  alternAddress tmpAddr;
  do {
    if(set.getCurrId() >= nElem)
      break;
    LPCTSTR p = set.getCurrString();
    stringToAddr(1, 1, p, &tmpAddr);
    dual d;
    d.logicAddr = set.getCurrId() - 1;
    d.ipAddr = tmpAddr.ipAddr;
    d.port = tmpAddr.port;
    d.db = tmpAddr.db;
    d.addr = tmpAddr.addr;
    dualSet.insertEx(d, dualCmp);
    } while(set.setNext());

  set.setFirst();
}
//-------------------------------------------------------
void simulation_Driver::verifySet(setOfString& set)
{
  baseClass::verifySet(set);
  // se ci sono dati esiste un file .adr, si usa quello
  if(set.setFirst()) {
    makeDual(set);
    return;
    }
  // altrimenti occorre creare tutto il set di indirizzi ed il codice per il tipo di dato
  set.replaceString(ID_DATATYPE, str_newdup(_T("4")), true);
  set.replaceString(ID_VER_ADR, str_newdup(CUR_VERSION_TXT), true);

  int nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  TCHAR buff[250];
  for(int i = 0; i < nElem; ++i) {
    wsprintf(buff, _T("0,%d,4,1,0,0,0,0,0,0"), i);
    set.replaceString(i + 1, str_newdup(buff), true);
    }
  makeDual(set);
}
//-------------------------------------------------------
gestComm::ComResult simulation_Driver::Send(const addrToComm* , Perif_TypeCommand )
{
  return gestComm::NoErr;
}
//-----------------------------------------------------
bool simulation_Driver::addCommand(gPerif_Command cmd, WORD p1, DWORD p2, DWORD id_req, actionFromFile action, LPBYTE buffer)
{
  if(writeCustom <= cmd) {
    criticalLock cs(cS);
    ++onWrite;
    }

  return baseClass::addCommand(cmd, p1, p2, id_req, action, buffer);
}
//-------------------------------------------------------
gestComm::ComResult simulation_Driver::write(ComResult last)
{
  criticalLock cs(cS);
  if(onWrite)
    Dirty = true;

  gestComm::ComResult result = baseClass::write(last);

  if(onWrite)
    --onWrite;
  return result;
}
//-----------------------------------------------------
void simulation_Driver::saveData(const addrToComm* pAddr, uint offs, const dual& d, uint size)
{
  LPDWORD pdw = (LPDWORD)Buff_Lav;
  CopyMemory(pdw + d.logicAddr, pAddr->buff + offs * size, size);
}
//-----------------------------------------------------
void simulation_Driver::loadData(const dual& d, BDATA* buff, uint size)
{
  LPDWORD pdw = (LPDWORD)Buff_Lav;
  CopyMemory(buff, pdw + d.logicAddr, size);
}
//-------------------------------------------------------
gestComm::ComResult simulation_Driver::SendData(addrToComm* pAddr)
{
  // non dovrebbe servire il lock perché è attiva la onWrite
  criticalLock cs(cS);
  uint size = prfData::getNByte((prfData::tData)(pAddr->type));
  if(!size)
    size = getTypeSize();

  dual d;
  d.ipAddr = pAddr->ipAddr;
  d.port = pAddr->port;
  d.db = pAddr->db;
  d.addr = pAddr->addr;
  uint pos;
  for(uint i = 0; i < pAddr->len; ++i) {
    if(dualSet.find(d, pos, dualCmp))
      saveData(pAddr, i, dualSet[pos], size);
    d.addr += size;
    }
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult simulation_Driver::Receive(const addrToComm* pAddr, BDATA* buff)
{  
  if(simulErr)
    return gestComm::RecErr;
  uint size = prfData::getNByte((prfData::tData)(pAddr->type));
  if(!size)
    size = getTypeSize();

  dual d;
  d.ipAddr = pAddr->ipAddr;
  d.port = pAddr->port;
  d.db = pAddr->db;
  d.addr = pAddr->addr;
  uint pos;
  for(uint i = 0; i < pAddr->len; ++i) {
    if(dualSet.find(d, pos, dualCmp))
      loadData(dualSet[pos], buff + i * size, size);
    d.addr += size;
    }
  return gestComm::NoErr;
}
//-----------------------------------------------------
void simulation_Driver::reloadData()
{
  if(pMF)
    return;
  criticalLock cs(cS);
  if(onWrite)
    return;
  P_File pf(FileName, P_READ_ONLY);
  if(pf.P_open()) {
    pf.P_read(Buff_Lav, MAX_DIM_PERIF);
    memcpy(Buff_Perif, Buff_Lav, MAX_DIM_PERIF);
    memcpy(Buff_Write, Buff_Lav, MAX_DIM_PERIF);
    }
}
//------------------------------------------------
unsigned FAR PASCAL ReaderProc(void* cl)
{
  simulation_Driver* Cl = reinterpret_cast<simulation_Driver*>(cl);

  for(;;) {
    DWORD result = WaitForSingleObject(Cl->hEventClose, 2000);
    if(WAIT_TIMEOUT == result)
      Cl->reloadData();
    else
      break;
    }
  return 0;
}
//-----------------------------------------------------
//-----------------------------------------------------

