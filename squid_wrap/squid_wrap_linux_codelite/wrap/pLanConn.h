//------------------ PLanConn.h ----------------------------
#ifndef PLANCONN_H_
#define PLANCONN_H_
//-----------------------------------------------------------
#include "SocketUtil.h"
//-----------------------------------------------------------
class PLanConn
{
  public:
    PLanConn(LPCSTR type_helper, DWORD addr, DWORD port);
    PLanConn(LPCSTR type_helper, LPCSTR server_name, DWORD port);
    virtual ~PLanConn();

    void setServerName(LPCSTR name);
    bool open();
    bool startOpened(SOCKET sock);
    void close();

    virtual long write(const void *buff, long len);
    virtual long read(void *buff, long len, bool lock = true);

    const SOCKET getSocket() const { return Sock; }
    bool isConnected();
  protected:
    void setSocket(SOCKET newSock) { Sock = newSock; }

    virtual int lanSend(SOCKET socket, LPCSTR buff, int len);
    virtual int lanRecv(SOCKET socket, LPSTR buff, int len);

  private:
    LPCSTR typeHelper;
    DWORD Addr;
    DWORD Port;
    long maxSend;
    SOCKET Sock;
    LPCSTR serverName;

    long performWrite(const void *buff, long len);
    long performRead(void *buff, long len);
};
//-----------------------------------------------------------
#endif
