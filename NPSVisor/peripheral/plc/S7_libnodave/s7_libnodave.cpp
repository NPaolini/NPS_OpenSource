//--------------- s7_libnodave.cpp -----------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include "s7_libnodave.h"
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"

#include "prfData.h"
#include "p_txt.h"

#include <Winsock2.h>
#include "openSocket.h"
#include "setport.h"
//----------------------------------------------------------
//#define LOG_TO_FILE
//#define TEST_NO_CONN
//----------------------------------------------------------
#ifdef TEST_NO_CONN
//----------------------------------------------------------
//----------------------------------------------------------
static int DB_READ(daveConnection*,int,int,int,void*) { return 0; }
static int DB_WRITE(daveConnection*,int,int,int,void*) { return 0; }
//----------------------------------------------------------
#else
//----------------------------------------------------------
#pragma comment(lib, ".\\libnodave.lib")
//----------------------------------------------------------
#define DB_READ(dc, DB, start, len, buffer)   \
    daveReadManyBytes(dc, daveDB, DB, start, len, buffer)

#define DB_WRITE(dc, DB, start, len, buffer)   \
    daveWriteManyBytes(dc, daveDB, DB, start, len, buffer)
//----------------------------------------------------------
#endif
//----------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("Siemens S7 Ver. Libnodave [%d]"), idPrf);
    w->setCaption(buff);
    }

  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new s7_libnodave(name, FLAG_PERIF(idPrf));
}
//----------------------------------------------------------
//----------------------------------------------------------
#ifdef LOG_TO_FILE
#include "p_logFile.h"

p_logFile* pLF;
#endif
//----------------------------------------------------------
s7_libnodave::s7_libnodave(LPCTSTR file, WORD flagCommand) :
    baseClass(file, flagCommand)
{
#ifdef LOG_TO_FILE
  pLF = new p_logFile(_T("error.log"));
#endif
}
//----------------------------------------------------------
s7_libnodave::~s7_libnodave()
{
  stopThread();

  flushPV(adr_table);

#ifdef LOG_TO_FILE
  delete pLF;
#endif
}
//----------------------------------------------------------
void s7_libnodave::addToTable(uint check, setOfString& sSet)
{
  if(!check)
    return;
  int nElem = adr_table.getElem();
  for(int i = 0; i < nElem; ++i)
    if(check == adr_table[i]->getCheck())
      return;
  adr_table[nElem] = new myAdrTable(check);
}
//----------------------------------------------------------
#define FILENAME_INFO_ _T("s7_libnodave")
//----------------------------------------------------------
void s7_libnodave::makeSetInfo(setOfString& set)
{
  commIdentity* ci = getIdentity();
  TCHAR name[_MAX_PATH];
  wsprintf(name, _T("%s_%d.txt"), FILENAME_INFO_, ci->getNum());
  set.add(name);
}
//----------------------------------------------------------
gestComm::ComResult s7_libnodave::Init()
{
  gestComm::ComResult result = baseClass::Init();
  if(result < gestComm::NoErr)
    return errorInit();

  setOfString sSet;
  makeSetInfo(sSet);
  if(!sSet.setFirst())
    return errorInit();
  uint nElem = pvFAddr.getElem();
  for(uint i = 0; i < nElem; ++i)
    addToTable(pvFAddr[i].V_CHECK, sSet);

  if(adr_table.getElem()) {
    int len = adr_table.getElem();
    for(int i = 0; i < len; ++i) {
      LPCTSTR p = sSet.getString(adr_table[i]->getCheck());
      if(!adr_table[i]->init(p))
        return errorInit();
      }
    }
  else
    return errorInit();

  startTread();
  return gestComm::NoErr;
}
//----------------------------------------------------------
daveConnection* s7_libnodave::getConn(DWORD ix)
{
  uint len = adr_table.getElem();
  for(uint i = 0; i < len; ++i)
    if(adr_table[i]->getCheck() == ix)
      return adr_table[i]->getConn();
  return 0;
}
//----------------------------------------------------------
int s7_libnodave::thPerformRead(fullAddr& fa)
{
  daveConnection* dc = getConn(fa.V_CHECK);
  if(!dc)
    return -1;
  int res = DB_READ(dc, fa.db, fa.addr, fa.len, fa.buff);
#ifdef LOG_TO_FILE
  if(res) {
    pLF->addLog(_T("DB_READ(thread)-> error code %04X - DB = %d, Addr = %d, len = %d\r\n"),
                  res, fa.db, fa.addr, fa.len);
        }
#endif
  return res;
}
//----------------------------------------------------------
int s7_libnodave::performSend(int check, int db, int addr, int dim, LPBYTE buff)
{
  daveConnection* dc = getConn(check);
  if(!dc)
    return -1;
  int res = DB_WRITE(dc, db, addr, dim, buff);
#ifdef LOG_TO_FILE
  if(res) {
    pLF->addLog(_T("DB_WRITE -> error code %04X - DB = %d, Addr = %d, len = %d\r\n"),
                res, db, addr, dim);
    }
#endif
  return res;
}
//----------------------------------------------------------
//----------------------------------------------------------
#define MPI_CONN 1
#define IP_CONN  2

#define DEF_ARGC 3
//----------------------------------------------------------
bool myAdrTable::openLan(LPCTSTR str)
{
#ifdef LOG_TO_FILE
  pvvChar target;
  uint nElem = splitParam(target, str);
  if(DEF_ARGC > nElem) {
    pLF->addLog(_T("openLan -> error numParam = %d, str = %s\r\n"), nElem, str);
    return false;
    }
  char peer[128];
  copyStrZ(peer, &target[0]);
  fds.rfd = openSocket(102, peer);
  fds.wfd = fds.rfd;
  if(0 == fds.rfd) {
    pLF->addLog(_T("openLan -> error open, port = 102, ip = %s\r\n"), &target[0]);
    return false;
    }

  di = daveNewInterface(fds,"IF1",0, daveProtoISOTCP, daveSpeed187k);
  pLF->addLog(_T("daveNewInterface -> result = %p\r\n"), di);
  daveSetTimeout(di, 5000000);
  int rack = _ttoi(&target[2]);
  int slot = _ttoi(&target[1]);
  dc = daveNewConnection(di, 2, rack, slot);
  pLF->addLog(_T("daveNewConnection -> rack(%d), slot(%d), result = %p\r\n"), dc);

  return 0 == daveConnectPLC(dc);

#else
  pvvChar target;
  uint nElem = splitParam(target, str);
  if(DEF_ARGC > nElem)
    return false;
  char peer[128];
  copyStrZ(peer, &target[0]);
  fds.rfd = openSocket(102, peer);
  fds.wfd = fds.rfd;
  if(0 == fds.rfd)
    return false;

  di = daveNewInterface(fds,"IF1",0, daveProtoISOTCP, daveSpeed187k);
  daveSetTimeout(di, 5000000);
  int rack = _ttoi(&target[2]);
  int slot = _ttoi(&target[1]);
  dc = daveNewConnection(di, 2, rack, slot);

  return 0 == daveConnectPLC(dc);
#endif
}
//----------------------------------------------------------
bool myAdrTable::openMPI(LPCTSTR str)
{
  pvvChar target;
  uint nElem = splitParam(target, str);
  if(DEF_ARGC > nElem)
    return false;
  char baud[64] = "38400";
  if(nElem > DEF_ARGC)
    copyStrZ(baud, &target[DEF_ARGC]);
  char port[64];
  copyStrZ(port, &target[0]);

  fds.rfd = setPort(port, baud, 'O');
  fds.wfd = fds.rfd;
  if(0 == fds.rfd)
    return false;
  int speed = daveSpeed187k;
  if(nElem > DEF_ARGC + 1) {
    int s = _ttoi(&target[DEF_ARGC + 1]);
    switch(s) {
      case 9:
        speed = daveSpeed9k;
        break;
      case 19:
        speed = daveSpeed19k;
        break;
      case 45:
        speed = daveSpeed45k;
        break;
      case 93:
        speed = daveSpeed93k;
        break;
      case 187:
        speed = daveSpeed187k;
        break;
      case 500:
        speed = daveSpeed500k;
        break;
      case 1500:
        speed = daveSpeed1500k;
        break;
      }
    }

  di = daveNewInterface(fds, "IF1", 0, daveProtoMPI, speed);
  daveSetTimeout(di, 5000000);
  bool success = false;
  for(uint i = 0; i < 3; ++i) {
    if(0 == daveInitAdapter(di)) {
      success = true;
      break;
      }
    daveDisconnectAdapter(di);
    }
  if(success) {
    int rack = _ttoi(&target[2]);
    int slot = _ttoi(&target[1]);
    int addr = 2;
    if(nElem > DEF_ARGC + 2)
      addr = _ttoi(&target[DEF_ARGC + 2]);
    dc = daveNewConnection(di, addr, rack, slot);
    return 0 == daveConnectPLC(dc);
    }
  return false;
}
//----------------------------------------------------------
bool myAdrTable::init(LPCTSTR str)
{
  if(!str)
    return false;

  connType = _ttoi(str);
  str = findNextParamTrim(str);
  switch(connType) {
    case IP_CONN:
      return openLan(str);
    case MPI_CONN:
      return openMPI(str);
    }
  return false;
}
//----------------------------------------------------------
void myAdrTable::close()
{
  if(dc)
    daveDisconnectPLC(dc);
  if(di)
    daveDisconnectAdapter(di);
  if(dc) {
    daveFree(dc);
    dc = 0;
    }
  if(di) {
    daveFree(di);
    di = 0;
    }
  switch(connType) {
    case IP_CONN:
      if(fds.rfd)
        closesocket((SOCKET)fds.rfd);
      break;
    case MPI_CONN:
      if(fds.rfd)
        CloseHandle(fds.rfd);
      break;
    }
}
