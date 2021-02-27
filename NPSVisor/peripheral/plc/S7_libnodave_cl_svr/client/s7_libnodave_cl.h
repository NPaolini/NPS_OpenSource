//--------------- s7_libnodave_cl.h ------------------------
#ifndef s7_libnodave_cl_H_
#define s7_libnodave_cl_H_
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "comgperif.h"
#include "p_Vect.h"
#include "PCrt_lck.h"
//---------------------------------------------------------
#include "commonUseBlock.h"
#include "s7_libnodave_common.h"
//---------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class myAdrTable
{
  public:
    myAdrTable(int chk) : V_CHECK(chk), Conn(0) {  }
    ~myAdrTable() { close(); }
    bool init(LPCTSTR p);
    int getCheck() const { return V_CHECK; }
    int readManyBytes_remote(int db, int start, int len, LPBYTE buffer);
    int writeManyBytes_remote(int db, int start, int len, LPBYTE buffer);
  private:
    int V_CHECK;
    void close();
    PConnBase* Conn;
    bool sendLogin(LPCTSTR login);
    void sendLogout();
};
//---------------------------------------------------------
class s7_libnodave_client : public commonUseBlock
{
  private:
    typedef commonUseBlock baseClass;
  public:
    s7_libnodave_client(LPCTSTR file, WORD flagCommand = 0);
    virtual ~s7_libnodave_client();
    virtual ComResult Init();
  protected:

    PVect<myAdrTable*> adr_table;
    void addToTable(uint check, setOfString& sSet);
    virtual int thPerformRead(fullAddr& fa);
    virtual int performSend(int check, int db, int addr, int dim, LPBYTE buff);
  private:
    void makeSetInfo(setOfString& set);
    void removeNullCheck();
};
//---------------------------------------------------------
#include "restorePack.h"
//---------------------------------------------------------
#endif
