//------------ PobjMutex.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PobjMutex_H_
#define PobjMutex_H_

#include "precHeader.h"

#include "setPack.h"
//----------------------------------------------------------------------------
class objMutex
{
  public:
    objMutex()  { hMutex = CreateMutex(0, false, 0); }
    ~objMutex() { CloseHandle(hMutex); }
    typedef HANDLE& refHANDLE;
    operator refHANDLE() { return get(); }
    HANDLE& get()     { return hMutex; }
  private:
    HANDLE hMutex;

    objMutex(const objMutex &sl);
    objMutex& operator=(const objMutex &sl);
};
//----------------------------------------------------------------------------
class objMutexLock
{
  public:
    objMutexLock(objMutex& hObj) : hObj(hObj) { WaitForSingleObject(&hObj, INFINITE); }
    ~objMutexLock() {  ReleaseMutex(&hObj); }
  private:
    objMutex &hObj;

    objMutexLock(const objMutexLock &sl);
    objMutexLock& operator=(const objMutexLock &sl);
};
//----------------------------------------------------------------------------
class mutexLock
{
  public:
    mutexLock(HANDLE &hM) : hM(hM) { WaitForSingleObject(hM, INFINITE); }
    ~mutexLock() {  ReleaseMutex(hM); }
  private:
    HANDLE &hM;

    mutexLock(const mutexLock &sl);
    mutexLock& operator=(const mutexLock &sl);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
