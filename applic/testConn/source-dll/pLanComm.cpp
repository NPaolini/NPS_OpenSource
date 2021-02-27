//------------------ PLanComm.cpp ---------------------------
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "wm_custom.h"
#include "PLanComm.h"
#include "p_util.h"
//-----------------------------------------------------------
#define MAX_RECEIVE 0x100000
//#define MAX_RECEIVE (MAX_BUFF_SOCK * 16)
//-----------------------------------------------------------
PLanComm::PLanComm(DWORD port, HWND hwnd, bool udp) : baseClass(hwnd, MAX_RECEIVE * 2),
    Port(port), Udp(udp), maxSend(MAX_BUFF_SOCK)
{
  ZeroMemory(&saRemote, sizeof(saRemote));
}
//-----------------------------------------------------------
PLanComm::~PLanComm()
{
  close();
}
//------------------------------------------------
bool PLanComm::open(LPCTSTR name)
{
  if(getSocket() != INVALID_SOCKET)
    close();
  if(getSocket() != INVALID_SOCKET)
    return false;

  DWORD addr = getParam();
  SOCKET socket = createClientSocket((WORD)Port, addr, getTOcount(), Udp ? SOCK_DGRAM : SOCK_STREAM, name);
  if(INVALID_SOCKET == socket)
    return false;
  return startOpened(socket);
}
//------------------------------------------------
bool PLanComm::startOpened(SOCKET sock)
{
  setSocket(sock);
  if(Udp) {
    int sz;
    int val;
    if(SOCKET_ERROR != getsockopt(sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&val, &sz))
      maxSend = val;
    }
  connected = true;
//  HANDLE hThread = (HANDLE)_beginthreadex(0, 0, LanCommProc, this, 0, &idThread);
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LanCommProc,
                  this, 0, (LPDWORD)&idThread);

  if(!hThread) {
    connected = false;
    close();
    return false;
    }

  CloseHandle(hThread);
  return true;
}
//------------------------------------------------
void PLanComm::close()
{
  connected = false;
  if(getSocket() != INVALID_SOCKET) {
    SOCKET sock = getSocket();
    setSocket(INVALID_SOCKET);

    while(idThread != 0) {
      Sleep(100);
      }
    shutdown(sock, SD_BOTH);
    closesocket(sock);
    }
}
//------------------------------------------------
void PLanComm::reset()
{
  criticalLock crtLck(cS);
  cL->reset();
}
//--------------------------------------------------------------------------------
DWORD PLanComm::performWrite_string(const void *buff, DWORD len)
{
  SOCKET Socket = getSocket();
  if(INVALID_SOCKET == Socket)
    return 0;

  int nRet = canSend(Socket);
  if(nRet <= 0)
    return 0;

  nRet = lanSend(Socket, (LPCSTR)buff, len);
  return nRet >= 0 ? nRet : 0;
}
//--------------------------------------------------------------------------------
DWORD PLanComm::write_string(const void *buff, DWORD len)
{
  long size = len;
  LPBYTE p = (LPBYTE)buff;
  uint count = 0;
  while(size) {
    long dim = min(size, maxSend);
    dim = performWrite_string(p, dim);
    if(!dim) {
      if(++count >= 5)
        break;
      continue;
      }
    size -= dim;
    p += dim;
//    if(size)
      Sleep(0);
    }
  return size ? 0 : len;
}
//--------------------------------------------------------------------------------
int PLanComm::lanSend(SOCKET socket, LPCSTR buff, int len)
{
  if(Udp && saRemote.sin_port) {
    int sz = sizeof(saRemote);
    return sendto(socket, buff, len, 0, (LPSOCKADDR)&saRemote, sz);
    }
  return send(socket, buff, len, 0);
}
//--------------------------------------------------------------------------------
int PLanComm::lanRecv(SOCKET socket, LPSTR buff, int len)
{
  if(Udp) {
    int sz = sizeof(saRemote);
    return recvfrom(socket, buff, len, 0, (LPSOCKADDR)&saRemote, &sz);
    }
  return recv(socket, buff, len, 0);
}
//--------------------------------------------------------------------------------
static bool hasKeep_Alive(PLanComm *Conn)
{
  return false;
/*
  int iOptVal;
  int iOptLen = sizeof(int);
  SOCKET sock = Conn->getSocket();

  if(getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR)
    return toBool(iOptVal);
  return false;
*/
}
//--------------------------------------------------------------------------------
unsigned FAR PASCAL LanCommProc(void* pData)
{
  PLanComm *Com = reinterpret_cast<PLanComm*>(pData);
  SOCKET Socket = Com->getSocket();
  LPBYTE buff = new BYTE[MAX_RECEIVE];
  DWORD Time4Close = 0;
  while(Com->isConnected()) {
    int nRet = hasByte(Socket);
//------ se ci sono dati in lettura --------------------
    if(nRet > 0 && RET_BY_GRACEFULL_CLOSE != nRet) {
      int free = 0;
      do {
        criticalLock crtLck(Com->cS);
        free = Com->cL->free();
        } while(false);
      if(free < MAX_BUFF_SOCK) {
        Com->notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, 1), (LPARAM)Com->getSocket());
        Sleep(100);
        continue;
        }
      int len = min(MAX_RECEIVE, free);
      len = Com->lanRecv(Socket, (LPSTR)buff, len);
      // se len < 0 -> errore, se len == 0 il socket remoto si è chiuso
      if(len < 0)
        break;
      if(!len)
        nRet = RET_BY_GRACEFULL_CLOSE;
      else {
        Time4Close = 0;
        DWORD stored;
        do {
          criticalLock crtLck(Com->cS);
          Com->cL->putBytes(buff, len);
          stored = Com->cL->stored();
          } while(false);
        Com->notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, stored), (LPARAM)Com->getSocket());
        Sleep(0);
        }
      }

    if(RET_BY_GRACEFULL_CLOSE == nRet) {
#if 0
      if(hasKeep_Alive(Com)) {
        if(!Time4Close)
          Time4Close = GetTickCount() + 1500;
        else {
          if(GetTickCount() > Time4Close)
            break;
          }
        }
      else
        break;
#else
      break;
#endif
      }
    else if(nRet < 0)
      break;
    else
      Sleep(30);
    }
  delete []buff;
  Com->resetThread();
  Com->close();
  Com->notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_BY_LAN_CLIENT, 0), 0);
  return true;
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
