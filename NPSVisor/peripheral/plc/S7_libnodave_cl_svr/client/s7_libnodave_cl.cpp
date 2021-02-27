//--------------- s7_libnodave_cl.cpp ---------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include "s7_libnodave_cl.h"
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"

#include "prfData.h"
#include "p_txt.h"
#include "p_util.h"
//----------------------------------------------------------
//#define LOG_TO_FILE
//#define TEST_NO_CONN
//----------------------------------------------------------
#ifdef TEST_NO_CONN
//----------------------------------------------------------
static int DB_READ(int,int,int,void*) { return 0; }
static int DB_WRITE(int,int,int,void*) { return 0; }
//----------------------------------------------------------
#else
//----------------------------------------------------------
#define DB_READ(DB, start, len, buffer)   \
    adr_table[i]->readManyBytes_remote(DB, start, len, buffer)

#define DB_WRITE(DB, start, len, buffer)   \
    adr_table[i]->writeManyBytes_remote(DB, start, len, buffer)
//----------------------------------------------------------
#endif
//----------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("Siemens S7 Ver. Libnodave_Client [%d]"), idPrf);
    w->setCaption(buff);
    }

  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new s7_libnodave_client(name, FLAG_PERIF(idPrf));
}
//----------------------------------------------------------
//----------------------------------------------------------
#ifdef LOG_TO_FILE
#include "p_logFile.h"

p_logFile* pLF;
#endif
//----------------------------------------------------------
s7_libnodave_client::s7_libnodave_client(LPCTSTR file, WORD flagCommand) :
    baseClass(file, flagCommand)
{
#ifdef LOG_TO_FILE
  pLF = new p_logFile(_T("error.log"));
#endif
}
//----------------------------------------------------------
s7_libnodave_client::~s7_libnodave_client()
{
  stopThread();

  flushPV(adr_table);

#ifdef LOG_TO_FILE
  delete pLF;
#endif
}
//----------------------------------------------------------
void s7_libnodave_client::addToTable(uint check, setOfString& sSet)
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
#define FILENAME_INFO_ _T("s7_libnodave_client")
//----------------------------------------------------------
void s7_libnodave_client::makeSetInfo(setOfString& set)
{
  commIdentity* ci = getIdentity();
  TCHAR name[_MAX_PATH];
  wsprintf(name, _T("%s_%d.txt"), FILENAME_INFO_, ci->getNum());
  set.add(name);
}
//----------------------------------------------------------
void s7_libnodave_client::removeNullCheck()
{
  int nElem = pvFAddr.getElem();
  for(int i = nElem - 1; i >= 0; --i)
    if(!pvFAddr[i].V_CHECK || pvFAddr[i].V_CHECK > MAX_ID)
      pvFAddr.remove(i);
}
//----------------------------------------------------------
gestComm::ComResult s7_libnodave_client::Init()
{
  gestComm::ComResult result = baseClass::Init();
  if(result < gestComm::NoErr)
    return errorInit();

  setOfString sSet;
  makeSetInfo(sSet);
  if(!sSet.setFirst())
    return errorInit();
  removeNullCheck();
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
// se non è impostato l'id del plc, per ora ignoriamo, eventualmente si può tornare errore
//----------------------------------------------------------
int s7_libnodave_client::thPerformRead(fullAddr& fa)
{
  if(!fa.V_CHECK)
    return 0;
  uint len = adr_table.getElem();
  int res = -1;
  for(uint i = 0; i < len; ++i) {
    if(adr_table[i]->getCheck() == fa.V_CHECK) {
      res = DB_READ(fa.db, fa.addr, fa.len, fa.buff);
#ifdef LOG_TO_FILE
      if(res) {
        pLF->addLog(_T("DB_READ(thread)-> error code %04X - DB = %d, Addr = %d, len = %d\r\n"),
                  res, db, addr, dim);
        }
#endif
      break;
      }
    }
  return res;
}
//----------------------------------------------------------
int s7_libnodave_client::performSend(int check, int db, int addr, int dim, LPBYTE buff)
{
  if(!check)
    return 0;
  uint len = adr_table.getElem();
  int res = -1;
  for(uint i = 0; i < len; ++i) {
    if(adr_table[i]->getCheck() == check) {
      res = DB_WRITE(db, addr, dim, buff);
#ifdef LOG_TO_FILE
      if(res) {
        pLF->addLog(_T("DB_WRITE -> error code %04X - DB = %d, Addr = %d, len = %d\r\n"),
                res, db, addr, dim);
        }
#endif
      break;
      }
    }
  return res;
}
//----------------------------------------------------------
/*
#define DEF_ARGC 3
//----------------------------------------------------------
bool myAdrTable::openLan(LPCTSTR str)
{
  pvvChar target;
  uint nElem = splitParam(target, str);
  if(DEF_ARGC > nElem)
    return false;
  fds.rfd = openSocket(102, &target[0]);
  fds.wfd = fds.rfd;
  if(0 == fds.rfd)
    return false;

  di = daveNewInterface(fds,"IF1",0, daveProtoISOTCP, daveSpeed187k);
  daveSetTimeout(di, 5000000);
  int rack = _ttoi(&target[2]);
  int slot = _ttoi(&target[1]);
  dc = daveNewConnection(di, 2, rack, slot);

  return 0 == daveConnectPLC(dc);
}
*/
//----------------------------------------------------------
static bool runServer(LPCTSTR name)
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags =  STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;
  si.wShowWindow = SW_HIDE;

  PROCESS_INFORMATION pi;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, name);
  if(CreateProcess(0, path, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
    WaitForInputIdle(pi.hProcess, 3 * 1000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
    }
  return false;
}
//----------------------------------------------------------
#define DEF_ARGC 3
#define TIMEOUT_MUTEX 30000
//----------------------------------------------------------
static void relase_mutex(HANDLE hMutex)
{
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
}
//----------------------------------------------------------
bool myAdrTable::init(LPCTSTR p)
{
  pvvChar target;
  uint nElem = splitParam(target, p);
  if(DEF_ARGC > nElem)
    return false;
  TCHAR mx[_MAX_PATH];
  makeClientMutexName(mx, &target[2]);
  HANDLE hMutex = CreateMutex(0, true, mx);
  if(hMutex) {
    if(ERROR_ALREADY_EXISTS == GetLastError()) {
      DWORD result = WaitForSingleObject(hMutex, TIMEOUT_MUTEX);
      if(WAIT_OBJECT_0 != result) {
        relase_mutex(hMutex);
        return false;
        }
      }
    makeServerMutexName(mx, &target[2]);
    HANDLE hMutex2 = CreateMutex(0, true, mx);
    bool started = false;
    if(hMutex2) {
      if(ERROR_ALREADY_EXISTS == GetLastError())
        started = true;
      relase_mutex(hMutex2);
      }
    if(!started)
      started = runServer(&target[2]);
//    MessageBox(0, _T("attesa"), _T("tit"), MB_OK);
    if(started) {
      Conn = new PConnBase(_T("127.0.0.1"), _ttoi(&target[0]), 0, false);
      if(!Conn->open()) {
        started = false;
        safeDeleteP(Conn);
        }
      else
        started = sendLogin(&target[1]);
      }
    if(!started)
      safeDeleteP(Conn);
    relase_mutex(hMutex);
    return started;
    }
  return false;
}
//----------------------------------------------------------
bool myAdrTable::sendLogin(LPCTSTR login)
{
  dataSend ds;
  ds.id = V_CHECK;
  ds.cmd = DB_LOGIN_LIBNODAVE;
  ds.db = 0;
  ds.start = 0;
  int len = _tcslen(login) + 1;
  ds.len = len;
  copyStrZ(ds.buffer, login, len);
  if(SZ_STRU_MAX(len) != Conn->write_string((LPBYTE)&ds, SZ_STRU_MAX(len)))
    return false;

  if(SZ_STRU_MIN != Conn->read_string((LPBYTE)&ds, SZ_STRU_MIN))
    return false;
    // controlli vari ...
  if(DB_LOGGED_LIBNODAVE != ds.cmd)
    return false;
  return true;
}
//----------------------------------------------------------
void myAdrTable::sendLogout()
{
  if(!Conn)
    return;
  dataSend ds;
  ds.id = V_CHECK;
  ds.cmd = DB_LOGOUT_LIBNODAVE;
  ds.db = 0;
  ds.start = 0;
  ds.len = 0;
  Conn->write_string((LPBYTE)&ds, SZ_STRU_MIN);
}
//----------------------------------------------------------
void myAdrTable::close()
{
  sendLogout();
  delete Conn;
  Conn = 0;
}
//----------------------------------------------------------
int myAdrTable::readManyBytes_remote(int db, int start, int len, LPBYTE buffer)
{
  if(Conn) {
    dataSend ds;
    ds.id = V_CHECK;
    ds.cmd = DB_READ_LIBNODAVE;
    ds.db = db;
    ds.start = start;
    ds.len = len;
    if(SZ_STRU_MIN != Conn->write_string((LPBYTE)&ds, SZ_STRU_MIN))
      return -1;
    // per sicurezza e verifica, si può togliere in release se tutto ok
    ZeroMemory(ds.buffer, len + 2);

    if(SZ_STRU_MAX(len) != Conn->read_string((LPBYTE)&ds, SZ_STRU_MAX(len)))
      return -1;
    // controlli vari ...
    if(DB_READED_LIBNODAVE != ds.cmd)
      return ds.len;
    memcpy(buffer, ds.buffer, len);
    return 0;
    }
  return -1;
}
//----------------------------------------------------------
int myAdrTable::writeManyBytes_remote(int db, int start, int len, LPBYTE buffer)
{
  if(Conn) {
    dataSend ds;
    ds.id = V_CHECK;
    ds.cmd = DB_WRITE_LIBNODAVE;
    ds.db = db;
    ds.start = start;
    ds.len = len;
    memcpy(ds.buffer, buffer, len);
    if(SZ_STRU_MAX(len) != Conn->write_string((LPBYTE)&ds, SZ_STRU_MAX(len)))
      return -1;

    if(SZ_STRU_MIN != Conn->read_string((LPBYTE)&ds, SZ_STRU_MIN))
      return -1;
    // controlli vari ...
    if(DB_WRITED_LIBNODAVE != ds.cmd)
      return ds.len;
    return 0;
    }
  return -1;
}
//----------------------------------------------------------
