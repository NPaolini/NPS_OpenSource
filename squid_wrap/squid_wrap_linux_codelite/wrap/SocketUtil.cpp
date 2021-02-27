//------------------ SocketUtil.cpp ---------------------------
//--------------------------------------------------------------------------------
#include "SocketUtil.h"
//--------------------------------------------------------------------------------
SOCKET createClientSocket(WORD port, DWORD &addr, int nRepeat, LPCSTR serverName)
{
  if(serverName) {
    addrinfo* ai;
    if(!GetAddrInfo(serverName, 0, 0, &ai)) {
      SOCKADDR_IN* sin = (SOCKADDR_IN*)ai->ai_addr;
      addr = sin->sin_addr.s_addr;
      }
    }
  SOCKET sock = createTalkSocket(port, addr, nRepeat);
  if(INVALID_SOCKET == sock)
    return INVALID_SOCKET;

  SOCKADDR_IN saTalk;
  socklen_t len = sizeof(saTalk);
  if(getsockname(sock, (LPSOCKADDR)&saTalk, &len))
    return INVALID_SOCKET;
  addr = saTalk.sin_addr.s_addr;
  return sock;
}
//--------------------------------------------------------------------------------
/*
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
*/
#ifdef WINDOWS
#else
  uint32_t getTick() {
      struct timespec ts;
      unsigned theTick = 0U;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      theTick  = ts.tv_nsec / 1000000;
      theTick += ts.tv_sec * 1000;
      return theTick;
  }
#endif
//--------------------------------------------------------------------------------
static int performHasByte(SOCKET sock, int delaySec = 0, int delayUSec = 20000)
{
  timeval timeout = { delaySec, delayUSec };
  fd_set readready;
  FD_ZERO(&readready);
  FD_SET(sock, &readready);
//  DWORD tick = GetTickCount();
  int result= select(FD_SETSIZE, &readready, 0, 0, &timeout);
/*
  if(!result) {
    int timeOut = delaySec * 1000 + delayUSec / 1000;
    int timeDiff = int(GetTickCount() - tick);
#ifdef PRINT_INFO
    printf("performHasByte: no result, timeout=%d, timeDiff=%d\n", timeOut, timeDiff);
#endif
    // se torna zero prima che sia scaduto il timeout (tolleranza di 10msec) vuol dire che la connessione
    // è stata chiusa normalmente con tutti i byte ricevuti, allora torniano il valore predefinito per uscire dal thread
    return timeOut - timeDiff > 10 ? RET_BY_GRACEFULL_CLOSE : 0;
    }
*/
  return result;
}
//--------------------------------------------------------------------------------
int hasByte(SOCKET sock)
{
  int ret = performHasByte(sock);
//  printf("hasByte:ret=%d\n", ret);
  if(!ret || RET_BY_GRACEFULL_CLOSE == ret)
    if(WSAEINPROGRESS == WSAGetLastError())
      return 0;
  return ret;
}
//--------------------------------------------------------------------------------
static int performCanSend(SOCKET sock, int delaySec = 0, int delaymSec = 300)
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
    if(!ret && WSAEINPROGRESS == WSAGetLastError()) {
      Sleep(5);
      continue;
      }
    return ret;
    } while(++count < 10);
  return ret;
}
//--------------------------------------------------------------------------------
SOCKET createTalkSocket(WORD port, DWORD addr, int nRepeat)
{
  bool success = false;
  SOCKET sock;
  int proto = 0; //IPPROTO_TCP;
  do {
    sock = socket(AF_INET, SOCK_STREAM, proto);
    if(INVALID_SOCKET == sock)
      break;
    int enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    SOCKADDR_IN saTalk;

    saTalk.sin_family = AF_INET;
    saTalk.sin_addr.s_addr = addr;
    saTalk.sin_port = htons(port);
#if 1
    if(SOCKET_ERROR == connect(sock, (LPSOCKADDR)&saTalk, sizeof(struct sockaddr)))
      break;
#else
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
#endif
    success = true;
    } while(false);

  if(!success) {
    if(INVALID_SOCKET != sock)
      closesocket(sock);
    return INVALID_SOCKET;
    }
  return sock;
}
//----------------------------------------------------------------------------
