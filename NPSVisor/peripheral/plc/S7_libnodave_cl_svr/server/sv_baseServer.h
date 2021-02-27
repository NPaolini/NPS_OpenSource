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

    virtual bool start(uint id, LPCTSTR param) = 0;

    virtual void stop() = 0;
};
//------------------------------------------------------------------
extern svServer* allocServer();
extern LPCTSTR getPassword();
extern LPCTSTR getServerName();
extern DWORD getTimeoutAck();
//------------------------------------------------------------------
#define MAX_LEN_PASSWORD 16
//------------------------------------------------------------------
#endif
