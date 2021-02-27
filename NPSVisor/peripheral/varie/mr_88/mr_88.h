//--------------- mr_88.h --------------------------------------------
//------------------------------------------------------------------------
#ifndef mr_88_H_
#define mr_88_H_
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include "comgperif.h"
#include "p_param_v.h"
#include "p_txt.h"
#include "mr_88_command.h"
//-----------------------------------------------------
struct dual
{
  uint logicAddr;
  uint addr;
};
//------------------------------------------------------------------------
#define MAX_VARS 256
//------------------------------------------------------------------------
class mr_88 : public gestCommgPerif
{
  public:
    mr_88(LPCTSTR file, WORD flagCommand = 0);
    virtual ~mr_88();
    virtual ComResult Init();
  protected:
    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual ComResult SendData(addrToComm* pAddr);
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff);
    //virtual int getStep(uint /*type*/) { return 1; }
    PConnBase* Conn;
    virtual void verifySet(class setOfString& set);
  private:
    typedef gestCommgPerif baseClass;
    ComResult SaveBuff(addrToComm* pAddr);
    ComResult saveBuffData(const addrToComm* pAddr);
    void copyBuffData(LPDWORD buff, uint num_data, uint addr);
    void saveData();
    void loadData();
    PVect<dual> dualSet;
    void makeDual(class setOfString& set);
    uint getLogicalAddr(uint addr);
    bool disableOutput;
    void setOutput(int x, int y);
#ifdef TEST_RECEIVE
    void  updateDataSend(uint ix, LPDWORD buff);
#else
    void  updateDataSend(uint ix);
#endif
};
//------------------------------------------------------------------------
#endif
