//------WrapSquidService.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef WrapSquidService_H_
#define WrapSquidService_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "NTService.h"

extern int stopMain();
extern int startMain();


class WrapSquidService : public CNTService {
  public: // construction
    WrapSquidService();

  public: // overridables
    virtual void  Run(DWORD, LPTSTR *);
//    virtual void  Pause();
//    virtual void  Continue();
    virtual void  Shutdown();
    virtual void  Stop();

  private:
    HANDLE  m_hStop;
//    HANDLE  m_hPause;
//    HANDLE  m_hContinue;
};

#endif
