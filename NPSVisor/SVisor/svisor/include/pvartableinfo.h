//-------- pVarTableInfo.h ---------------------------------------------------
#ifndef pVarTableInfo_H_
#define pVarTableInfo_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#ifndef PTEXTPANEL_H_
  #include "ptextpanel.h"
#endif
//----------------------------------------------------------------------------
namespace varTblInfo {
class manRow;
};
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class pVarTableInfo : public baseVar
{
  public:
    pVarTableInfo(P_BaseBody* owner, uint id);
    ~pVarTableInfo();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);
    void draw(HDC hdc, const PRect& rect);

  protected:
    virtual uint getBaseId() const { return ID_INIT_VAR_TABLE_INFO; }
    void performDraw(HDC) {}
  private:
    DWORD BitStat;
    varTblInfo::manRow* ManageRow;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

