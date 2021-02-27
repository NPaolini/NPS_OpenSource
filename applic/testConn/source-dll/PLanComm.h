//------------------ PLanComm.h ----------------------------
#ifndef PLANCOMM_H_
#define PLANCOMM_H_
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include "pcom.h"
#include "SocketUtil.h"
//-----------------------------------------------------------
// paramConn viene usato come DWORD per ip
// l'handle viene usato come socket
//-----------------------------------------------------------
class PLanComm : public PCom
{
  private:
    typedef PCom baseClass;
  public:
    PLanComm(DWORD port, HWND hwnd = 0, bool udp = false);
    virtual ~PLanComm();

    bool open(LPCTSTR name = 0);
    bool startOpened(SOCKET sock);
    void close();

    virtual DWORD write_string(const void *buff, DWORD len);

    const SOCKET getSocket() const { return (SOCKET)getIdCom(); }
    void reset();
    void setRemote(const SOCKADDR_IN& sa) { saRemote = sa; }
  protected:
    void setSocket(SOCKET newSock) { setIdCom((HANDLE)newSock); }

    virtual int lanSend(SOCKET socket, LPCSTR buff, int len);
    virtual int lanRecv(SOCKET socket, LPSTR buff, int len);

  private:
    SOCKADDR_IN saRemote;
    DWORD Port;
    bool Udp;
    long maxSend;

    DWORD performWrite_string(const void *buff, DWORD len);

    friend unsigned FAR PASCAL LanCommProc(void*);
};
//-----------------------------------------------------------
#endif
