//-------- svIserverConn.h -----------------------------------------
//------------------------------------------------------------------
#ifndef svIserverConn_H_
#define svIserverConn_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_data.h"
#include "sv_baseServer.h"
#include "winServerConn.h"
#define BCCWIN
#include "nodave.h"
//------------------------------------------------------------------
//#define TEST_NO_CONN
//------------------------------------------------------------------
struct serializeConn
{
  HANDLE hWait;
  daveConnection* dC;
  uint id;
  serializeConn(HANDLE hwait) : hWait(hwait), dC(0), id(0) {}
};
//------------------------------------------------------------------
class IsvServer;
//------------------------------------------------------------------
class PWinThread : public winServerConn
{
  private:
    typedef winServerConn baseClass;
  public:
    PWinThread(IsvServer* server);
    ~PWinThread() {  destroy(); }

    PVect<winClientConn*>& getClientSet() { return Clients; }

    void getDaveConn(serializeConn& srzC);
    void releaseDaveConn(serializeConn& srzC);
    DWORD getIdPlc() const;
  protected:
    virtual bool createConn();

  private:
    IsvServer* mServer;
};
//------------------------------------------------------------------
#endif
