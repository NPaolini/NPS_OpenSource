//------------ PCrt_lck.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PCRT_LCK_H_
#define PCRT_LCK_H_

#include "precHeader.h"

#include "setPack.h"

class criticalSect
{
  public:
    criticalSect()  { InitializeCriticalSection(&cS); }
    ~criticalSect() { DeleteCriticalSection(&cS); }
    typedef CRITICAL_SECTION& refCRITICAL_SECTION;
    operator refCRITICAL_SECTION() { return get(); }
    CRITICAL_SECTION& get()        { return cS; }
  private:
    CRITICAL_SECTION cS;

    criticalSect(const criticalSect &sl);
    criticalSect& operator=(const criticalSect &sl);
};
//----------------------------------------------------------------------------
class criticalLock
{
  public:
    criticalLock(CRITICAL_SECTION &cs) : cS(cs) { EnterCriticalSection(&cS); }
    ~criticalLock() { LeaveCriticalSection(&cS); }
  private:
    CRITICAL_SECTION &cS;

    criticalLock(const criticalLock &sl);
    criticalLock& operator=(const criticalLock &sl);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
