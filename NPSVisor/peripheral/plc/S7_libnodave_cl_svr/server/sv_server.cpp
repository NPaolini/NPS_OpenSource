//-------- sv_server.cpp -------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_server.h"
#include "openSocket.h"
#include "pdllComm.h"
//------------------------------------------------------------------
svServer* allocServer()
{
  return new IsvServer;
}
//--------------------------------------------------------------------------------
IsvServer::IsvServer() : Id(0), Port(0), daveIface(0), daveConn(0), ServerName(0), winMsg(0), hThread(0)
{
  hEventClose = CreateEvent(0, TRUE, 0, 0);
  ZeroMemory(&fds, sizeof(fds));
}
//--------------------------------------------------------------------------------
IsvServer::~IsvServer()
{
  stop();
#ifndef TEST_NO_CONN
  if(daveConn) {
    daveDisconnectPLC(daveConn);
    if(daveIface)
      daveDisconnectAdapter(daveIface);
    daveFree(daveConn);
    daveConn = 0;
    if(daveIface) {
      daveFree(daveIface);
      daveIface = 0;
      }
    }
  if(fds.rfd)
    closesocket((SOCKET)fds.rfd);
#endif
  delete []ServerName;
  CloseHandle(hEventClose);
  pDll::end();
}
//--------------------------------------------------------------------------------
criticalSect& IsvServer::getCritSect() { return winMsg->getCritSect(); }
//--------------------------------------------------------------------------------
void IsvServer::stop()
{
  SetEvent(hEventClose);
  for(;;) {
    bool closed = false;
    {
      criticalLock crtLck(csThread);
      closed = !toBool(GetWinMsg());
    }
    if(closed)
      break;
    Sleep(100);
    }
}
//----------------------------------------------------------
bool IsvServer::openDaveLan(LPCTSTR ip, DWORD rack, DWORD slot)
{
#ifdef TEST_NO_CONN
  daveConn = (daveConnection*)1;
  return true;
#else
  char t[64];
  copyStrZ(t, ip);
  fds.rfd = openSocket(102, t);
  fds.wfd = fds.rfd;
  if(0 == fds.rfd)
    return false;

  daveIface = daveNewInterface(fds,"IF1",0, daveProtoISOTCP, daveSpeed187k);
  daveSetTimeout(daveIface, 5000000);
  daveConn = daveNewConnection(daveIface, 2, rack, slot);

  return 0 == daveConnectPLC(daveConn);
#endif
}
//--------------------------------------------------------------------------------
bool IsvServer::start(uint id, LPCTSTR param)
{
  if(!pDll::init())
    return false;
  Id = id;
  delete []ServerName;
  TCHAR t[_MAX_PATH];
  wsprintf(t, _T("[%d] %s"), Id, getServerName());
  ServerName = str_newdup(t);
  DWORD Slot;
  DWORD Rack;
  while(param) {
    Port = _ttoi(param);
    param = findNextParamTrim(param);
    if(!param)
      break;
    Slot = _ttoi(param);
    param = findNextParamTrim(param);
    if(!param)
      break;
    Rack = _ttoi(param);
    param = findNextParamTrim(param);
    break;
    }
  if(!param)
    return false;
  if(!openDaveLan(param, Rack, Slot))
    return false;

  DWORD idThread;
  hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MsgThread, this, 0, &idThread);
  if(!hThread)
    return false;
  CloseHandle(hThread);
  for(;;) {
    bool started = false;
    {
      criticalLock crtLck(csThread);
      started = toBool(GetWinMsg());
    }
    if(started)
      break;
    Sleep(100);
    }

  // segnalazione di errore in init da parte del thread
  if(-1 == (LONG)GetWinMsg()) {
    SetWinMsg(0);
    return false;
    }

  return true;
}
//--------------------------------------------------------------------------------
PWinThread::PWinThread(IsvServer* server) : baseClass(0, 0, server->getName(), getHInstance()),
      mServer(server)
{
  Attr.style = WS_OVERLAPPEDWINDOW;
}
//--------------------------------------------------------------------------------
DWORD PWinThread::getIdPlc() const
{
  return mServer->getIdPlc();
}
//--------------------------------------------------------------------------------
void PWinThread::getDaveConn(serializeConn& srzC)
{
  static uint cnt;
  t_daveQueue& DaveQueue =  mServer->DaveQueue;

  serializeConn* t = DaveQueue.get();
  srzC.id = ++cnt;
  srzC.dC = mServer->daveConn;
  *t = srzC;
  bool canStart = !DaveQueue.getStored();
  DaveQueue.push(t);
  if(canStart)
    SetEvent(srzC.hWait);
}
//----------------------------------------------------------------------------
void PWinThread::releaseDaveConn(serializeConn& srzC)
{
  t_daveQueue& DaveQueue =  mServer->DaveQueue;
  if(DaveQueue.getStored()) {
    serializeConn* t = mServer->DaveQueue.retrieve();
    serializeConn* old = t;
    do {
      if(srzC.id == t->id) {
        DaveQueue.release(t);
        if(DaveQueue.getStored()) {
          t = DaveQueue.retrieveNoRemove();
          SetEvent(t->hWait);
          }
        t = 0;
        break;
        }
      DaveQueue.push(t);
      t = DaveQueue.retrieve();
      } while(t != old);
    if(t)
      DaveQueue.push(t);
    }
}
//--------------------------------------------------------------------------------
bool PWinThread::createConn()
{
  serverConn = new PConnBase(mServer->getPort(), getHandle(), false);
  if(!serverConn->open()) {
    delete serverConn;
    serverConn = 0;
    }
  return toBool(serverConn);
}
//--------------------------------------------------------------------------------
unsigned FAR PASCAL MsgThread(void* pData)
{
  IsvServer* svS = (IsvServer*)pData;

  PWinThread* W = new PWinThread(svS);

  if(!W->create()) {
    delete W;
    criticalLock crtLck(svS->csThread);
    svS->SetWinMsg((PWinThread*)-1);
    return 0;
    }

  do {
    criticalLock crtLck(svS->csThread);
    svS->SetWinMsg(W);
    } while(false);

  HANDLE HandlesToWaitFor[] = {
    svS->hEventClose,
    };

  ResetEvent(svS->hEventClose);

  DWORD timeout = INFINITE;
  bool terminate = false;

  while(!terminate) {

    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        terminate = true;
        break;
        }
      DispatchMessage(&msg);
      }
    if(!terminate) {
      DWORD result = MsgWaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout, QS_ALLINPUT);
      if(result == (WAIT_OBJECT_0 + SIZE_A(HandlesToWaitFor)))
         continue;
      switch(result) {
        case WAIT_TIMEOUT:
          break;
        case WAIT_OBJECT_0:
        default:
          terminate = true;
          break;
        }
      }
    }
  do {
    criticalLock crtLck(svS->csThread);
    delete W;
    svS->SetWinMsg(0);
    } while(false);
  return 1;
}
