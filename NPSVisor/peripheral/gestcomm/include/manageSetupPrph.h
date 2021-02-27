//------------------ manageSetupPrph.H ------------------------------
//-----------------------------------------------------------
#ifndef manageSetupPrph_H_
#define manageSetupPrph_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "comgperif.h"
//-----------------------------------------------------------
class var4Setup : public genericSet
{
  public:
    var4Setup(uint ipAddr = 0, uint port = 0, uint db = 0, uint addr = 0, uint dataType = 0) :
        ipAddr(ipAddr), port(port), db(db),
        addr(addr), dataType(dataType), dwVal(0),
        readed(false), committed(true), lAddr(0) {}

    var4Setup(const addrToComm& pAddr) : ipAddr(pAddr.ipAddr), port(pAddr.port), db(pAddr.db),
        addr(pAddr.addr), dataType(pAddr.type), dwVal(0),
        readed(false), committed(true), lAddr(0) {}

    var4Setup(const var4Setup& other) : ipAddr(other.ipAddr), port(other.port), db(other.db),
        addr(other.addr), dataType(other.dataType), dwVal(other.dwVal),
        readed(other.readed), committed(other.committed), lAddr(other.lAddr) {}

    const var4Setup& operator=(const var4Setup& other) {
        ipAddr = other.ipAddr; port = other.port; db = other.db;
        addr = other.addr; dataType = other.dataType; dwVal = other.dwVal;
        readed = other.readed; committed = other.committed; lAddr = other.lAddr;
        return *this;
        }


    // dati scritti su file
    uint ipAddr;
    uint port;
    uint db;
    uint addr;
    uint dataType;

    // se si usano i tipi a 64bit occorre modificare la richiesta di scrittura
    DWDATA dwVal;

    // dati solo in memoria
    bool readed;
    bool committed;
    uint lAddr;

    bool read(P_File& pf);
    bool write(P_File& pf);

    bool operator >(const var4Setup& other) {
      CHECK_MIN_SUP(db);
      CHECK_MIN_SUP(addr);
      CHECK_MIN_SUP(ipAddr);
      CHECK_MIN_SUP(port);
      return false;
      }

    void copyFrom(const alternAddress& aa, uint lAddr);
};
//-----------------------------------------------------------------------------
typedef genericAvl<var4Setup> orderedVarSetup;
//-----------------------------------------------------------------------------
struct transf4Send
{
  uint addr;
  DWDATA value;
};
//-----------------------------------------------------------------------------
/*
  1) carica le variabili di setup, con loadSet() oppure con addVar()
  2) se non esistono si può distruggere l'oggetto.
  3) si carica da file le variabili precedentemente salvate. Se non esistono
      nel set possono essere omesse


*/
//-----------------------------------------------------------------------------
class manageSetupPrph
{
  public:
    manageSetupPrph(gestCommgPerif* owner, uint id);
    ~manageSetupPrph();

    bool loadSet(setOfString& set);
//    void addVar(const alternAddress& aa, uint lAddr);

    bool loadFile();

    void checkReadedVar(LPBYTE buff, uint size, const addrToComm* atc, uint offs);
    bool endedRead() const { return !readCount; }
    bool committed() const { return !commitCount; }

//    bool getValToRead(orderedPAddr& set);

    bool getValToSend(PVect<transf4Send>& set);

    void refreshWrite(addrToComm* pAddr);
  private:
    gestCommgPerif* Owner;
    uint Id;
    orderedVarSetup oVS;
    DWORD readCount;
    DWORD commitCount;
    bool saveFile();
    bool needSave;
    DWORD countDelaySave;
};
//-----------------------------------------------------------------------------
#endif
