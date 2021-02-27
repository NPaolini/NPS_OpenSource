//------------------ SocketUtil.h ------------------------------------------------
#ifndef SOCKETUTIL_H_
#define SOCKETUTIL_H_
//--------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
//--------------------------------------------------------------------------------
#define MAX_BUFF_SOCK 8192
//--------------------------------------------------------------------------------
#define RET_BY_GRACEFULL_CLOSE 1000
//--------------------------------------------------------------------------------
#define MAX_LEN_CONN_NAME 256
//--------------------------------------------------------------------------------
//#define PRINT_INFO
//--------------------------------------------------------------------------------
#ifdef WINDOWS
#else
  uint32_t getTick();
  #define GetTickCount getTick
  #define Sleep(msec) usleep((msec) * 1000)
  #define WSAEINPROGRESS EINPROGRESS
  #define WSAGetLastError() errno
  #define WSAEWOULDBLOCK EWOULDBLOCK
  #define SOCKET int
  #define INVALID_SOCKET          ((SOCKET)(~0))
  #define SOCKET_ERROR            (-1)
  typedef unsigned short WORD;
  typedef unsigned long DWORD;
  typedef char* LPSTR;
  typedef const char* LPCSTR;
  #define SOCKADDR_IN sockaddr_in
  #define GetAddrInfo getaddrinfo
  typedef sockaddr * LPSOCKADDR;
  #define ioctlsocket ioctl
  #define closesocket close
  #define SD_BOTH SHUT_RDWR
  typedef unsigned char BYTE;
  typedef BYTE* LPBYTE;
  #define min(a, b) ((a) < (b) ? (a) : (b))
#endif
//--------------------------------------------------------------------------------
SOCKET createClientSocket(WORD port, DWORD &addr, int nRepeat = 50, LPCSTR serverName = 0);
//--------------------------------------------------------------------------------
// nRepeat è in decimi di secondo (approssimato, all'interno esegue dieci cicli con 10ms di sleep)
SOCKET createTalkSocket(WORD port, DWORD addr, int nRepeat = 50);
//--------------------------------------------------------------------------------
/*
WORD getPort(SOCKET sock, bool remote = false);
//--------------------------------------------------------------------------------
DWORD getAddr(SOCKET sock, bool remote = false);
//--------------------------------------------------------------------------------
bool getAddrAndPort(SOCKET sock, DWORD& addr, DWORD& port, bool remote = false);
//--------------------------------------------------------------------------------
bool getConnName(SOCKET sock, LPTSTR buff, bool remote = false);
*/
//--------------------------------------------------------------------------------
int hasByte(SOCKET sock);
//--------------------------------------------------------------------------------
int canSend(SOCKET sock);
//--------------------------------------------------------------------------------
#endif
