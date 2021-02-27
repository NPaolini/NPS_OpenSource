//-------- sv_server.cpp -------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_server.h"
//------------------------------------------------------------------
//------------------------------------------------------------------
class PWinThread : public winServerConn
{
  private:
    typedef winServerConn baseClass;
  public:
    PWinThread(IsvServer* server) : baseClass(0, 0, server->getName(), getHinstance()),
      mServer(server)

    {
      Attr.style = WS_OVERLAPPEDWINDOW;
    }
    ~PWinThread() {  destroy(); }

    PVect<winClientConn*>& getClientSet() { return Clients; }

  protected:
    virtual bool createConn();

  private:
    IsvServer* mServer;
};
//------------------------------------------------------------------
//------------------------------------------------------------------
svServer* allocServer()
{
  return new IsvServer;
}
//--------------------------------------------------------------------------------
IsvServer::IsvServer() : Port(0), ServerName(0), winMsg(0), hThread(0)
{
  hEventClose = CreateEvent(0, TRUE, 0, 0);
}
//--------------------------------------------------------------------------------
IsvServer::~IsvServer()
{
  stop();
  delete []ServerName;
  CloseHandle(hEventClose);
}
//--------------------------------------------------------------------------------
criticalSect& IsvServer::getCritSect() { return winMsg->getCritSect(); }
//--------------------------------------------------------------------------------
void IsvServer::stop()
{
  SetEvent(hEventClose);
  // non è necessario, viene subito posto a zero in caso di errore iniziale
//  if(-1 == (LONG)GetWinMsg()) return;

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
//--------------------------------------------------------------------------------
bool IsvServer::start(uint port, LPCTSTR serverName)
{
  Port = port;
  delete []ServerName;
  ServerName = str_newdup(serverName);

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
struct prphClient
{
  DWORD idPrph;
  PVect<winClientConn*> set;
  prphClient(DWORD id = 0) : idPrph(id) {}
};
//--------------------------------------------------------------------------------
static void addToSet(PVect<prphClient>& pSet, winClientConn* cl)
{
  DWORD id = cl->getIdPrph();
  uint nElem = pSet.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(id == pSet[i].idPrph) {
      PVect<winClientConn*>& set = pSet[i].set;
      uint nEl = set.getElem();
      set[nEl] = cl;
      return;
      }
    }
  pSet[nElem].idPrph = id;
  PVect<winClientConn*>& set = pSet[nElem].set;
  set[0] = cl;
}
//--------------------------------------------------------------------------------
void IsvServer::perform()
{
  criticalLock crtLck(getCritSect());
  PWinThread* Win = GetWinMsg();
  if(!Win)
    return;

  PVect<winClientConn*>& cl = Win->getClientSet();
  uint nElem = cl.getElem();

  if(!nElem)
    return;
//  si verifica se i client hanno ricevuto comandi di scrittura,
  for(uint i = 0; i < nElem; ++i)
    cl[i]->writeCommand();

//  si memorizza per ciascuna periferica il numero di utenti
//  poi si inizia a leggere il file dati delle periferiche e si invia a ciascun client
  PVect<prphClient> pSet;
  for(uint i = 0; i < nElem; ++i)
    addToSet(pSet, cl[i]);

  nElem = pSet.getElem();
  FreePacket& fp = getFreeBuff();
  LPDWORD buff = (LPDWORD)fp.get();
  for(uint i = 0; i < nElem; ++i) {
    blockInfoPrph bip = { pSet[i].idPrph, 0, MAX_VARS };
    if(npSV_GetBodyRoutine(eSV_GET_BLOCK_DATA_PRPH, (LPDWORD)&bip, buff)) {
      PVect<winClientConn*>& set = pSet[i].set;
      uint nCl = set.getElem();
      for(uint j = 0; j < nCl; ++j)
        set[j]->sendModifiedData(buff);
      }
    }
  fp.release(buff);

}
//--------------------------------------------------------------------------------
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
static void sendPing(const staticIp4DynamicIp& infoDynIP)
{
  PConnBase conn(infoDynIP.Ip, infoDynIP.Port, 0, false);
  if(!conn.open())
    return;

  ping4dynamicIP ping;
  ping.Info.port = infoDynIP.localPort;
  copyStrZ(ping.Info.code, getPassword(), SIZE_A(ping.Info.code) - 1);
  copyStrZ(ping.Info.hostname, getServerName(), SIZE_A(ping.Info.hostname) - 1);
  ping.Head.makeChkSum();

  conn.write_string((LPBYTE)&ping, sizeof(ping));
  Sleep(1000);
  conn.read_string((LPBYTE)&ping.Head, sizeof(ping.Head));
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

  staticIp4DynamicIp infoDynIP = getInfoDynamicIP();
  if(infoDynIP.Ip && infoDynIP.Port) {
    timeout = infoDynIP.pingMinute * 60 * 1000;
    sendPing(infoDynIP);
    }
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
          sendPing(infoDynIP);
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
