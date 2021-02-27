//--------------- commonUseBlock.h -------------------------
#ifndef commonUseBlock_H_
#define commonUseBlock_H_
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "comgperif.h"
#include "p_Vect.h"
#include "PCrt_lck.h"
//-----------------------------------------------------------
#define V_CHECK ipAddr
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
struct tempAddr
{
  int db;
  int addr;
  int len;
  int V_CHECK;
};
//-----------------------------------------------------------
#define MAX_FULL_BUFF (MAX_BDATA_PERIF - sizeof(tempAddr))
//-----------------------------------------------------------
struct fullAddr : public tempAddr
{
  BYTE buff[MAX_FULL_BUFF];
  const fullAddr& operator =(const tempAddr& other)
  {
    db = other.db;
    addr = other.addr;
    len = other.len;
    V_CHECK = other.V_CHECK;
    memset(buff, 0, sizeof(buff));
    return *this;
  }
};
//---------------------------------------------------------
class commonUseBlock : public gestCommgPerif
{
  private:
    typedef gestCommgPerif baseClass;
  public:
    commonUseBlock(LPCTSTR file, WORD flagCommand = 0);
    virtual ~commonUseBlock();
    virtual ComResult Init();
    virtual bool manageCounter(int& countSend, int& countRec, int& countTot, int& consErr);
  protected:
    HANDLE hThread;
    criticalSect criticalSectionConn;
    criticalSect criticalSectionBuff;

    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual ComResult SendData(addrToComm* pAddr);
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff);

    virtual void verifySet(setOfString& set);

    PVect<fullAddr> pvFAddr;

    // deve tornare zero per nessun errore
    // addr e dim sono in BYTE
    virtual int thPerformRead(fullAddr& fa) = 0;
    virtual int performSend(int check, int db, int addr, int dim, LPBYTE buff) = 0;

    virtual void setErrorByThread(bool set);
    virtual void setWaitForReaded() { ResetEvent(hEventReady);  }
    virtual void setReadyToRead() { SetEvent(hEventReady); }

    virtual DWORD waitForReady() { return WaitForSingleObject(hEventReady, 50);  }

    virtual gestComm::ComResult errorInit();
    void startTread() { if(hThread) ResumeThread(hThread); }

    void stopThread();
  private:
    bool errorByThread;
    int realErrorReceive;
    int realTotCount;
    int realConsErr;

    // usato dal thread per avvisare che si possono leggere i dati (solo per la prima lettura)
    HANDLE hEventReady;

    friend unsigned FAR PASCAL ReaderProc(void*);
};
#include "restorePack.h"
#endif
