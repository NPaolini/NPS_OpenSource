//--------------- s7_libnodave.h --------------------------
#ifndef s7_libnodave_H_
#define s7_libnodave_H_
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "comgperif.h"
#include "p_Vect.h"
#include "PCrt_lck.h"
//---------------------------------------------------------
#include "setPack1.h"
#define BCCWIN
#include "nodave.h"
#include "restorePack.h"
//---------------------------------------------------------
#include "commonUseBlock.h"
//---------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class myAdrTable
{
  public:
    myAdrTable(int chk) : V_CHECK(chk), di(0), dc(0), connType(0) { ZeroMemory(&fds, sizeof(fds)); }
    ~myAdrTable() { close(); }
    bool init(LPCTSTR str);
    daveConnection* getConn() { return dc; }
    int getCheck() const { return V_CHECK; }
  private:
    void close();
    bool openLan(LPCTSTR str);
    bool openMPI(LPCTSTR str);

    daveInterface* di;
    daveConnection* dc;
    _daveOSserialType fds;
    int V_CHECK;
    int connType;
};
//---------------------------------------------------------
class s7_libnodave : public commonUseBlock
{
  private:
    typedef commonUseBlock baseClass;
  public:
    s7_libnodave(LPCTSTR file, WORD flagCommand = 0);
    virtual ~s7_libnodave();
    virtual ComResult Init();
  protected:

    PVect<myAdrTable*> adr_table;
    void addToTable(uint check, setOfString& sSet);
    virtual int thPerformRead(fullAddr& fa);
    virtual int performSend(int check, int db, int addr, int dim, LPBYTE buff);
  private:
    void makeSetInfo(setOfString& set);
    daveConnection* getConn(DWORD ix);
};
//---------------------------------------------------------
#include "restorePack.h"
//---------------------------------------------------------
#endif
