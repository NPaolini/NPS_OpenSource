//-------- sv_baseServer.h -----------------------------------------
//------------------------------------------------------------------
#ifndef sv_baseServer_H_
#define sv_baseServer_H_
//------------------------------------------------------------------
#include "precHeader.h"
#include "connClass.h"
//------------------------------------------------------------------
class svServer
{
  public:
    svServer() {}
    virtual ~svServer() {}

    virtual bool start(uint port, LPCTSTR serverName) = 0;

    virtual void perform() = 0;
    virtual void stop() = 0;
};
//------------------------------------------------------------------
struct staticIp4DynamicIp
{
  DWORD Ip;
  DWORD Port;
  DWORD pingMinute;
  DWORD localPort;

  staticIp4DynamicIp(DWORD ip = 0, DWORD port = 0) : Ip(ip), Port(port), pingMinute(10), localPort(0) {}
};
//------------------------------------------------------------------
extern svServer* allocServer();
extern LPCSTR getPassword();
extern LPCTSTR getServerName();
extern DWORD getTimeoutAck();
extern staticIp4DynamicIp getInfoDynamicIP();
//------------------------------------------------------------------
#define MAX_PASSWORD 16
//------------------------------------------------------------------
#endif
