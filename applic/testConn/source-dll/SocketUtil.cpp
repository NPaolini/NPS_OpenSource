//------------------ SocketUtil.cpp ---------------------------
//--------------------------------------------------------------------------------
#include "precompHeader.h"
//--------------------------------------------------------------------------------
#include <stdlib.h>
#include <process.h>
#include <Ws2tcpip.h>
#include "SocketUtil.h"
//--------------------------------------------------------------------------------
SOCKET createClientSocket(WORD port, DWORD &addr, int nRepeat, int type, LPCTSTR serverName)
{
  bool success = false;
  if(serverName) {
    ADDRINFOT* ai;
    if(!GetAddrInfo(serverName, 0, 0, &ai)) {
      SOCKADDR_IN* sin = (SOCKADDR_IN*)ai->ai_addr;
      addr = sin->sin_addr.S_un.S_addr;
      }
    }
  SOCKET sock = createTalkSocket(port, addr, type, nRepeat);
  if(INVALID_SOCKET == sock)
    return INVALID_SOCKET;

  SOCKADDR_IN saTalk;
  int len = sizeof(saTalk);
  if(getsockname(sock, (LPSOCKADDR)&saTalk, &len))
    return INVALID_SOCKET;
  addr = saTalk.sin_addr.s_addr;
  return sock;
}
//--------------------------------------------------------------------------------
WORD getPort(SOCKET sock, bool remote)
{
  SOCKADDR_IN sa;
  int len = sizeof(sa);
  int result;
  if(remote)
    result = getpeername(sock, (LPSOCKADDR)&sa, &len);
  else
    result = getsockname(sock, (LPSOCKADDR)&sa, &len);
  if(!result)
    return ntohs(sa.sin_port);
  return 0;
}
//--------------------------------------------------------------------------------
DWORD getAddr(SOCKET sock, bool remote)
{
  SOCKADDR_IN sa;
  int len = sizeof(sa);
  int result;
  if(remote)
    result = getpeername(sock, (LPSOCKADDR)&sa, &len);
  else
    result = getsockname(sock, (LPSOCKADDR)&sa, &len);
  if(!result)
    return ntohl(sa.sin_addr.S_un.S_addr);
  return 0;
}
//--------------------------------------------------------------------------------
bool getAddrAndPort(SOCKET sock, DWORD& addr, DWORD& port, bool remote)
{
  SOCKADDR_IN sa;
  int len = sizeof(sa);
  int result;
  if(remote)
    result = getpeername(sock, (LPSOCKADDR)&sa, &len);
  else
    result = getsockname(sock, (LPSOCKADDR)&sa, &len);
  if(!result) {
    addr = ntohl(sa.sin_addr.S_un.S_addr);
    port = ntohs(sa.sin_port);
    return true;
    }
  return false;
}
//--------------------------------------------------------------------------------
bool getConnName(SOCKET sock, LPTSTR buff, bool remote)
{
  DWORD addr;
  DWORD port;
  if(!getAddrAndPort(sock, addr, port, remote))
    return false;

  struct sockaddr_in saGNI;
  TCHAR hostname[NI_MAXHOST];
  TCHAR servInfo[NI_MAXSERV];
  saGNI.sin_family = AF_INET;
  saGNI.sin_addr.s_addr = addr;
  saGNI.sin_port = htons(port);

  if(GetNameInfo((struct sockaddr *) &saGNI, sizeof(saGNI), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV)) 
    return false;
  _tcscpy_s(buff, MAX_LEN_CONN_NAME, hostname);
  return true;
}
//--------------------------------------------------------------------------------
static int performHasByte(SOCKET sock, int delaySec = 1, int delayUSec = 0)
{
  timeval timeout = { delaySec, delayUSec };
  fd_set readready;
  FD_ZERO(&readready);
  FD_SET(sock, &readready);
  DWORD tick = GetTickCount();
  int result= select(FD_SETSIZE, &readready, 0, 0, &timeout);
  if(!result) {
    int timeOut = delaySec * 1000 + delayUSec / 1000;
    int timeDiff = int(GetTickCount() - tick);
    // se torna zero prima che sia scaduto il timeout (tolleranza di 100msec) vuol dire che la connessione
    // è stata chiusa normalmente con tutti i byte ricevuti, allora torniano il valore predefinito per uscire dal thread
    return timeOut - timeDiff > 100 ? RET_BY_GRACEFULL_CLOSE : 0;
    }
  return result;
}
//--------------------------------------------------------------------------------
int hasByte(SOCKET sock)
{
  int ret = performHasByte(sock);
  if(!ret || RET_BY_GRACEFULL_CLOSE == ret)
    if(WSAEINPROGRESS == WSAGetLastError())
      return 0;
  return ret;
}
//--------------------------------------------------------------------------------
static int performCanSend(SOCKET sock, int delaySec = 1, int delaymSec = 0)
{
  timeval timeout = { delaySec, delaymSec };
  fd_set writeready;
  FD_ZERO(&writeready);
  FD_SET(sock, &writeready);
  return select(FD_SETSIZE, 0, &writeready, 0, &timeout);
}
//--------------------------------------------------------------------------------
int canSend(SOCKET sock)
{
  DWORD count = 0;
  int ret;
  do {
    ret = performCanSend(sock);
    if(ret > 0)
      return ret;
    if(!ret || WSAEINPROGRESS == WSAGetLastError()) {
      Sleep(20);
      continue;
      }
    return ret;
    } while(++count < 10);
  return ret;
}
//--------------------------------------------------------------------------------
SOCKET createServerSocket(WORD port, int type)
{
  bool success = false;
  SOCKET sock;
  int proto = SOCK_STREAM == type ? IPPROTO_TCP : IPPROTO_UDP;
  do {
    sock = socket(AF_INET, type, proto);
    if(INVALID_SOCKET == sock)
      break;

    BOOL set = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (LPSTR)&set, sizeof(set));

    SOCKADDR_IN saServer;

    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = INADDR_ANY;
    saServer.sin_port = htons(port);

    if(SOCKET_ERROR == bind(sock, (LPSOCKADDR)&saServer, sizeof(struct sockaddr)))
      break;

    success = true;
    } while(false);

  if(!success) {
    if(INVALID_SOCKET != sock)
      closesocket(sock);
    return INVALID_SOCKET;
    }
  return sock;
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
SOCKET createTalkSocket(WORD port, DWORD addr, int type, int nRepeat)
{
  bool success = false;
  SOCKET sock;
  int proto = SOCK_STREAM == type ? IPPROTO_TCP : IPPROTO_UDP;
  do {
    sock = socket(AF_INET, type, proto);
    if(INVALID_SOCKET == sock)
      break;

    SOCKADDR_IN saTalk;

    int len = sizeof(saTalk);
    saTalk.sin_family = AF_INET;
    saTalk.sin_addr.s_addr = addr;
    saTalk.sin_port = htons(port);

    unsigned long ul = 1;
    int nRet = ioctlsocket(sock, FIONBIO, &ul);
    if(nRet == SOCKET_ERROR) {
      // prova connessione blocking
      if(SOCKET_ERROR == connect(sock, (LPSOCKADDR)&saTalk, sizeof(struct sockaddr)))
        break;
      }
    else {
      int i = 0;
      if(SOCKET_ERROR == connect(sock, (LPSOCKADDR)&saTalk, sizeof(struct sockaddr))) {
        if(WSAEWOULDBLOCK == WSAGetLastError()){
          for(i = 0; i < nRepeat; ++i) {
            if(performCanSend(sock, 0) > 0)
              break;
            Sleep(100);
            }
          }
        }
      ul = 0;
      ioctlsocket(sock, FIONBIO, &ul);
      if(i >= nRepeat)
        break;
      }
    success = true;
    } while(false);

  if(!success) {
    if(INVALID_SOCKET != sock)
      closesocket(sock);
    return INVALID_SOCKET;
    }
  return sock;
}
//--------------------------------------------------------------------------------
serverLanData::serverLanData() : Socket(INVALID_SOCKET), needClose(0), Udp(false) {}
//--------------------------------------------------------------------------------
serverLanData::~serverLanData()
{
  if(!needClose)
    needClose = 1;
  if(INVALID_SOCKET != Socket)
    closesocket(Socket);
  for(int i = 0; i < 100; ++i) {
    if(needClose < 0)
      break;
    Sleep(50);
    }
}
//--------------------------------------------------------------------------------
bool serverLanData::run(WORD port, int type)
{
  Socket = createServerSocket(port, type);
  Udp = SOCK_STREAM != type;
  if(INVALID_SOCKET != Socket) {
    DWORD idThread;
    HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainServerLanProc, this, 0, &idThread);
    if(!hThread) {
      closesocket(Socket);
      Socket = INVALID_SOCKET;
      return false;
      }
    CloseHandle(hThread);
    return true;
    }
  return false;
}
//--------------------------------------------------------------------------------
#include "conn_dll.h"
#include "wm_custom.h"
#include "PLanComm.h"
//--------------------------------------------------------------------------------
unsigned FAR PASCAL mainServerLanProc(void* pData)
{
  serverLanData* ServerLan = reinterpret_cast<serverLanData*>(pData);
  SOCKET Socket = ServerLan->Socket;

  SOCKADDR_IN& sa = ServerLan->saRemote;

  if(ServerLan->Udp) {
    int len = sizeof(sa);
    char buff[1024];
    int result = recvfrom(Socket, buff, sizeof(buff), 0, (LPSOCKADDR)&sa, &len);
    if(SOCKET_ERROR != result || WSAEMSGSIZE == WSAGetLastError()) {
      DWORD remoteIP = ntohl(sa.sin_addr.S_un.S_addr);
      DWORD remotePort = ntohs(sa.sin_port);
      PLanComm* Com = (PLanComm*)ServerLan->addConnection(Socket, remoteIP, remotePort);
      ServerLan->Socket = INVALID_SOCKET;
      if(result > 0)
        Com->appendToBuffer((LPBYTE)buff, result);
      }
    }
  else if(SOCKET_ERROR != listen(Socket, SOMAXCONN)) {

    while(true) {
      int len = sizeof(sa);
      SOCKET remoteSocket = accept(Socket, (LPSOCKADDR)&sa, &len);

      if (INVALID_SOCKET == remoteSocket)
        break;

      if(ServerLan->needClose) {
        closesocket(remoteSocket);
        break;
        }
      DWORD remoteIP = ntohl(sa.sin_addr.S_un.S_addr);
      DWORD remotePort = ntohs(sa.sin_port);
      ServerLan->addConnection(remoteSocket, remoteIP, remotePort);
      }
    }
  ServerLan->needClose = -1;
  return 1;
}
//--------------------------------------------------------------------------------
extern
HANDLE_CONN makeHandle(HWND owner, LPVOID conn, ConnType type, DWORD p1, UINT_PTR p2);
//----------------------------------------------------------------------------
LPVOID customServerLanData::addConnection(SOCKET socket, DWORD remoteIP, DWORD remotePort)
{
  DWORD addr;
  DWORD port;
  getAddrAndPort(socket, addr, port);
  PLanComm* Com = new PLanComm(port, Client, Udp);
  paramConn pc(addr);
  Com->setParam(pc);
  Com->setRemote(saRemote);
  Com->startOpened(socket);
  HANDLE_CONN h = makeHandle(Client, (LPVOID)Com, cLanClient, remotePort, remoteIP);
  if(ICB.fz_callBack)
    (*ICB.fz_callBack)(HIWP_ADD_CLIENT, h, ICB.customData);
  else
    PostMessage(Owner, WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_BY_LAN_SERVER, HIWP_ADD_CLIENT), (LPARAM)h);
  return (LPVOID)Com;
}
//----------------------------------------------------------------------------
