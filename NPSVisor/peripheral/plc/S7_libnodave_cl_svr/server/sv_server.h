//-------- sv_server.h ---------------------------------------------
//------------------------------------------------------------------
#ifndef sv_server_H_
#define sv_server_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "svIserverConn.h"
typedef P_FreePacketQueue<serializeConn, 100> t_daveQueue;
//------------------------------------------------------------------
class IsvServer : public svServer
{
  private:
    typedef svServer baseClass;
  public:
    IsvServer();
    ~IsvServer();

    bool start(uint id, LPCTSTR param);

    void stop();

    LPCTSTR getName() const { return ServerName; }
    uint getPort() const { return Port; }
    PWinThread* GetWinMsg() { return winMsg; }
    criticalSect& getCritSect();
    DWORD getIdPlc() const { return Id; }
  private:
    LPCTSTR ServerName;
    uint Id;
    uint Port;
    PWinThread* winMsg;
    criticalSect csMsg;
    HANDLE hEventClose;

    criticalSect csThread;
    HANDLE hThread;

    daveInterface* daveIface;
    daveConnection* daveConn;
    _daveOSserialType fds;

    t_daveQueue DaveQueue;
    void SetWinMsg(PWinThread* w) { winMsg = w; }

    bool openDaveLan(LPCTSTR ip, DWORD rack, DWORD slot);

    friend unsigned FAR PASCAL MsgThread(void* pData);
    friend class PWinThread;
};
//------------------------------------------------------------------
//------------------------------------------------------------------
#endif
