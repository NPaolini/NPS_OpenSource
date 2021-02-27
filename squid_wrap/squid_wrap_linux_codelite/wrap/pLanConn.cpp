//------------------ PLanConn.cpp ---------------------------
//-----------------------------------------------------------
#include "pLanConn.h"
//-----------------------------------------------------------
#define N_REPEAT 50
//-----------------------------------------------------------
PLanConn::PLanConn(LPCSTR type_helper, DWORD addr, DWORD port) :
    typeHelper(strdup(type_helper)), Addr(addr), 
    Port(port), maxSend(MAX_BUFF_SOCK), Sock(INVALID_SOCKET), serverName(0)
{
}
//-----------------------------------------------------------
PLanConn::PLanConn(LPCSTR type_helper, LPCSTR server_name, DWORD port) :
    typeHelper(strdup(type_helper)), Addr(0), 
    Port(port), maxSend(MAX_BUFF_SOCK), Sock(INVALID_SOCKET), serverName(strdup(server_name))
{
}
//-----------------------------------------------------------
PLanConn::~PLanConn()
{
  delete []typeHelper;
  delete []serverName;
  close();
}
//------------------------------------------------
void PLanConn::setServerName(LPCSTR name)
{
  delete[]serverName;
  serverName = strdup(name);
}
//------------------------------------------------
bool PLanConn::open()
{
  if(getSocket() != INVALID_SOCKET)
    close();
  if(getSocket() != INVALID_SOCKET)
    return false;
  SOCKET socket = createClientSocket((WORD)Port, Addr, N_REPEAT, serverName);
  if(INVALID_SOCKET == socket)
    return false;
  return startOpened(socket);
}
//------------------------------------------------
bool PLanConn::startOpened(SOCKET sock)
{
  setSocket(sock);
  char t[256];
  sprintf(t, "%s\n", typeHelper);
  write(t, strlen(t));
  read(t, sizeof(t));
  const char* resp = "ok-";
  for(int i = 0; i < 3; ++i)
      if(t[i] != resp[i])
          return false;
  for(uint i = 0; i < strlen(typeHelper); ++i)
      if(t[i + 3] != typeHelper[i])
          return false;
  return true;
}
//------------------------------------------------
void PLanConn::close()
{
  if(getSocket() != INVALID_SOCKET) {
    SOCKET sock = getSocket();
    setSocket(INVALID_SOCKET);
    shutdown(sock, SD_BOTH);
    ::closesocket(sock);
    }
}
//--------------------------------------------------------------------------------
long PLanConn::performWrite(const void *buff, long len)
{
  SOCKET Socket = getSocket();
  if(INVALID_SOCKET == Socket)
    return -1;
/*
  int nRet = canSend(Socket);
  if(nRet <= 0)
    return 0;
*/
  int nRet = lanSend(Socket, (LPCSTR)buff, len);
  return nRet >= 0 ? nRet : 0;
}
//--------------------------------------------------------------------------------
long PLanConn::write(const void *buff, long len)
{
  long size = len;
  LPBYTE p = (LPBYTE)buff;
  uint count = 0;
  while(size) {
    long dim = min(size, maxSend);
    dim = performWrite(p, dim);
    if(-1 == dim)
        return 0;
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
long PLanConn::performRead(void *buff, long len)
{
  SOCKET Socket = getSocket();
  if(INVALID_SOCKET == Socket)
    return -1;

  int nRet = hasByte(Socket);
  if(nRet <= 0 ) {
#ifdef PRINT_INFO
    printf("performRead: nRet = %d\n", nRet);  
#endif
    return nRet;
    }
  if(RET_BY_GRACEFULL_CLOSE == nRet) {
#ifdef PRINT_INFO
    printf("performRead: nRet = %d\n", nRet);  
#endif
    return RET_BY_GRACEFULL_CLOSE;
    }

  nRet = lanRecv(Socket, (LPSTR)buff, len);
  return nRet >= 0 ? nRet : 0;
}
//--------------------------------------------------------------------------------
long PLanConn::read(void *buff, long len, bool lock)
{
#if 1
  SOCKET Socket = getSocket();
  if(INVALID_SOCKET == Socket)
    return -1;
  long nRet = lanRecv(Socket, (LPSTR)buff, len);
  return nRet >= 0 ? nRet : 0;

#else    
  long size = len;
  LPBYTE p = (LPBYTE)buff;
  uint count = 0;
  uint max_count = 5;
//  if(lock)
//      max_count *= 100;
  while(size) {
    long dim = performRead(p, size);
    if(dim < 0) {
        dim = 0;
        break;
        }
    if(!dim) {
      if(!lock && ++count >= max_count) {
#ifdef PRINT_INFO
        printf("count >= %d\n", max_count);      
#endif
        break;
        }
      continue;
      }
    size -= dim;
    p += dim;
    lock = false;
//    if(size)
      Sleep(10);
    }
#ifdef PRINT_INFO
  printf("exit read: readed=%d\n", len - size);
#endif
  return len - size;
#endif  
}
//--------------------------------------------------------------------------------
int PLanConn::lanSend(SOCKET socket, LPCSTR buff, int len)
{
//  return ::write(socket, buff, len);
  return send(socket, buff, len, 0);
}
//--------------------------------------------------------------------------------
int PLanConn::lanRecv(SOCKET socket, LPSTR buff, int len)
{
//  return ::read(socket, buff, len);
  return recv(socket, buff, len, 0);
}
//--------------------------------------------------------------------------------
bool PLanConn::isConnected()
{
  return getSocket() != INVALID_SOCKET && canSend(getSocket()) > 0;
}
