//-------- sv_server.h ---------------------------------------------
//------------------------------------------------------------------
#ifndef sv_server_H_
#define sv_server_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_data.h"
#include "sv_make_dll.h"
#include "sv_baseServer.h"
#include "winServerConn.h"
//------------------------------------------------------------------
class PWinThread;
//------------------------------------------------------------------
class IsvServer : public svServer
{
  private:
    typedef svServer baseClass;
  public:
    IsvServer();
    ~IsvServer();

    bool start(uint port, LPCTSTR serverName);

    void perform();
    void stop();

    LPCTSTR getName() const { return ServerName; }
    uint getPort() const { return Port; }
    PWinThread* GetWinMsg() { return winMsg; }
    criticalSect& getCritSect();
  private:
    LPCTSTR ServerName;
    uint Port;
    PWinThread* winMsg;
    criticalSect csMsg;
    HANDLE hEventClose;

    criticalSect csThread;
    HANDLE hThread;

    void SetWinMsg(PWinThread* w) { winMsg = w; }

    friend unsigned FAR PASCAL MsgThread(void* pData);
};
//------------------------------------------------------------------
//------------------------------------------------------------------
#endif
