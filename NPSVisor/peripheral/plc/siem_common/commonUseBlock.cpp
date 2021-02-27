//--------------- commonUseBlock.cpp -----------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
#include "commonUseBlock.h"
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"

#include "prfData.h"
#include "p_txt.h"
#include "p_file.h"
//----------------------------------------------------------
//#define TEST_NO_CONN
//----------------------------------------------------------
// azzera sempre dopo lo shift
// non dovrebbe essere necessario se valori unsigned
  #define SWAB__S(v) ( (((v) >> 24) & 0xff)     | \
                      (((v) << 24) & 0xff000000)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
// se valori unsigned
  #define SWAB__(v) ( ((v) >> 24)     | \
                      ((v) << 24)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
//----------------------------------------------------------
static void mySwab(void *from, void *to, int n)
{
  n >>= 2;
  DWDATA *t = reinterpret_cast<DWDATA*>(to);
  DWDATA *s = reinterpret_cast<DWDATA*>(from);
  for(int i = 0; i < n; ++i)
    t[i] = SWAB__(s[i]);
}
//----------------------------------------------------------
commonUseBlock::commonUseBlock(LPCTSTR file, WORD flagCommand) :
    gestCommgPerif(file, flagCommand), hThread(0), errorByThread(false),
    realErrorReceive(0), realTotCount(0),  hEventReady(CreateEvent(0, 1, 0, 0)),
    realConsErr(0) { }
//----------------------------------------------------------
commonUseBlock::~commonUseBlock()
{
  stopThread();
}
//----------------------------------------------------------
void commonUseBlock::stopThread()
{
  Logoff = true;
  if(hThread) {
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    hThread = 0;
    }
  if(hEventReady) {
    CloseHandle(hEventReady);
    hEventReady = 0;
    }
}
//----------------------------------------------------------
bool commonUseBlock::manageCounter(int& countSend, int& countRec, int& countTot, int& consErr)
{
  criticalLock crtLck(criticalSectionBuff);
  countRec = realErrorReceive;
  countTot = realTotCount + countSend;
  consErr = realConsErr;
  return true;
}
//----------------------------------------------------------
void assignAddrAndLen(PVect<tempAddr>& pvT, int minAddr, int maxAddr, int lastCheck, int lastDb, int& num)
{
  bool found = false;
  for(int j = 0; j < num; ++j) {
    if(pvT[j].db == lastDb && pvT[j].V_CHECK == lastCheck) {
      if(pvT[j].addr > minAddr)
        pvT[j].addr = minAddr;
      if(pvT[j].len < maxAddr - pvT[j].addr)
        pvT[j].len =  maxAddr - pvT[j].addr;
      found = true;
      break;
      }
    }
  if(!found) {
    pvT[num].V_CHECK = lastCheck;
    pvT[num].db = lastDb;
    pvT[num].addr = minAddr;
    pvT[num].len =  maxAddr - minAddr;
    ++num;
    }
}
//----------------------------------------------------------
void commonUseBlock::verifySet(setOfString& set)
{
  baseClass::verifySet(set);

  if(!set.setFirst())
    return;

  PVect<tempAddr> pvT;

  uint minAddr = 0x7fffff;
  uint maxAddr = 0;
  int lastDb  = -1;
  int lastCheck = -1;
  int num = 0;

  alternAddress tmpAddr;

  int nElem = MAX_DIM_PERIF / getTypeSize();

  do {
    if(set.getCurrId() >= nElem)
      continue;
    LPCTSTR p = set.getCurrString();
    stringToAddr(1, 1, p, &tmpAddr);

    if(tmpAddr.action) {
      if(tmpAddr.db != lastDb || tmpAddr.V_CHECK != lastCheck) {
        if(lastDb >= 0 || lastCheck >= 0)
          assignAddrAndLen(pvT, minAddr, maxAddr, lastCheck, lastDb, num);

        lastDb = tmpAddr.db;
        lastCheck = tmpAddr.V_CHECK;
        minAddr = 0x7fffff;
        maxAddr = 0;
        }
      if(tmpAddr.addr < minAddr)
        minAddr = tmpAddr.addr;
      int lenData = prfData::getNByte((prfData::tData)tmpAddr.dataType);
      if(tmpAddr.addr + lenData > maxAddr)
        maxAddr = tmpAddr.addr + lenData;
      }
    } while(set.setNext());

  if(lastCheck < 0)
    lastCheck = 0;
  if(lastDb < 0)
    lastDb = 0;
  assignAddrAndLen(pvT, minAddr, maxAddr, lastCheck, lastDb, num);

  pvFAddr.setDim(num);
  for(int i = 0; i < num; ++i)
    pvFAddr[i] = pvT[i];
  set.setFirst();
}
//----------------------------------------------------------
gestComm::ComResult commonUseBlock::Send(const addrToComm*, Perif_TypeCommand )
{
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult commonUseBlock::errorInit()
{
  Logoff = true;
  startTread();
  return gestComm::OpenComErr;
}
//----------------------------------------------------------
gestComm::ComResult commonUseBlock::Init()
{
  gestComm::ComResult result = baseClass::Init();
  if(gestComm::NoErr == result) {
    DWORD idThread;
    hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderProc, this, CREATE_SUSPENDED, &idThread);

    if(!hThread)
      result = gestComm::OpenComErr;
    }
  return result;
}
//----------------------------------------------------------
gestComm::ComResult commonUseBlock::SendData(addrToComm* pAddr)
{
  if(Logoff)
    return gestComm::noAction;

  uint size = GET_NBYTE(pAddr->type);
  if(!size || prfData::tStrData == pAddr->type || prfData::tBitData == pAddr->type)
    size = getTypeSize();
  int dim = pAddr->len * size;

  if(prfData::tStrData != pAddr->type && prfData::tBitData != pAddr->type)
    switch(size) {
      case 0:
      case 1:
        break;
      case 2:
        _swab((char*)pAddr->buff, (char*)pAddr->buff, dim);
        break;
      case 4:
        mySwab(pAddr->buff, pAddr->buff, dim);
        break;
      }
  int res;
  do {
    criticalLock crtLckW(criticalSectionConn);
    res = performSend(pAddr->V_CHECK, pAddr->db, pAddr->addr, dim, pAddr->buff);
    } while(false);
  if(res)
    return gestComm::SendErr;

  return gestComm::NoErr;
}
//----------------------------------------------------------
gestComm::ComResult commonUseBlock::Receive(const addrToComm* pAddr, BDATA* buff)
{
  if(Logoff)
    return gestComm::noAction;
  if(pAddr->onCycleReading) {
    DWORD result = waitForReady();
    if(WAIT_OBJECT_0 != result)
      return gestComm::noAction;
    }
  uint size = GET_NBYTE(pAddr->type);
  if(!size || prfData::tStrData == pAddr->type || prfData::tBitData == pAddr->type)
    size = getTypeSize();
  uint dim = pAddr->len * size;

  uint nElem = pvFAddr.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(pAddr->db == pvFAddr[i].db) {
      int addr = pAddr->addr - pvFAddr[i].addr;
      criticalLock crtLck(criticalSectionBuff);
      memcpy(buff, pvFAddr[i].buff + addr, dim);
      break;
      }
    }

  if(prfData::tStrData != pAddr->type && prfData::tBitData != pAddr->type) {
    switch(size) {
      case 0:
      case 1:
        break;
      case 2:
        _swab((char*)buff, (char*)buff, dim);
        break;
      case 4:
        mySwab(buff, buff, dim);
        break;
      }
    }
  if(errorByThread)
    return gestComm::RecErr;
  return gestComm::NoErr;
}
//----------------------------------------------------------
void commonUseBlock::setErrorByThread(bool set)
{
  if(set) {
    ++realErrorReceive;
    errorByThread = true;
    ++realConsErr;
    ++realTotCount;
    }
  else {
    errorByThread = false;
    realConsErr /= 2;
    ++realTotCount;
    }
}
//----------------------------------------------------------
#define REPEAT_LOG 50
//----------------------------------------------------------
unsigned FAR PASCAL ReaderProc(void* cl)
{
  commonUseBlock *Cl = reinterpret_cast<commonUseBlock*>(cl);
  int nElem = Cl->pvFAddr.getElem();
  if(!nElem)
    return false;
  uint tick = Cl->getTimerTick();
  uint stepTime = tick / nElem;
  commIdentity* ci = getIdentity();
  TCHAR fn[_MAX_PATH];
  wsprintf(fn, _T("log_time_thread_Prph_%d.txt"), ci->getNum());

  P_File pf(fn, P_CREAT);
  pf.P_open();
  wsprintf(fn, _T("Init - step = %d, tick = %d, nElem = %d\n"), stepTime, tick, nElem);
  pf.P_writeString(fn);
  uint repeat = 0;
  for(;;) {
    if(Cl->Logoff)
      break;
    for(int i = 0; i < nElem; ++i) {
      if(Cl->Logoff)
        break;
      uint curr = GetTickCount();
      int res;
      do {
        criticalLock crtLck(Cl->criticalSectionConn);
        res = Cl->thPerformRead(Cl->pvFAddr[i]);
        } while(false);
      do {
        criticalLock crtLck(Cl->criticalSectionBuff);
        Cl->setErrorByThread(toBool(res));
        } while(false);
      curr = GetTickCount() - curr;
      if(repeat < REPEAT_LOG) {
        wsprintf(fn, _T("%d - time = %d"), repeat + 1, curr);
        pf.P_writeString(fn);
        }
      if(stepTime > curr)
        curr = stepTime - curr;
      else
        curr = 10;
      if(repeat < REPEAT_LOG) {
        ++repeat;
        wsprintf(fn, _T(", sleep = %d\n"), curr);
        pf.P_writeString(fn);
        }
      if(REPEAT_LOG == repeat) {
        ++repeat;
        pf.P_close();
        }

      Sleep(curr);
      }
    Cl->setReadyToRead();
    }
  return true;
}
//----------------------------------------------------------
