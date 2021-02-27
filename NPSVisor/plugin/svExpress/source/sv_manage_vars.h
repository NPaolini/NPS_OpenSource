//----------- sv_manage_vars.h -------------------------------------
#ifndef sv_manage_vars_H_
#define sv_manage_vars_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_make_dll.h"
#include "sv_manage_express.h"
#include "p_avl.h"
#include "P_SimpleFreePacket.h"
#include "defgPerif.h"
//----------------------------------------------------------------------------
#if (MAX_DWORD_PERIF > SIZE_OTHER_DATA)
  #define DIM_DAT MAX_DWORD_PERIF
#else
  #define DIM_DAT SIZE_OTHER_DATA
#endif
//------------------------------------------------------------------
typedef PVect<SV_prfData> prfDataSet;
typedef PVect<prfDataSet> prfDataMultiSet;
//------------------------------------------------------------------
class svRetrieveTypeByPrph
{
  public:
    svRetrieveTypeByPrph() : Type(SV_prfData::tNoData) {}
    SV_prfData::tData getDataType(uint idPrph);
  private:
    SV_prfData::tData Type;
};
//------------------------------------------------------------------
#define RET_ON_CMP(a) \
  if(a > other.a) \
    return true; \
  if(a < other.a) \
    return false
//------------------------------------------------------------------
struct infoVar
{
  int id;
  int prph;
  int addr;
  int type;
  int nbit;
  int offs;
  int norm;
};
typedef PVect<infoVar> vInfoVar;
//------------------------------------------------------------------
class infoDat
{
  public:
    infoDat() { ZeroMemory(value, sizeof(value)); }
    DWORD& operator[](int offs) { return value[offs]; }
  private:
    DWORD value[DIM_DAT];
};
//------------------------------------------------------------------
class manageBuff
{
  public:
    manageBuff() { }
    infoDat& operator[](int offs) { return iDat[offs]; }
  private:
    PVect<infoDat> iDat;
};
//------------------------------------------------------------------
class prph_Data : public genericSet
{
  public:
    prph_Data(uint id = 0, int offset = 0) : Id(id), Offset(offset), toSend(false) {}
    bool operator >(const prph_Data& other) {
      RET_ON_CMP(Id);
      RET_ON_CMP(Offset);
      return false;
      }
    void init(const infoVar& ivar);

    void* operator new(size_t s, void* p) {  return p;  }
    void operator delete(void* p, void* p2) {   }

    __int64 getValueFinal(const infoVar& ivar) const;
    __int64 getValue(const infoVar& ivar) const;
    void setValue(const infoVar& ivar, __int64 v);

    uint getId() const { return Id; }
    void setId(uint id) { Id = id; }
    int getOffset() const { return Offset; }
    void setOffset(int offs) { Offset = offs; }

    bool needSend() const { return toSend; }
    void setToSend(bool set = true) { toSend = set; }
  private:
    uint Id;
    int Offset;
    bool toSend;
    static manageBuff mBuff;
    __int64 getValue_p(const infoVar& ivar) const;
};
//-------------------------------------------------------------------
typedef genericAvl<prph_Data> orderedPrphDataSet;
//------------------------------------------------------------------
class svManVars : public manExpressionVars
{
  public:
    svManVars() : cacheVars(true) {}
    virtual ~svManVars();

    virtual P_Expr::exprVar getVariable(int id, int offset, int offsByStep, bool& exist);
    virtual void sendResult(const infoExpr& ie, P_Expr::exprVar val, int cycle, LPCTSTR realName);
    virtual void sendFinal();
    virtual void shoMsg(int id, LPCTSTR var, int num2, int row);
    virtual void manageInit(LPTSTR p);
    virtual void manageVar(LPTSTR p);
    virtual int getTypeResult(int id);
    virtual void reset() { Vars.reset(); Inits.reset(); clearCache(); }
    virtual void performInit();
    virtual void clearCache();
  private:
    vInfoVar Vars;

    struct infoInit
    {
      int id;
      P_Expr::exprVar val;
    };

    typedef PVect<infoInit> vInfoInit;
    vInfoInit Inits;

    int getTrueOffset(int offset);
    P_Expr::exprVar loadVal(const infoVar& ivar, int& offset, int offsByStep);
    bool getVariableAndData(int id, int& offset, P_Expr::exprVar& var, infoVar& idata, int offsByStep);
    orderedPrphDataSet cacheVars;
    P_SimpleFreePacket<prph_Data> freeSet;

    void sendCacheResult(const infoVar& ivar, P_Expr::exprVar val, int offset);
    void sendData(uint prph, prfDataSet& set);
    P_Expr::exprVar loadCacheVal(const infoVar& ivar, int offset);

    PVect<svRetrieveTypeByPrph> DefDataType;
    P_Expr::exprVar getTypedVal(const prph_Data* t, const infoVar& ivar);
    bool getInfoVar(infoVar& iVar);

};
//------------------------------------------------------------------
#endif
