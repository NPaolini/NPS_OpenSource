//--------------- prf_simulation_v7.H ---------------------
//---------------------------------------------------------
#ifndef prf_simulation_v7_H_
#define prf_simulation_v7_H_
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <windows.h>
#include "comgperif.h"
#include "PCrt_lck.h"
//-----------------------------------------------------
struct dual
{
  uint logicAddr;

  uint ipAddr;
  uint port;
  uint db;
  uint addr;
};
//---------------------------------------------------------
class simulation_Driver : public gestCommgPerif
{
  public:
    simulation_Driver(LPCTSTR file, WORD flagCommand, uint idPrf);
    virtual ~simulation_Driver();
    virtual ComResult Init();
    virtual bool addCommand(gPerif_Command cmd, WORD p1, DWORD p2, DWORD id_req, actionFromFile action = NO, LPBYTE buffer = 0);
  protected:
    uint idPrf;

    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual ComResult SendData(addrToComm* pAddr);
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff);

    virtual ComResult write(ComResult);

    virtual void verifySet(class setOfString& set);
  private:
    typedef gestCommgPerif baseClass;

    DWORD onWrite;

    HANDLE hThread;
    HANDLE hEventClose;
    criticalSect cS;
    bool simulErr;
    friend unsigned FAR PASCAL ReaderProc(void*);
    void reloadData();

    PVect<dual> dualSet;
    void makeDual(class setOfString& set);
    void saveData(const addrToComm* pAddr, uint offs, const dual& d, uint size);
    void loadData(const dual& d, BDATA* buff, uint size);

};
//---------------------------------------------------------
#endif
