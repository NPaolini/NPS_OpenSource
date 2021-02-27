//--------------- prf_theremino_v7.H ---------------------
//---------------------------------------------------------
#ifndef prf_theremino_v7_H_
#define prf_theremino_v7_H_
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <windows.h>
#include "comgperif.h"
#include "PCrt_lck.h"
#include "p_MappedMemory.h"
//---------------------------------------------------------
class theremino_Driver : public gestCommgPerif
{
  public:
    theremino_Driver(LPCTSTR file, WORD flagCommand, uint idPrf);
    virtual ~theremino_Driver();
    virtual ComResult Init();
  protected:
    p_MappedMemory pMM;
    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual ComResult SendData(addrToComm* pAddr);
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff);

  private:
    typedef gestCommgPerif baseClass;

};
//---------------------------------------------------------
#endif
