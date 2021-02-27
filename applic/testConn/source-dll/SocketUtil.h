//------------------ SocketUtil.h ------------------------------------------------
#ifndef SOCKETUTIL_H_
#define SOCKETUTIL_H_
//--------------------------------------------------------------------------------
#include "precompHeader.h"
//--------------------------------------------------------------------------------
#include "conn_dll.h"
//--------------------------------------------------------------------------------
#define MAX_BUFF_SOCK 8192
//--------------------------------------------------------------------------------
#define RET_BY_GRACEFULL_CLOSE 1000
//--------------------------------------------------------------------------------
#define MAX_LEN_CONN_NAME 256
//--------------------------------------------------------------------------------
SOCKET createClientSocket(WORD port, DWORD &addr, int nRepeat = 50, int type = SOCK_STREAM, LPCTSTR serverName = 0);
//--------------------------------------------------------------------------------
SOCKET createServerSocket(WORD port, int type = SOCK_STREAM);
//--------------------------------------------------------------------------------
// nRepeat è in decimi di secondo (approssimato, all'interno esegue dieci cicli con 10ms di sleep)
SOCKET createTalkSocket(WORD port, DWORD addr, int type = SOCK_STREAM, int nRepeat = 50);
//--------------------------------------------------------------------------------
WORD getPort(SOCKET sock, bool remote = false);
//--------------------------------------------------------------------------------
DWORD getAddr(SOCKET sock, bool remote = false);
//--------------------------------------------------------------------------------
bool getAddrAndPort(SOCKET sock, DWORD& addr, DWORD& port, bool remote = false);
//--------------------------------------------------------------------------------
bool getConnName(SOCKET sock, LPTSTR buff, bool remote = false);
//--------------------------------------------------------------------------------
int hasByte(SOCKET sock);
//--------------------------------------------------------------------------------
int canSend(SOCKET sock);
//--------------------------------------------------------------------------------
class serverLanData
{
  public:
    serverLanData();
    virtual ~serverLanData();

    virtual bool run(WORD port, int type = SOCK_STREAM);
    virtual LPVOID addConnection(SOCKET socket, DWORD remoteIP, DWORD remotePort) = 0;

    const SOCKET getSocket() const { return Socket; }

  protected:
    SOCKET Socket;
    int needClose;
    bool Udp;
    SOCKADDR_IN saRemote;

    friend unsigned FAR PASCAL mainServerLanProc(void* pData);
};
//----------------------------------------------------------------------------
class customServerLanData : public serverLanData
{
  public:
    customServerLanData(HWND owner, HWND client) : Owner(owner), Client(client)
    {
      ZeroMemory(&ICB, sizeof(ICB));
    }

    virtual LPVOID addConnection(SOCKET socket, DWORD remoteIP, DWORD remotePort);
    void setCallBack(const InfoCallBack& icb) { ICB = icb; }

  private:
    HWND Owner;
    HWND Client;
    InfoCallBack ICB;
};
//--------------------------------------------------------------------------------
#endif
